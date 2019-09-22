#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/thread.h"
#include "../include/mutex.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"
#include "../../common/include/socket.h"

/* Global variables */
Thread Threads[MAX_THREADS];
int THREAD_COLLECTOR_ALIVE = 1;
MutexCV* mcvThread;

/* Function: initThread
*  Initialize threads.
*/
int initThread() {
	
	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = -1;
		Threads[i].running = 0;
	}

	mcvThread = createMutexCV();
	if (mcvThread == NULL) return INIT_THREAD_ERR;

	return 0;
}

/* Function: threadCollector
*  Collect the ended threads.
*/
void* threadCollector(void* input) {

	while (THREAD_COLLECTOR_ALIVE == 1) {
		mutexLock(mcvThread->mutex);
		for (int i = 0; i < MAX_THREADS; i++) {
			if (Threads[i].running == 1 && pthread_tryjoin_np(Threads[i].Th, NULL) == 0) {
				Threads[i].Th = 0;
				Threads[i].running = 0;
				decrementSocket(Threads[i].socketIndex);
				Threads[i].socketIndex = -1;
				//printlog("Thread with id %d is now collected\n", Threads[i].id, NULL);
			} 
		}
		mutexUnlock(mcvThread->mutex);
		usleep(100000);
	}
	return NULL;
}

/* Function: startThread
*  Start a new thread.
*/
int startThread(void* (*f)(void*), void* data, int sock) {
	
	mutexLock(mcvThread->mutex);
		
		for (int i = 0; i < MAX_THREADS; i++) {
			if (Threads[i].running == 0) {
				if (pthread_create(&Threads[i].Th, NULL, f, data) == 0) {
					Threads[i].running = 1;
					Threads[i].id = i;
					Threads[i].socketIndex = sock;
					mutexUnlock(mcvThread->mutex);
					return Threads[i].id;
				} else {
					mutexUnlock(mcvThread->mutex);
					return THREAD_ERROR;
				}
			}
		}
	
	mutexUnlock(mcvThread->mutex);
	return THREAD_UNAVAILABLE;
}

/* Function: startThread
*  Start a new response thread.
*/
pthread_t startResponseThread(void* (*f)(void*), void* data) {
	
	pthread_t id;
	int err = pthread_create(&id, NULL, f, data);
	if (err != 0) {
		throwError(err);
		return THREAD_ERROR;
	}
	return id;
}

/* Function: joinCollect
*  Wait for a given thread and collect it (ignore return value).
*/
void joinThread(pthread_t id) {
	
	pthread_join(id, NULL);
}

/* Function: stopThreadCollector
*  Stop thread collector.
*/
void stopThreadCollector() {
	
	THREAD_COLLECTOR_ALIVE = 0;
}

/* Function: destroyThreads
*  Destroy all thread.
*/
void destroyThreads() {
	
	int err;
	for (int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].running == 1) {
			err = pthread_join(Threads[i].Th, NULL);
			if (err != 0) {
				throwError(THREAD_JOIN_ERROR);
			}
		}
	}
	destroyMutexCV(mcvThread);
}