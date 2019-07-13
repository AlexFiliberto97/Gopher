#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

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
	return -1;
}

int pipeIndex(char* name) {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0) {
			return i;
		}
	}
	return -1;
}

int createPipe(char *name){

	int counter = newpipeIndex();
	if (counter == -1) return -1;

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
	return NULL;
}

HANDLE getWriter(char* name) {
	
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0) {
			return Pipes[i].hWrite;
		}
	}
	return NULL;
}

int addPipe(char* name, HANDLE hRead, HANDLE hWrite) {

	DWORD counter = newpipeIndex();
	if (counter == -1) return FALSE;

	Pipes[counter].name = (char*) malloc (strlen(name) +1);
	strcpy(Pipes[counter].name, name);
	Pipes[counter].hRead = hRead;
	Pipes[counter].hWrite = hWrite;
	return 0;
}

int writePipe(char* name, char* msg){

	int index = pipeIndex(name);
	if (index == -1) return -1;

	char buffer[PIPE_PACKET_LENGTH];
	DWORD written, c = 0;
	BOOL succ;
	
	for(int i = 0; i < strlen(msg) + 1; i++){
		buffer[c++] = msg[i];
		if (c == PIPE_PACKET_LENGTH || msg[i] == '\0'){
			succ = WriteFile(Pipes[index].hWrite, buffer, PIPE_PACKET_LENGTH, &written, NULL);
			c = 0;
		}
		if (!succ) return -1;
	}
	return 1;
}

char* readPipe(char* name) {

	int index = pipeIndex(name);
	if (index == -1) return NULL;

	char* msg = (char *) malloc(1);
	if (msg == NULL) return NULL;
	msg[0] = '\0';
	

	char buf[PIPE_PACKET_LENGTH];
    DWORD bytes_read;
    size_t r_tot = 0;

    do {

    	BOOL succ = ReadFile(Pipes[index].hRead, buf, PIPE_PACKET_LENGTH, &bytes_read, NULL);
		if (!succ) return NULL;

    	r_tot += bytes_read;
    	msg = (char *) realloc(msg, r_tot);
    	if (msg == NULL) return NULL;
		
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