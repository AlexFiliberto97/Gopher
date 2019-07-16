#ifndef PIPE_H_

	#define PIPE_H_

	#include "pipe.c"
	
	int initPipes();	
	int createPipe(char*);
	int newPipeIndex();
	int pipeIndex(char*);
	int createPipe(char*);
	int writePipe(char*, char*);
	char *readPipe(char*);

#endif
