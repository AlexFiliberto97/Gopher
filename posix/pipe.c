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


#define PIPE_PACKET_LENGTH 32
#define MAX_PIPE_NUM 10


struct Pipe {
	char* name;
	int handles[2];
};


static struct Pipe Pipes[MAX_PIPE_NUM];


int initPipes() {

	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		Pipes[i].name = NULL;
		Pipes[i].handles[0] = -1;
		Pipes[i].handles[1] = -1;
	}

}



//Return the first avaliable index in Pipes array
int newPipeIndex() {
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (Pipes[i].name == NULL)
			return i;
	}
	return -1;
}

//Return the first avaliable index in Pipes array
int pipeIndex(char* name) {
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (strcmp(Pipes[i].name, name) == 0)
			return i;
	}
	return -1;
}

//Create a new pipe identified by a name
int createPipe(char* name){

	int index = newPipeIndex();
	if(index == -1)
		return -1;

	if(pipe(Pipes[index].handles) == -1){
		return -1;
	}

	Pipes[index].name = name;

	return 0;

}


//Write on a pipe
int writePipe(char* name, char* msg) {

	int index = pipeIndex(name);
	if(index == -1)
		return -1;

	close(Pipes[index].handles[0]);

	char buffer[PIPE_PACKET_LENGTH];
	int written = 0, tot_written = 0;
	char* endchar = NULL;
	
	while (tot_written < strlen(msg) + 1) {

		memcpy(buffer, &msg[tot_written], PIPE_PACKET_LENGTH);

		written = write(Pipes[index].handles[1], buffer, PIPE_PACKET_LENGTH);

		if (written < 0) {
			return -1;
		}

		tot_written += written;

	}

	return 0;

}


//Read from a pipe
char* readPipe(char* name){

	int index = pipeIndex(name);
	if(index == -1) return NULL;

	close(Pipes[index].handles[1]);

	char buffer[PIPE_PACKET_LENGTH+1];
	char* msg = (char*) malloc(1);
	msg[0] = '\0';
	int counter = 0, r = 0;
	char* endchar = NULL;
	
	do {

		int r = read(Pipes[index].handles[0], buffer, PIPE_PACKET_LENGTH);
    	if (r < 0) return NULL;

    	endchar = memchr(buffer, '\0', PIPE_PACKET_LENGTH);
    	buffer[PIPE_PACKET_LENGTH] = '\0';
		msg = (char *) realloc(msg, ++counter * PIPE_PACKET_LENGTH);
    	strcat(msg, buffer);


	} while (endchar == NULL);

	msg = (char *) realloc(msg, strlen(msg) + 1);

	return msg;

}


void destroyPipes() {
	for (int i = 0; i < MAX_PIPE_NUM; i++) {
		if (Pipes[i].name != NULL) {
			free(Pipes[i].name);
			close(Pipes[i].handles[0]);
			close(Pipes[i].handles[1]);
			Pipes[i].handles[0] = -1;
			Pipes[i].handles[1] = -1;
		}
	}
}