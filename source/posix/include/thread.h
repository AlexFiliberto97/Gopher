#ifndef THREAD_H_
	
	#define THREAD_H_
	
	typedef struct _Thread {
		
		pthread_t Th;
		int running;
		int id;
		int collect;
		int socketIndex;
	} Thread;

	int initThread();
	void* threadCollector(void*);
	int startThread(void* (*f)(void*), void*, int);
	pthread_t startResponseThread(void* (*f)(void*), void*);
	void joinThread(pthread_t);
	void stopThreadCollector();
	void destroyThreads();
	
#endif
