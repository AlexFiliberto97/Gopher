#ifndef PROCESS_H_
	
	#define PROCESS_H_
	
	#include <windows.h>
	#include "process.c"
	
	void initProcess();
	int processIndex();
	int startProcess(char*, int, char**);
	BOOL processIsRunning(HANDLE);
	void* processCollector(void*);
	void destroyProcess();

#endif