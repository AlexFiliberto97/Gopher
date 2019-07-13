#ifndef THREAD_H_
	
	#define THREAD_H_
	
	#include <windows.h>
	#include "thread.c"

	void initThread();
	int startThread(void*(*f)(void*), void*);
	int threadIsEnded(HANDLE);
	void* threadCollector(void*);

#endif