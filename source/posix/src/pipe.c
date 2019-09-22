#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "../include/pipe.h"
#include "../include/utils_posix.h"
#include "../../common/include/error.h"

/* Global variables*/
Pipe* loggerPipe;

/* Function: createLoggerPipe
*  create the logger pipe.
*/
Pipe* createLoggerPipe() {
	
	loggerPipe = (Pipe*) create_shared_memory(sizeof(Pipe));
	if (loggerPipe == MAP_FAILED) {
		throwError(CREATE_MAPPING);
		loggerPipe->err = 1;
		return loggerPipe;
	}

	loggerPipe->err = (pipe(loggerPipe->handles) != 0) ? 1 : 0;
	return loggerPipe;
}

/* Function: writePipe
*  Write on the logger pipe.
*/
int writePipe(Pipe* pipe, char* msg) {

	char sz[11];
	sprintf(sz, "%d", (int) strlen(msg) + 1);

	char size[12];

	for (int i = 0; i < 11; i++) {
		if (i < 11 - strlen(sz)) {
			size[i] = '0';
		} else {
			size[i] = sz[i -11 +strlen(sz)];
		}
	}
	size[11] = '\0';
	char* comp_msg = (char*) malloc(strlen(msg) + strlen(size) + 1);
	if (comp_msg == NULL) {
		return ALLOC_ERROR;
	}

	sprintf(comp_msg, "%s%s", size, msg);
	int written = write(pipe->handles[1], comp_msg, strlen(comp_msg) + 1);
	if (written < strlen(comp_msg) + 1) {
		free(comp_msg);
		return WRITE_PIPE;
	}

	free(comp_msg);
	return 0;
}

/* Function: readPipe
*  Read from logger pipe.
*/
char* readPipe(Pipe* pipe) {

	int r; 
	char size[12];
	r = read(pipe->handles[0], size, 11);
	if (r < 11) {
		throwError(READ_PIPE);
		return NULL;
	}

	size[11] = '\0';
	int sz = atoi(size);
	char* msg = (char*) malloc(sz);
	if (msg == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	r = read(pipe->handles[0], msg, sz);
	if (r < sz) {
		free(msg);
		throwError(READ_PIPE);
		return NULL;
	}

	return msg;
}

/* Function: destroyLoggerPipe
*  Destroy the logger pipe.
*/
void destroyLoggerPipe() {
	
	close(loggerPipe->handles[0]);
	close(loggerPipe->handles[1]);
	free_shared_memory(loggerPipe, sizeof(loggerPipe));
}