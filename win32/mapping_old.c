#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../error.h"
#include "../utils.h"


static const int MAP_VIEW_SIZE = 536870912; // 512 MB


struct FileMap {
	char* item;
	char* mapName;
	long long size;
	int err;
	HANDLE handle;
};


void freeFileMapStruct(struct FileMap* fmap) {
	free(fmap->item);
	free(fmap->mapName);
	free(fmap);
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

	HANDLE hMap = openMapping(fmap->mapName);
	if (hMap == NULL) {
		fmap->err = 1;
		return NULL;
	}

	DWORD offsetHigh = (DWORD) (offset >> 32);
	DWORD offsetLow = (DWORD) ((offset << 32) >> 32);

	long long bytes_left = fmap->size - offset;

	if (bytes_left >= MAP_VIEW_SIZE) {
		*n_bytes = MAP_VIEW_SIZE;
	} else {
		*n_bytes = bytes_left;
	}

	void* view_ptr = MapViewOfFile(hMap, FILE_MAP_READ, offsetHigh, offsetLow, (SIZE_T) *n_bytes);
	if (view_ptr == NULL) {
		fmap->err = 1;
		return NULL;
	}

	*handle = (int) hMap;
	fmap->err = 0;
	return view_ptr;

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