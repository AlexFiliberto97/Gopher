#ifndef THREAD_H_
	
	#define THREAD_H_
	
	#include <stdio.h>
	#include <stdlib.h>
	#include "thread.c"
	
	void initThread();
	int joinCollect(int);
	void* threadCollector(void*);
	int startThread(void* (*f)(void*), void*, int);
	void stopThreadCollector();
	void destroyThreads();
	
#endif
