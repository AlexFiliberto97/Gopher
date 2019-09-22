#ifndef PIPE_H_

	#define PIPE_H_
	
	typedef struct _Pipe {
		
		int handles[2];
		int err;
	} Pipe;

	/* Global variables */
	extern Pipe* loggerPipe;

	Pipe* createLoggerPipe();
	int writePipe(Pipe*, char*);
	char* readPipe(Pipe*);
	void destroyLoggerPipe();
	
#endif
