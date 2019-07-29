#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h> 
#include "../error.h"
#include "../utils.h"

//static const int MAP_VIEW_SIZE = 536870912; // 512 MB
//#define MAP_VIEW_SIZE 536870912 // 512 MB
#define MAP_VIEW_SIZE 8388608 // 8 MB
#define CACHE_SIZE 8


struct FileMap {
	
	char* item;
	char* mapName;
	long long size;
	int err;
	HANDLE handle;
};

struct CachePage {

	char* item;
	void* view_ptr;
	long long size;
 	time_t lru;
};

struct CachePage cache[CACHE_SIZE];

void initCache() {

	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].item = NULL;
		cache[i].view_ptr = NULL;
		cache[i].size = -1;
		cache[i].lru = -1;
	}
}

int pageIndex() {

	time_t min = time(NULL);
	int index = -1;

	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].item == NULL) {
			return i;
		} else {
			if (cache[i].lru <= min) {
				min = cache[i].lru;
				index = i;
			}
		}
	}
	return index;
}

int checkCache(char* item) {

	for (int i = 0; i < CACHE_SIZE; i++) {
		if (strcmp(cache[i].item, item) == 0) {
			
			printf("Cache HIT: %s\n", item);
			return i;
		}
	}
	printf("Cache MISS: %s\n", item);
	return -1;
}

void freeFileMapStruct(struct FileMap* fmap) {
	
	if (fmap->item != NULL) free(fmap->item);
	if (fmap->mapName != NULL) free(fmap->mapName);
	if (fmap != NULL) free(fmap);
}


int createMapping(char* item, char* mapName, struct FileMap* fmap) {

	HANDLE hFile, hMap;
	BOOL succ;

	hFile = CreateFile(item, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		throwError(1, INVALID_HANDLE);
		return -1;
	}

	fmap->size = getFileSize(item);

	// OVERLAPPED overlapped = {0};
	// succ = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, *size, 0, &overlapped);
	// if (!succ) {
	// 	throwError(1, LOCK_FILE);
	// 	CloseHandle(hFile);
	// 	return NULL;
	// }

	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, mapName);
	if (hMap == NULL) {
		CloseHandle(hFile);
		return -1;
	}

	fmap->handle = hMap;

	fmap->item = (char*) malloc(strlen(item) + 1);
	if (fmap->item == NULL) {
		throwError(1, ALLOC_ERROR);
		return -1;
	}
	strcpy(fmap->item, item);

	fmap->mapName = (char*) malloc(strlen(mapName) + 1);
	if (fmap->mapName == NULL) {
		throwError(1, ALLOC_ERROR);
		return -1;
	}
	strcpy(fmap->mapName, mapName);

	// succ = UnlockFileEx(hFile, 0, *size, 0, &overlapped);
	// if (!succ) {
	// 	throwError(1, UNLOCK_FILE);
	// 	CloseHandle(hFile);
	// 	return NULL;
	// }

	return 0;
}


//Open an existingfile mapping
HANDLE openMapping(char* mapName) {
	
	HANDLE hMap;
	hMap = OpenFileMapping(FILE_MAP_READ, TRUE, mapName);
	if (hMap == NULL) return NULL;
	return hMap;
}

//Read an existing file mapping
void* readMapping(struct FileMap* fmap, long long offset, int* n_bytes, int* handle) {

	int nPage = offset / MAP_VIEW_SIZE;
	char* pageName = (char*) malloc (strlen(fmap->item) +13);
	sprintf(pageName, "%s-%d", fmap->item, nPage);
	int index = checkCache(pageName);

	if (index != -1) {
		free(pageName); 
		return cache[index].view_ptr;
	}

	HANDLE hMap = openMapping(fmap->mapName);
	if (hMap == NULL) {
		fmap->err = 1;
		return NULL;
	}

	//int nPage = offset / MAP_VIEW_SIZE;
	int cIndex = pageIndex();
	cache[cIndex].item = pageName;
	//sprintf(pageName, "%s-%d", fmap->item, nPage);


	DWORD offsetHigh = (DWORD) (offset >> 32);
	DWORD offsetLow = (DWORD) ((offset << 32) >> 32);
	long long bytes_left = fmap->size - offset;

	if (bytes_left >= MAP_VIEW_SIZE) {
		*n_bytes = MAP_VIEW_SIZE;

	} else {
		*n_bytes = bytes_left;

	}

	cache[cIndex].size = *n_bytes;
	cache[cIndex].view_ptr = MapViewOfFile(hMap, FILE_MAP_READ, offsetHigh, offsetLow, (SIZE_T) *n_bytes);
	//void* view_ptr = MapViewOfFile(hMap, FILE_MAP_READ, offsetHigh, offsetLow, (SIZE_T) *n_bytes);
	/*if (view_ptr == NULL) {
		fmap->err = 1;
		return NULL;
	}*/

	cache[cIndex].lru = time(NULL);

	*handle = (int) hMap;
	fmap->err = 0;
	return cache[cIndex].view_ptr;

}


//Delete an existing file mapping
int deleteView(char* view_ptr, int handle, int ignore) {
	BOOL succ;
	succ = UnmapViewOfFile(view_ptr);
	if (!succ) return DELETE_MAPPING;
	CloseHandle((HANDLE) handle);
	return 0;
}


void closeMapping(struct FileMap* fmap) {
	CloseHandle(fmap->handle);
}