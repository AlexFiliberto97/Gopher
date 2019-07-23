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
#include "../error.h"

struct Mutex {
    
    pthread_cond_t* cond1;
    pthread_cond_t* cond2;
    pthread_mutex_t* mutex;
    volatile int* full;
};

struct Mutex* shared_mutex;

int initMutex() {

	pthread_cond_t* cond1;
	pthread_cond_t* cond2;
	pthread_mutex_t* mutex;
	int* full;

	mutex = (pthread_mutex_t*) create_shared_memory(sizeof(pthread_mutex_t));
    if (mutex == MAP_FAILED) return CREATE_MAPPING;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    cond1 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond1 == MAP_FAILED) return CREATE_MAPPING;

    pthread_condattr_t cond1_attr;
    pthread_condattr_init(&cond1_attr);
    pthread_condattr_setpshared(&cond1_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond1, &cond1_attr);
    pthread_condattr_destroy(&cond1_attr);

    cond2 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond2 == MAP_FAILED) return CREATE_MAPPING;

    pthread_condattr_t cond2_attr;
    pthread_condattr_init(&cond2_attr);
    pthread_condattr_setpshared(&cond2_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond2, &cond2_attr);
    pthread_condattr_destroy(&cond2_attr);

    full = (int*) create_shared_memory(sizeof(int));
    if (full == MAP_FAILED) return CREATE_MAPPING;

    *full = 0;
    shared_mutex = (struct Lock*) create_shared_memory(sizeof(struct Lock));
    if (shared_mutex == MAP_FAILED) return CREATE_MAPPING;
    
    shared_mutex->mutex = mutex;
    shared_mutex->cond1 = cond1;
    shared_mutex->cond2 = cond2;
    shared_mutex->full = full;

    return 0;
}

int destroySharedMutex() {
    
    int err = 0;
    err |= free_shared_memory((void*) shared_mutex->cond1, sizeof(shared_mutex->cond1));
    err |= free_shared_memory((void*) shared_mutex->cond2, sizeof(shared_mutex->cond2));
    err |= free_shared_memory((void*) shared_mutex->mutex, sizeof(shared_mutex->mutex));
    err |= free_shared_memory((void*) shared_mutex->full, sizeof(shared_mutex->full));
    err |= free_shared_memory((void*) shared_mutex, sizeof(shared_mutex));
    return err;
}