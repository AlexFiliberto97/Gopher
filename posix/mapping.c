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
// #include "utils_posix.h"
#include "../utils.h"

#define MAX_MAP_SIZE 1073741824



// struct FileMapping {
// 	void* map;
// 	size_t size;
// };

void* createAndOpenMapping(char* path, long long* size, int process_mode) { 

	// LOCK

	*size = getFileSize2(path);

	int fd = open(path, O_RDONLY);
	assert(fd != -1);

    void* map = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);

    // UNLOCK

    return map;

}


int deleteMapping(void* map, size_t size) {

	int err = munmap(map, size);

	if (err != 0) return -1;

	return 0;

}


// struct FileMapping {
// 	void* map;
// 	size_t size;
// };

// void* createAndOpenMapping(char* path, long long* size, int process_mode, pthread_mutex_t* mutex) { 
// // void** createAndOpenMapping(char* path, long long* size, int process_mode) { 

// 	// LOCK
// 	pthread_mutex_lock(mutex);

// 	*size = getFileSize2(path);

// 	int fd = open(path, O_RDONLY);
// 	assert(fd != -1);

//     void* map = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);

//     close(fd);

//     // UNLOCK
// 	pthread_mutex_unlock(mutex);

//     return map;

// 	// int fd = open(path, O_RDONLY);
// 	// assert(fd != -1);

// 	// *size = getFileSize2(path);

// 	// int n_maps = *size / MAX_MAP_SIZE;
// 	// if (*size % MAX_MAP_SIZE > 0) n_maps++;

// 	// void** maps = (void**) malloc(sizeof(void*) * n_maps);

// 	// for (int i = 0; i < n_maps; i++) {
// 	// 	if (i == n_maps - 1) {
// 	// 		maps[i] = mmap(NULL, *size %  MAX_MAP_SIZE, PROT_READ, MAP_SHARED, fd, i * MAX_MAP_SIZE);	
// 	// 	} else {
// 	// 		maps[i] = mmap(NULL, (size_t) MAX_MAP_SIZE, PROT_READ, MAP_SHARED, fd, i * MAX_MAP_SIZE);
// 	// 	}
// 	// }

// 	// return maps;

// }