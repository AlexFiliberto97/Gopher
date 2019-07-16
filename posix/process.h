#ifndef PROCESS_H_
	
	#define PROCESS_H_
	
	#include "process.c"
	
	int startProcess(void* (*f)(void*), int, char**);
	int processIndex();
	void* processCollector(void*);

#endif