#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/caching.h"
#include "../include/mutex.h"
#include "../include/locking.h"
#include "../../common/include/const.h"
#include "../../common/include/error.h"
#include "../../common/include/utils.h"

/* Global variables */
CachePage* cache;
MutexCV* mcv;

/* Function: initCache
*  Initialize the cache.
*/
int initCache() {

	cache = (CachePage*) malloc(sizeof(CachePage) * CACHE_SIZE);
	if (cache == NULL) return ALLOC_ERROR;

	mcv = createMutexCV();
	if (mcv == NULL) {
		free(cache);
		return INIT_CACHE_ERR;
	}
	
	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].h_item = 0;
		cache[i].hFile = INVALID_HANDLE_VALUE;
		cache[i].hMap = INVALID_HANDLE_VALUE;
		cache[i].overlapped = NULL;
		cache[i].view_ptr = NULL;
		cache[i].nPage = 0;
		cache[i].size = 0;
		cache[i].used = 0;
		cache[i].lru = 0;
	}
	return 0;
}

/* Function: createMapping
*  Create file mapping for a given file.
*/
int createMapping(char* item, long long size, HANDLE* hFile, HANDLE* hMap, OVERLAPPED* overlapped) {

	*hFile = CreateFile(item, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (*hFile == INVALID_HANDLE_VALUE || *hFile == NULL) {
		throwError(INVALID_HANDLE);
		return -1;
	}
	
	*hMap = CreateFileMapping(*hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	
	if (*hMap == INVALID_HANDLE_VALUE || *hMap == NULL) {
		CloseHandle(*hFile);
		throwError(INVALID_HANDLE);
		return -1;
	}
	
	return 0;
}

/* Function: isFull
*  Check if the cache is full.
*/
BOOL isFull() {
	
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].used == 0) {
			return FALSE;
		}
	}
	return TRUE;
}

/* Function: incrementUsed
*  Increment the used value for a given cache page.
*/
int incrementUsed(int i) {
	
	if (cache[i].used == 0) {
		if (!lockFile(cache[i].hFile, cache[i].size, cache[i].overlapped)) {
			return LOCK_FILE;
		}
	}
	cache[i].used++;
	if (isFull()) mcv->full = 1;
	return 0;
}

/* Function: decrementUsed
*  Decrement the used value for a given cache page.
*/
void decrementUsed(int i) {
	
	mutexLock(mcv->mutex);
		cache[i].used--;
		if (cache[i].used == 0) {
			mcv->full = 0;
			if (!PulseEvent(mcv->cv)) {
				throwError(PULSE_EVENT_ERR);
			}
			
			if (!unlockFile(cache[i].hFile, cache[i].size, cache[i].overlapped)) {
				throwError(UNLOCK_FILE);
			}
		}
	mutexUnlock(mcv->mutex);
}

/* Function: checkCache
*  Check if a given item is in cache.
*/
int checkCache(long long h_item, int nPage) {	

	int index = -1;
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].h_item == h_item && cache[i].nPage == nPage) {
			index = i;
		} else {
			cache[i].lru++;
		}
	}
	return index;
}

/* Function: newPageIndex
*  Cache replacement policy.
*/
int newPageIndex(long long h_item, int nPage) {

	int older = 0, index = -1;
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].view_ptr == NULL) return i;
	}

	while (mcv->full == 1) mcvWait(mcv);

	index = checkCache(h_item, nPage);
	if (index == -1) {
		for (int i = 0; i < CACHE_SIZE; i++) {
			if (cache[i].h_item != h_item && cache[i].used == 0) {
				if (cache[i].lru > older) {
					older = cache[i].lru;
					index = i;
				}
			}
		}
	}

	if (index == -1) {
		for (int i = 0; i < CACHE_SIZE; i++) {
			if (cache[i].lru > older && cache[i].used == 0) {
				older = cache[i].lru;
				index = i;
			}
		}
	}
	return index;
}

/* Function: readMapping
*  Manage the cache and return the pointer (mapping) to a requested file.
*/
void* readMapping(char* item, long long size, long long offset, int* n_bytes, int* cache_index) {

	int nPage = offset / MAP_VIEW_SIZE, err = 0;
	long long h_item = hash_item(item);
	if (h_item == 0) return NULL;

	mutexLock(mcv->mutex);

		int index = checkCache(h_item, nPage);
		if (index != -1) {
			cache[index].lru = 0;
			err = incrementUsed(index);
			if (err != 0) {
				throwError(err);
				mutexUnlock(mcv->mutex);
				return NULL;
			}
			*cache_index = index;
			*n_bytes = cache[index].size;
			mutexUnlock(mcv->mutex);
			return cache[index].view_ptr;
		}

		index = newPageIndex(h_item, nPage);
		*cache_index = index;
		if (cache[index].view_ptr != NULL) {
			if (!UnmapViewOfFile(cache[index].view_ptr)) {
				throwError(UNMAP_VOF_ERROR);
				mutexUnlock(mcv->mutex);
				return NULL;
			}

			cache[index].h_item = 0;
			cache[index].view_ptr = NULL;
			free(cache[index].overlapped);
			CloseHandle(cache[index].hMap);
			CloseHandle(cache[index].hFile);
		}

		long long bytes_left = size - offset;
		*n_bytes = bytes_left >= MAP_VIEW_SIZE ? MAP_VIEW_SIZE : bytes_left;

		cache[index].size = *n_bytes;
		cache[index].nPage = nPage;
		cache[index].h_item = h_item;
		cache[index].lru = 0;

		DWORD offsetHigh = (DWORD) (offset >> 32);
		DWORD offsetLow = (DWORD) ((offset << 32) >> 32);
		OVERLAPPED* overlapped = (OVERLAPPED*) malloc(sizeof(OVERLAPPED));
		if (overlapped == NULL) {
			throwError(ALLOC_ERROR);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

		ZeroMemory(overlapped, sizeof(OVERLAPPED));
		overlapped->Offset = offsetLow;
		overlapped->OffsetHigh = offsetHigh;
		err = createMapping(item, *n_bytes, &(cache[index].hFile), &(cache[index].hMap), overlapped);
		if (err != 0) {
			throwError(CREATE_MAPPING);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

		cache[index].overlapped = overlapped;
		err = incrementUsed(index);
		if (err != 0) {
			throwError(err);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

		cache[index].view_ptr = MapViewOfFile(cache[index].hMap, FILE_MAP_READ, offsetHigh, offsetLow, (SIZE_T) cache[index].size);
		if (cache[index].view_ptr == NULL) {
			throwError(MAP_VOF_ERROR);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

	mutexUnlock(mcv->mutex);
	return cache[index].view_ptr;
}

/* Function: destroyCache
*  Destroy the cache.
*/
void destroyCache() {
	
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].view_ptr != NULL) {
	 		if (!UnmapViewOfFile(cache[i].view_ptr)) {
	 			throwError(UNMAP_VOF_ERROR);
			}
	 	} 
	 	if (cache[i].hFile != INVALID_HANDLE_VALUE) CloseHandle(cache[i].hFile);
	 	if (cache[i].hMap != INVALID_HANDLE_VALUE) CloseHandle(cache[i].hMap);
	 	if (cache[i].overlapped != NULL) free(cache[i].overlapped);
	}
	
	free(cache);
	destroyMutexCV(mcv);
}