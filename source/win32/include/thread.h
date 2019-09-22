#ifndef THREAD_H_
	
	#define THREAD_H_
	#include <windows.h>
	
	typedef struct _Thread {
		
		HANDLE Th;
		BOOL running;
		int socketIndex;
	} Thread;

	int initThread();
	void joinThread(HANDLE);
	int startThread(void* (*f)(void *), void*, int);
	HANDLE startResponseThread(void* (*f)(void *), void*);
	BOOL threadIsAlive(HANDLE);
	void* threadCollector(void*);
	void destroyThreads();
	void stopThreadCollector();
	
#endif