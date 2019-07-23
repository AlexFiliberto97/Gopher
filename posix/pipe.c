#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "process.h"
#include "utils_posix.h"
#include <pthread.h>
#include "../error.h"

struct Pipe {
	
	int handles[2];
	int err;
};

struct Pipe* loggerPipe;

struct Pipe* createLoggerPipe() {
	
	loggerPipe = (struct Pipe*) create_shared_memory(sizeof(struct Pipe));
	if (loggerPipe == MAP_FAILED) {
		throwError(1, CREATE_MAPPING);
		loggerPipe->err = 1;
		return loggerPipe;
	}
	if (pipe(loggerPipe->handles) != 0) {
		loggerPipe->err = PIPE_ERROR;
	} else {
		loggerPipe->err = 0;
	}
	return loggerPipe;
}

int writePipe(struct Pipe* pipe, char* msg) {

	char sz[11];
	sprintf(sz, "%d", (int) strlen(msg) + 1);
	char size[12];
	for (int i = 0; i < 11; i++) {
		if (i < 11 - strlen(sz)) {
			size[i] = '0';
		} else {
			size[i] = sz[i-11+strlen(sz)];
		}
	}
	size[11] = '\0';

	char* comp_msg = (char*) malloc(strlen(msg) + strlen(size) + 1);
	if (comp_msg == NULL) return ALLOC_ERROR;

	sprintf(comp_msg, "%s%s", size, msg);
	int written = write(pipe->handles[1], comp_msg, strlen(comp_msg) + 1);
	if (written < strlen(comp_msg) + 1) return WRITE_PIPE;

	free(comp_msg);
	return 0;
}

char* readPipe(struct Pipe* pipe) {

	int r; 
	char size[12];
	r = read(pipe->handles[0], size, 11);
	if (r < 11) {
		throwError(1, READ_PIPE);
		return NULL;
	}
	
	size[11] = '\0';
	int sz = atoi(size);
	char* msg = (char*) malloc(sz);
	if (msg == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	r = read(pipe->handles[0], msg, sz);
	if (r < sz) {
		free(msg);
		throwError(1, READ_PIPE);
		return NULL;
	}
	return msg;
}

int destroyLoggerPipe() {

	int err;

	err = close(loggerPipe->handles[0]);
	if (err == -1) return CLOSE_FD_ERROR;

	err = close(loggerPipe->handles[1]);
	if (err == -1) return CLOSE_FD_ERROR;

	err = free_shared_memory(loggerPipe, sizeof(loggerPipe));
	if (err = DELETE_MAPPING) return err;

	return 0;
}