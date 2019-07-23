#ifndef PIPE_H_

	#define PIPE_H_

	#include "pipe.c"
	
	struct Pipe* createLoggerPipe();
	int writePipe(struct Pipe*, char*);
	char* readPipe(struct Pipe*);
	int destroyLoggerPipe();

#endif