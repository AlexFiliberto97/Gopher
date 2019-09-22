#ifndef PIPE_H_
	
	#define PIPE_H_
	#include <windows.h>

	typedef struct _Pipe {
		
		HANDLE hRead;
		HANDLE hWrite;
	} Pipe;

	int createLoggerPipe();
	HANDLE getReader();
	HANDLE getWriter();
	void setPipe(HANDLE, HANDLE);
	int writePipe(char*);
	char* readPipe();
	void destroyLoggerPipe();
	
#endif