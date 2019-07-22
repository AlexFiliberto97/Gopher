#ifndef PIPE_H_
	
	#define PIPE_H_

	#include <stdio.h>
	#include "pipe.c"
	
	//void initPipes();
	//int newpipeIndex();
	//int pipeIndex();
	int createLoggerPipe();
	HANDLE getReader();
	HANDLE getWriter();
	void addPipe(HANDLE, HANDLE);
	int writePipe(char*);
	char* readPipe();
	void destroyLoggerPipe();

#endif