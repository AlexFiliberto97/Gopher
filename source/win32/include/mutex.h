#ifndef MUTEX_H_
	
	#define MUTEX_H_
	#include <windows.h>

	typedef struct _MutexCV {
    	
    	HANDLE mutex;
    	HANDLE cv;
    	int err;
    	volatile int full;
	} MutexCV;
	
	MutexCV* createMutexCV();
	void mutexLock(HANDLE);
	void mutexUnlock(HANDLE);
	void mcvWait(MutexCV*);
	void destroyMutexCV(MutexCV*);

#endif