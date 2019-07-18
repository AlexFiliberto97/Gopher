// #define _GNU_SOURCE
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


int SERVER_ALIVE = 0;


void init_env() {
	SERVER_ALIVE = 1;
	initPipes();
	initThread();
	initProcess();
}


//Initialize the environment
int start_env(){

	int err;

	// initPipes();

	int success = createPipe("LOGGER_PIPE");
	if (success == -1) return -1;

	err = initLocking();

	int pid = startProcess(logger, 0, NULL);
	if (pid < 0) return -1;

	//Creating the garbage collector for threads e processes
	startThread(threadCollector, NULL, 1);
	startThread(processCollector, NULL, 1);

	return 0;
}

int clean_env() {
	// KILL LOGGER
	destroyProcess();
	destroyThreads();
	destroyPipes();
	return 0;
}