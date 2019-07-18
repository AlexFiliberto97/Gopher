#ifndef PIPE_H_
	
	#define PIPE_H_

	#include <stdio.h>
	#include "pipe.c"
	
	void initPipes();
	int newpipeIndex();
	int pipeIndex(char*);
	int createPipe(char*);
	HANDLE getReader(char*);
	HANDLE getWriter(char*);
	int addPipe(char*, HANDLE, HANDLE);
	int writePipe(char*, char*);
	char* readPipe(char*);
	void destroyPipes();

#endif