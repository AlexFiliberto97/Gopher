#ifndef MUTEX_H_

    #define MUTEX_H_

	typedef struct _MutexCV {
   	 	
   	 	pthread_cond_t* cond1;
    	pthread_cond_t* cond2;
   	 	pthread_mutex_t* mutex;
    	volatile int full;
	} MutexCV;

	/* Global variables */
	extern MutexCV* mcvLogger;
	
	MutexCV* createMutexCV();
	void destroyMutexCV();
	int initSharedMutexCV();
	void mutexLock(pthread_mutex_t*);
	void mutexUnlock(pthread_mutex_t*);

#endif