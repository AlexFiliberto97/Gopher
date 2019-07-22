#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../error.h"

#define MAX_THREADS 1024

struct Thread {
	pthread_t Th;
	int running;
	int id;
	int collect;
};

static struct Thread Threads[MAX_THREADS];
static int THREAD_COLLECTOR_ALIVE = 1;

void initThread() {
	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = -1;
		Threads[i].running = 0;
	}
}

int joinCollect(int id) {

	int err;
	err = pthread_join(Threads[id].Th, NULL);
	if (err != 0) return -1;
	
	Threads[id].Th = 0;
	Threads[id].running = 0;	
	return 0;

}

void* threadCollector(void* input) {
	while (THREAD_COLLECTOR_ALIVE == 1) {
		for (int i = 0; i < MAX_THREADS; i++) {
			if (Threads[i].collect == 1 && Threads[i].running == 1 && pthread_tryjoin_np(Threads[i].Th, NULL) == 0) {
				Threads[i].Th = 0;
				Threads[i].running = 0;
				printlog("Thread with id %d is now collected\n", Threads[i].id, NULL);
			} 
		}
	}

	return NULL;

}

int startThread(void* (*f)(void*), void* data, int collect) {
	for (int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].running == 0) {
			if (pthread_create(&Threads[i].Th, NULL, f, data) == 0) {
				Threads[i].running = 1;
				Threads[i].id = i;
				Threads[i].collect = collect;
				return Threads[i].id;
			} else {
				return THREAD_ERROR;
			}
		}
	}
	return THREAD_UNAVAILABLE;
}


void stopThreadCollector() {
	THREAD_COLLECTOR_ALIVE = 0;
}


void destroyThreads() {
	int err;
	for (int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].running == 1) {
			err = pthread_join(Threads[i].Th, NULL);
			if (err != 0) {
				throwError(1, THREAD_JOIN_ERROR);
			}
		}
	}
}