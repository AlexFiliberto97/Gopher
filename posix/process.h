#ifndef PROCESS_H_
	
	#define PROCESS_H_
	
	#include "process.c"
	
	void initProcess();
	int processIndex();
	int startProcess(void* (*f)(void*), void*);
	void* processCollector(void*);
	void stopProcessCollector();
	void destroyProcess();

#endif