#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../error.h"

#define PIPE_PACKET_LENGTH 32
#define MAX_PIPE_NUM 10

struct Pipe {
	
	HANDLE hRead;
	HANDLE hWrite;
};

static struct Pipe loggerPipe;

int createLoggerPipe(){

	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES pipeSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	if (CreatePipe(&hRead, &hWrite, &pipeSA, 0) == FALSE) return PIPE_ERROR;

	loggerPipe.hRead = hRead;
	loggerPipe.hWrite = hWrite;
	return 0;
}

HANDLE getReader() {

	return loggerPipe.hRead;
}

HANDLE getWriter() {
		
	return loggerPipe.hWrite;
}

void addPipe(HANDLE hRead, HANDLE hWrite) {

	loggerPipe.hRead = hRead;
	loggerPipe.hWrite = hWrite;
}

int writePipe(char* msg) {

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
	size[11] = '\0';

	char* comp_msg = (char*) malloc(strlen(msg) + strlen(size) + 1);
	if (comp_msg == NULL) return ALLOC_ERROR;

	sprintf(comp_msg, "%s%s", size, msg);
	DWORD written;
	BOOL succ = WriteFile(loggerPipe.hWrite, comp_msg, strlen(comp_msg) +1, &written, NULL);
	if (written < strlen(comp_msg) + 1 || succ == FALSE) {
		free(comp_msg);
		return WRITE_PIPE;
	}

	free(comp_msg);
	return 0;
}

char* readPipe(){

	char size[12];
	DWORD bytes_read;
	BOOL succ = ReadFile(loggerPipe.hRead, size, 11, &bytes_read, NULL);
	if (bytes_read < 11 || succ == FALSE) {
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

	succ = ReadFile(loggerPipe.hRead, msg, sz, &bytes_read, NULL);
	if (bytes_read < sz || succ == FALSE) {
		free(msg);
		throwError(1, READ_PIPE);
		return NULL;
	}
	return msg;
}

void destroyLoggerPipe() {

	CloseHandle(loggerPipe.hRead);
	CloseHandle(loggerPipe.hWrite);
}