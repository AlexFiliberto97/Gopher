#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <assert.h>
#include <pthread.h>
#include "process.h"
#include "../utils.h"
#include "../error.h"
#include "locking.h"

#define MAP_VIEW_SIZE 8388608 // 8 MB
#define CACHE_SIZE 12


struct FileMap {
	char* item;
	char* mapName;
	long long size;
	int err;
	int fd;
	struct flock lock;
};


struct CachePage {
	char* item;
	int n_page;
	void* view_ptr;
	long long size;
	int lru;
};


struct CachePage cache[CACHE_SIZE];


void initCache() {
	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].item = NULL;
		cache[i].n_page = -1;
		cache[i].view_ptr = NULL;
		cache[i].size = -1;
		cache[i].lru = 0;
	}
}


int pageIndex(char* item) {

	int older = 0;
	int index = -1;

	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].item == NULL) return i;
	}

	for (int i = 0; i < CACHE_SIZE; i++) {

		if (strcmp(item, cache[i].item) != 0) {
			if (cache[i].lru > older) {
				older = cache[i].lru;
				index = i;
			}
		}

	}

	if (index != -1) return index;

	for (int i = 0; i < CACHE_SIZE; i++) {

		if (cache[i].lru < older) {
			older = cache[i].lru;
			index = i;
		}
	}

	return index;

}

int checkCache(char* item, int n_page) {

	int index = -1;

	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].item != NULL && strcmp(cache[i].item, item) == 0 && cache[i].n_page == n_page) {
			printf("Cache HIT: %s\n", item);
			index = i;
		} else {
			cache[i].lru++;
		}
	}
	if (index == -1) printf("Cache MISS: %s\n", item);
	return index;

}


void freeFileMapStruct(struct FileMap* fmap) {
	free(fmap->item);
	free(fmap->mapName);
	free(fmap);
}


int createMapping(char* item, char* ignore, struct FileMap* fmap) {

	fmap->size = getFileSize(item);

	int fd = open(item, O_RDWR);
	if (fd == -1) {
		fmap->err = 1;
		throwError(1, CREATE_MAPPING);
		return -1;
	}

	fmap->fd = fd;

	fmap->lock = create_lock();

	fmap->item = (char*) malloc(strlen(item) + 1);
	if (fmap->item == NULL) {
		fmap->err = 1;
		close(fd);
		throwError(1, ALLOC_ERROR);
		return -1;
	}
	strcpy(fmap->item, item);

	fmap->err = 0;

    return 0;

}


int deleteView(char* view_ptr, int ignore, int size) {
	return free_shared_memory(view_ptr, size);
}


void* readMapping(struct FileMap* fmap, long long offset, int* n_bytes, int* ignore) {

	char* item = cpyalloc(fmap->item);
	if (item == NULL) return NULL;

	int n_page = offset / MAP_VIEW_SIZE;

	int index = checkCache(item, n_page);

	if (index != -1) {
		cache[index].lru = 0;
		*n_bytes = cache[index].size;
		free(item); 
		return cache[index].view_ptr;
	}

	int page_index = pageIndex(item);
	// free(cache[page_index].item);
	// deleteView(cache[page_index].view_ptr, 0, cache[page_index].size);
	cache[page_index].item = item;
	cache[page_index].n_page = n_page;

	long long bytes_left = fmap->size - offset;

	if (bytes_left >= MAP_VIEW_SIZE) {
		*n_bytes = MAP_VIEW_SIZE;
	} else {
		*n_bytes = bytes_left;
	}

	cache[page_index].size = *n_bytes;

	cache[page_index].view_ptr = mmap(NULL, (size_t) *n_bytes, PROT_READ, MAP_SHARED, fmap->fd, (off_t) offset);
	if (cache[page_index].view_ptr == MAP_FAILED) {
		close(fmap->fd);
		free(cache[page_index].item);
		cache[page_index].item = NULL;
		fmap->err = 1;
		throwError(1, CREATE_MAPPING);
		return NULL;
	}

	cache[page_index].lru = 0;

	fmap->err = 0;
	return cache[page_index].view_ptr;

}


void closeMapping(struct FileMap* fmap) {
	close(fmap->fd);
}