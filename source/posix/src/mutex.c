#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include "../include/mutex.h"
#include "../include/utils_posix.h"
#include "../../common/include/error.h"

/* Global variables */
MutexCV* mcvLogger = NULL;

/* Function: createMutexCV
*  Create new mutex.
*/
MutexCV* createMutexCV() {

	pthread_cond_t* cond1;
	pthread_cond_t* cond2;
	pthread_mutex_t* mutex;
	
	mutex = (pthread_mutex_t*) create_shared_memory(sizeof(pthread_mutex_t));
    if (mutex == MAP_FAILED) {
        throwError(CREATE_MAPPING);
        return NULL;
    }

    pthread_mutexattr_t mutex_attr;  //Set mutex in shared memory
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);

    cond1 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond1 == MAP_FAILED) {
    	free_shared_memory(mutex, sizeof(pthread_mutex_t));
        throwError(CREATE_MAPPING);
        return NULL;
    }

    pthread_condattr_t cond1_attr;
    pthread_condattr_init(&cond1_attr);
    pthread_condattr_setpshared(&cond1_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond1, &cond1_attr);
    pthread_condattr_destroy(&cond1_attr);

    cond2 = (pthread_cond_t*) create_shared_memory(sizeof(pthread_cond_t));
    if (cond2 == MAP_FAILED) {
    	free_shared_memory(cond1, sizeof(pthread_mutex_t));
    	free_shared_memory(mutex, sizeof(pthread_mutex_t));
        throwError(CREATE_MAPPING);
        return NULL;
    }

    pthread_condattr_t cond2_attr;
    pthread_condattr_init(&cond2_attr);
    pthread_condattr_setpshared(&cond2_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(cond2, &cond2_attr);
    pthread_condattr_destroy(&cond2_attr);

    MutexCV* lock = (MutexCV*) create_shared_memory(sizeof(MutexCV));
    if (lock == MAP_FAILED) {
    	free_shared_memory(cond1, sizeof(pthread_mutex_t));
    	free_shared_memory(cond2, sizeof(pthread_mutex_t));
    	free_shared_memory(mutex, sizeof(pthread_mutex_t));
        throwError(CREATE_MAPPING);
        return NULL;
    }

    lock->mutex = mutex;
    lock->cond1 = cond1;
    lock->cond2 = cond2;
    lock->full = 0;
    return lock;
}

/* Function: mutexLock
*  Lock the given mutex.
*/
void mutexLock(pthread_mutex_t* mutex) {
    
    pthread_mutex_lock(mutex);
}

/* Function: mutexUnlock
*  Unlock the given mutex.
*/
void mutexUnlock(pthread_mutex_t* mutex) {
    
    pthread_mutex_unlock(mutex);
}

/* Function: destroyMutexCV
*  Destroy given mutex.
*/
void destroyMutexCV(MutexCV* mutex) {
    
    free_shared_memory((void*) mutex->cond1, sizeof(mutex->cond1));
    free_shared_memory((void*) mutex->cond2, sizeof(mutex->cond2));
    free_shared_memory((void*) mutex->mutex, sizeof(mutex->mutex));
    free_shared_memory((void*) mutex, sizeof(mutex));
}