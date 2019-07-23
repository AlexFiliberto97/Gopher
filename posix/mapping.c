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

void* createAndOpenMapping(char* path, long long* size, int process_mode) { 

	*size = getFileSize(path);
	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		throwError(1, CREATE_MAPPING);
		return NULL;
	}

    void* map = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		throwError(1, CREATE_MAPPING);
		return NULL;
	}

    close(fd);



    return map;

}


int deleteMapping(void* map, size_t size) {
	return free_shared_memory(map, size);
}


// #define MAX_MAP_SIZE 1073741824

// struct FileMapping {
// 	void* map;
// 	size_t size;
// };

// void* createAndOpenMapping(char* path, long long* size, int process_mode, pthread_mutex_t* mutex) { 
// // void** createAndOpenMapping(char* path, long long* size, int process_mode) { 

// 	// LOCK
// 	pthread_mutex_lock(mutex);

// 	*size = getFileSize(path);

// 	int fd = open(path, O_RDONLY);
// 	assert(fd != -1);

//     void* map = mmap(NULL, *size, PROT_READ, MAP_SHARED, fd, 0);

//     close(fd);

//     // UNLOCK
// 	pthread_mutex_unlock(mutex);

//     return map;

// 	// int fd = open(path, O_RDONLY);
// 	// assert(fd != -1);

// 	// *size = getFileSize(path);

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