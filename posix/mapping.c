#define _GNU_SOURCE
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
#include "process.h"
#include "utils_posix.h"


void* createAndOpenMapping(char* path, size_t* size, int process_mode) { 

	// LOCK

	*size = getFileSize2(path);

	int fd = open(path, O_RDONLY);
	assert(fd != -1);

    int protection = PROT_READ;
    int visibility = MAP_SHARED;

    void* map = mmap(NULL, *size, protection, visibility, fd, 0);

    close(fd);

    printf("IUYHASDYHIJUASHYBASFYHBUJK\n");
    write(1, map, *size);

    // UNLOCK

    return map;

}


int deleteMapping(void* map, size_t size) {

	int err = munmap(map, size);

	if (err != 0) return -1;

	return 0;

}