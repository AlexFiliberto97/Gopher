#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h> 
#include <signal.h>
#include "pipe.h"
#include "thread.h"
#include "process.h"
#include "logger.h"


//Initialize the environment
int start_env(){

	int err;

	initPipes();

	err = createPipe("LOGGER_PIPE");
	if(err != 0) return -1; //Error: creating Pipes

	err = initLocking();

	// int pid = startProcess(logger, 0, NULL);
	// if (pid < 0) return -1;

	//Creating the garbage collector for threads e processes
	startThread(threadCollector, NULL);
	// startThread(processCollector, NULL);

	return 0;
}

// void* foo(void* input) { 

// 	pthread_mutex_lock(shared_lock->mutex);

// 		while (*(shared_lock->full) == 1) pthread_cond_wait(shared_lock->cond1, shared_lock->mutex);

// 	    writePipe("LOGGER_PIPE", "Dio sborra.\n");

// 	    *(shared_lock->full) = 1;

//         pthread_cond_signal(shared_lock->cond2); 

//     pthread_mutex_unlock(shared_lock->mutex);


//     return NULL; 
// } 


// int main() {

// 	int err;

// 	err = initLocking();
// 	if (err != 0) return -1;

// 	createPipe("LOGGER_PIPE");

// 	int pid = startProcess(logger, 0, NULL);

// 	for (int i = 0; i < 10; i++) {
//         startProcess(foo, 0, NULL); 
//     }

// 	sleep(2);

// 	kill(pid, SIGKILL);

// 	return 0;

// }