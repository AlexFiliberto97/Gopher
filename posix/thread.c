// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS 1024


struct Thread {
	pthread_t Th;
	int running;
	int id;
	int collect;
};


static struct Thread Threads[MAX_THREADS];


void initThread() {
	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = -1;
		Threads[i].running = 0;
	}
}


int joinCollect(pthread_t th) {

	int err;
	err = pthread_join(th, NULL);

	if (err != 0) return -1;

	for (int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].Th == th) {
			Threads[i].Th = 0;
			Threads[i].running = 0;
			printf("Thread with id %d is now collected\n", Threads[i].id);
			return 0;
		}
	}

	return -1;

}


void* threadCollector(void* input) {
	while (1) {
		for (int i = 0; i < MAX_THREADS; i++) {
			if (Threads[i].collect == 1 && Threads[i].running == 1 && pthread_tryjoin_np(Threads[i].Th, NULL) == 0) {
				Threads[i].Th = 0;
				Threads[i].running = 0;
				printf("Thread with id %d is now collected\n", Threads[i].id);
			} 
		}
	}
}


pthread_t startThread(void* (*f)(void*), void* data, int collect) {
	for (int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].running == 0) {
			if (pthread_create(&Threads[i].Th, NULL, f, data) == 0) {
				Threads[i].running = 1;
				Threads[i].id = i;
				Threads[i].collect = collect;
				return Threads[i].Th;
			} else {
				return -1;
			}
		}
	}
	return -1;
}


void destroyThreads() {
	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_cancel(Threads[i].Th);
	}
}