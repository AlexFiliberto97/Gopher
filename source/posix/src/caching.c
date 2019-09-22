#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <pthread.h>
#include "../include/caching.h"
#include "../include/locking.h"
#include "../include/mutex.h"
#include "../../common/include/const.h"
#include "../../common/include/utils.h"
#include "../../common/include/error.h"

/* Global variables */
CachePage* cache;
MutexCV* mcv;

/* Function: initCache
*  Initialize the cache.
*/
int initCache() {

	cache = (CachePage*) malloc(sizeof(CachePage) * CACHE_SIZE);
	if (cache == NULL) return INIT_CACHE_ERR;
	
	mcv = createMutexCV();
	if (mcv == NULL) {
		free(cache); 
		return INIT_CACHE_ERR;
	}

	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].h_item = 0;
		cache[i].nPage = -1;
		cache[i].view_ptr = NULL;
		cache[i].size = 0;
		cache[i].lru = 0;
		cache[i].used = 0;
		cache[i].lock = create_lock();
	}
	return 0;
}

/* Function: createMapping
*  Create file mapping for a given part of file.
*/
int createMapping(char* item, long long size, long long offset, int cacheIndex) {

	int fd = open(item, O_RDWR);
	if (fd == -1) return CREATE_MAPPING;
    return fd;
}

/* Function: isFull
*  Check if the cache is full.
*/
int isFull() {
	
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].used == 0) return 0;
	}
	return 1;
}

/* Function: incrementUsed
*  Increment the used value for a given cache page and lock the file.
*  NB: this function is called inside a locked region of code.
*/
int incrementUsed(int i, long long offset) {
	
	if (cache[i].used == 0) {
		if (lock_fd(cache[i].fd, cache[i].lock, offset, cache[i].size) != 0) {
			return LOCK_FILE;
		}
	}

	cache[i].used++;
	if (isFull()) mcv->full = 1;
	return 0;
}

/* Function: decrementUsed
*  Decrement the used value for a given cache page and unlock the file.
*/
void decrementUsed(int i) {
	
	mutexLock(mcv->mutex);
		
		cache[i].used--;
		if (cache[i].used == 0) {
			mcv->full = 0;
			pthread_cond_signal(mcv->cond1); 
			unlock_fd(cache[i].fd, cache[i].lock);
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
	
	while (mcv->full == 1) pthread_cond_wait(mcv->cond1, mcv->mutex);
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
void* readMapping(char* item, long long size, long long offset, int* nBytes, int* cacheIndex) {

	int nPage = offset / MAP_VIEW_SIZE;
	long long h_item = hash_item(item);
	if (h_item == 0) return NULL;

	mutexLock(mcv->mutex);

		int index = checkCache(h_item, nPage);
		if (index != -1) {
			cache[index].lru = 0;
			int err = incrementUsed(index, offset);
			if (err != 0) {
				throwError(err);
				mutexUnlock(mcv->mutex);
				return NULL;
			}
			*cacheIndex = index;
			*nBytes = cache[index].size;
			mutexUnlock(mcv->mutex);
			return cache[index].view_ptr;
		}

		index = newPageIndex(h_item, nPage);
		*cacheIndex = index;

		if (cache[index].view_ptr != NULL) {
			if (munmap(cache[index].view_ptr, cache[index].size) != 0) {
				throwError(UNMAP_VOF_ERROR);
				mutexUnlock(mcv->mutex);
				return NULL;
			}

			cache[index].h_item = 0;
			cache[index].view_ptr = NULL;
			reset_lock(cache[index].lock);
			close(cache[index].fd);			
		}

		long long bytes_left = size - offset;
		*nBytes = bytes_left >= MAP_VIEW_SIZE ? MAP_VIEW_SIZE : bytes_left;

		cache[index].size = *nBytes;
		cache[index].nPage = nPage;
		cache[index].h_item = h_item;
		cache[index].lru = 0;
		int err = incrementUsed(index, offset);
		if (err != 0) {
			throwError(err);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

		cache[index].fd = createMapping(item, *nBytes, offset, index);
		if (cache[index].fd < 0) {
			throwError(cache[index].fd);
			mutexUnlock(mcv->mutex);
			return NULL;
		}

		cache[index].view_ptr = mmap(NULL, *nBytes, PROT_READ, MAP_PRIVATE, cache[index].fd, offset);
		if (cache[index].view_ptr == MAP_FAILED) {
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
		munmap(cache[i].view_ptr, cache[i].size);
	}
	
	free(cache);
	destroyMutexCV(mcv);
}