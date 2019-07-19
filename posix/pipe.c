// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "process.h"
#include "utils_posix.h"
#include <pthread.h>


struct Pipe {
	int handles[2];
	int err;
};


struct Pipe* loggerPipe;


struct Pipe* createLoggerPipe() {
	loggerPipe = (struct Pipe*) create_shared_memory(sizeof(struct Pipe));
	if (pipe(loggerPipe->handles) != 0) {
		loggerPipe->err = 1;
	} else {
		loggerPipe->err = 0;
	}
	return loggerPipe;
}


int writePipe(struct Pipe* pipe, char* msg) {

	close(pipe->handles[0]);

	char sz[11];
	sprintf(sz, "%d", (int) strlen(msg) + 1);

	char size[11];

	for (int i = 0; i < 11; i++) {
		if (i < 11 - strlen(sz)) {
			size[i] = '0';
		} else {
			size[i] = sz[i-11+strlen(sz)];
		}
	}

	char* comp_msg = (char*) malloc(strlen(msg) + strlen(size) + 1);
	sprintf(comp_msg, "%s%s", size, msg);

	int written = write(pipe->handles[1], comp_msg, strlen(comp_msg) + 1);

	if (written < strlen(comp_msg) + 1) {
		return -1;
	}

	free(comp_msg);

	return 0;

}


char* readPipe(struct Pipe* pipe){

	close(pipe->handles[1]);

	int r; 

	char size[12];
	r = read(pipe->handles[0], size, 11);
	if (r < 11) return NULL;
	size[11] = '\0';
	
	int sz = atoi(size);


	char* msg = (char*) malloc(sz);
	r = read(pipe->handles[0], msg, sz);
	if (r < sz) return NULL;

	return msg;

}


int destroyLoggerPipe() {
	close(loggerPipe->handles[0]);
	close(loggerPipe->handles[1]);
	free_shared_memory(loggerPipe, sizeof(loggerPipe));
}