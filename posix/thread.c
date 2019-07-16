#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_THREADS 1024


struct Thread {
	pthread_t Th;
	int running;
	int id;
};


static struct Thread Threads[MAX_THREADS];


// pthread_t getThread() {
	
// }


void* threadCollector(void* input) {
	int i = 0;
	while (i++ < 5) {
		for (int i = 0; i < MAX_THREADS; i++) {
			if (Threads[i].running == 1 && pthread_tryjoin_np(Threads[i].Th, NULL) == 0) {
				Threads[i].Th = 0;
				Threads[i].running = 0;
				printf("Thread with id %d is now collected\n", Threads[i].id);
			} 
		}
		sleep(1);
	}
}


pthread_t startThread(void* (*f)(void*), void* data) {
	for (int i = 0; i < MAX_THREADS; i++) {
		if (!Threads[i].running) {
			if (pthread_create(&Threads[i].Th, NULL, f, data)) {
				return Threads[i].Th;
			} else {
				Threads[i].running = 1;
				Threads[i].id = i;
				return -1;
			}
		}
	}
	return -1;
}