#define _GNU_SOURCE
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <signal.h>
#include <string.h>
#include "process.h"
#include "thread.h"
#include <sys/mman.h>


struct Lock {
    pthread_cond_t* cond1;
    pthread_cond_t* cond2;
    pthread_mutex_t* mutex;
    volatile int* full;
};


void* create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(NULL, size, protection, visibility, -1, 0);
}


void* foo(void* input) { 

    struct Lock* lock = (struct Lock*) input;

    pthread_mutex_lock(lock->mutex);

        while (*(lock->full) == 1) pthread_cond_wait(lock->cond1, lock->mutex);

        printf("PORCO DIACCIO write\n");
        *(lock->full) = 1;

        pthread_cond_signal(lock->cond2); 

    pthread_mutex_unlock(lock->mutex);


    return NULL; 
} 

void* asdfiasdji(void* input) {

    struct Lock* lock = (struct Lock*) input;

    while (1) {

        pthread_mutex_lock(lock->mutex);

            while (*(lock->full) == 0) pthread_cond_wait(lock->cond2, lock->mutex);

            printf("MADONNACCIA read\n");
            *(lock->full) = 0;

            pthread_cond_signal(lock->cond1); 

        pthread_mutex_unlock(lock->mutex);

    }

}


int main() { 

    // Initialize mutex and condition variables //
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

    //////////////////////////////////////////////

    struct Lock* shared_lock = (struct Lock*) create_shared_memory(sizeof(struct Lock));
    shared_lock->mutex = mutex;
    shared_lock->cond1 = cond1;
    shared_lock->cond2 = cond2;
    shared_lock->full = full;

    int pid = startProcess(asdfiasdji, 1, (void*) shared_lock); 
  
    for (int i = 0; i < 10; i++) {
        startProcess(foo, 1, (void*) shared_lock); 
    }

    // startThread(asdfiasdji, (void*) shared_lock); 
  
    // for (int i = 0; i < 10; i++) {
    //     startThread(foo, (void*) shared_lock); 
    // }

    sleep(2);

    kill(pid, SIGKILL);

    return 0; 

} 