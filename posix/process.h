#ifndef PROCESS_H_
	
	#define PROCESS_H_
	
	#include "process.c"
	
	// int startProcess(void* (*f)(void*), int, char**);
	int startProcess(void* (*f)(void*), void*);
	int processIndex();
	void* processCollector(void*);

#endif