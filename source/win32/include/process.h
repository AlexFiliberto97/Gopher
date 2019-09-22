#ifndef PROCESS_H_
	
	#define PROCESS_H_
	#include <windows.h>

	typedef struct _Process {
		
		HANDLE hProcess;
		BOOL running;
		int socketIndex;
	} Process;
	
	int initProcess();
	int processIndex();
	int startProcess(char*, int, char**, int);
	BOOL processIsRunning(HANDLE);
	void* processCollector(void*);
	void destroyProcess();
	void stopProcessCollector();
	HANDLE getLoggerHandle();
	
#endif