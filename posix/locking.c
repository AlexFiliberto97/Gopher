#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mman.h>
#include "process.h"
#include "thread.h"
#include <sys/mman.h>


struct Lock {
    pthread_cond_t* cond1;
    pthread_cond_t* cond2;
    pthread_mutex_t* mutex;
    volatile int* full;
};


struct Lock* shared_lock;


int initLocking() {

	pthread_cond_t* cond1;
	pthread_cond_t* cond2;
	pthread_mutex_t* mutex;
	int* full;

	mutex = (pthread_mutex_t*) create_shared_memory(sizeof(pthread_mutex_t));
    if (mutex == MAP_FAILED)
        return -1;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    cond1 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond1 == MAP_FAILED)
        return -1;

    pthread_condattr_t cond1_attr;
    pthread_condattr_init(&cond1_attr);
    pthread_condattr_setpshared(&cond1_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond1, &cond1_attr);
    pthread_condattr_destroy(&cond1_attr);

    cond2 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond2 == MAP_FAILED)
        return -1;

    pthread_condattr_t cond2_attr;
    pthread_condattr_init(&cond2_attr);
    pthread_condattr_setpshared(&cond2_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond2, &cond2_attr);
    pthread_condattr_destroy(&cond2_attr);

    full = (int*) create_shared_memory(sizeof(int));
    if (full == MAP_FAILED)
        return -1;

    *full = 0;

    shared_lock = (struct Lock*) create_shared_memory(sizeof(struct Lock));
    if (shared_lock == MAP_FAILED)
        return -1;
    shared_lock->mutex = mutex;
    shared_lock->cond1 = cond1;
    shared_lock->cond2 = cond2;
    shared_lock->full = full;

    return 0;

}