#ifndef PIPE_H_
	
	#define PIPE_H_

	#include "pipe.c"

	int createLoggerPipe();
	HANDLE getReader();
	HANDLE getWriter();
	void addPipe(HANDLE, HANDLE);
	int writePipe(char*);
	char* readPipe();
	void destroyLoggerPipe();

#endif