#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../error.h"

#define PIPE_PACKET_LENGTH 32
#define MAX_PIPE_NUM 10

struct Pipe {
	char* name;
	HANDLE hRead;
	HANDLE hWrite;
};

static struct Pipe Pipes[MAX_PIPE_NUM];

void initPipes() {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		Pipes[i].name = NULL;
		Pipes[i].hRead = NULL;
		Pipes[i].hWrite = NULL;
	}
}

int newpipeIndex() {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (Pipes[i].name == NULL) {
			return i;
		}
	}
	return PIPE_UNAVAILABLE;
}

int pipeIndex(char* name) {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0) {
			return i;
		}
	}
	return PIPE_UNAVAILABLE;
}

int createPipe(char *name){

	int counter = newpipeIndex();
	if (counter < 0) {
		throwError(1, counter);
		return PIPE_ERROR;
	}

	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES pipeSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	if (CreatePipe(&hRead, &hWrite, &pipeSA, 0) == FALSE) return -1;

	Pipes[counter].name = (char*) malloc (strlen(name) +1);
	strcpy(Pipes[counter].name, name);
	Pipes[counter].hRead = hRead;
	Pipes[counter].hWrite = hWrite;
	return 0;
}

HANDLE getReader(char* name) {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0) {
			return Pipes[i].hRead;
		}
	}
	throwError(1, PIPE_UNAVAILABLE);
	return NULL;
}

HANDLE getWriter(char* name) {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0) {
			return Pipes[i].hWrite;
		}
	}
	throwError(1, PIPE_UNAVAILABLE);
	return NULL;
}

int addPipe(char* name, HANDLE hRead, HANDLE hWrite) {

	DWORD counter = newpipeIndex();
	if (counter < 0) return counter;

	Pipes[counter].name = (char*) malloc (strlen(name) +1);
	strcpy(Pipes[counter].name, name);
	Pipes[counter].hRead = hRead;
	Pipes[counter].hWrite = hWrite;
	return 0;
}

int writePipe(char* name, char* msg){

	int index = pipeIndex(name);
	if (index < 0) {
		throwError(1, index);
		return WRITE_PIPE;
	}

	char buffer[PIPE_PACKET_LENGTH];
	DWORD written, c = 0;
	BOOL succ;
	
	for(int i = 0; i < strlen(msg) + 1; i++){
		buffer[c++] = msg[i];
		if (c == PIPE_PACKET_LENGTH || msg[i] == '\0'){
			succ = WriteFile(Pipes[index].hWrite, buffer, PIPE_PACKET_LENGTH, &written, NULL);
			c = 0;
		}
		if (!succ) return WRITE_PIPE;
	}
	return 0;
}

char* readPipe(char* name) {

	int index = pipeIndex(name);
	if (index < 0) {
		throwError(2, READ_PIPE, index);
		return NULL;
	}

	char* msg = (char *) malloc(1);
	if (msg == NULL){
		throwError(2, READ_PIPE, ALLOC_ERROR);
		return NULL;
	}

	msg[0] = '\0';
	char buf[PIPE_PACKET_LENGTH];
    DWORD bytes_read;
    size_t r_tot = 0;
    do {

    	BOOL succ = ReadFile(Pipes[index].hRead, buf, PIPE_PACKET_LENGTH, &bytes_read, NULL);
		if (!succ) {
			throwError(1, READ_PIPE);
			return NULL;
		}

    	r_tot += bytes_read;
    	msg = (char *) realloc(msg, r_tot);
    	if (msg == NULL) {
			throwError(2, READ_PIPE, ALLOC_ERROR);
			return NULL;
		}
		
		memcpy((void *) &msg[r_tot-bytes_read], (void *) buf, bytes_read);
    } while (memchr(buf, '\0', PIPE_PACKET_LENGTH) == NULL);
	
	return msg;
}

void destroyPipes() {

	for (int i = 0; i < MAX_PIPE_NUM; i++) {

		if (Pipes[i].name != NULL) {
			free(Pipes[i].name);
			CloseHandle(Pipes[i].hRead);
			CloseHandle(Pipes[i].hWrite);
		}
	}
}