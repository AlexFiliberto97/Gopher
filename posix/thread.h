#ifndef THREAD_H_
	
	#define THREAD_H_
	
	#include <stdio.h>
	#include <stdlib.h>
	#include "thread.c"
	
	void* threadCollector(void*);
	pthread_t startThread(void* (*f)(void*), void*);
	
#endif
