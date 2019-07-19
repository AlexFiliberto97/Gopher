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
int LOGGER_PID;


void init_env() {
	SERVER_ALIVE = 1;
	// initLoggerPipe();
	initThread();
	initProcess();
}


int start_env(){

	int err;

	// initPipes();

	struct Pipe* loggerPipe = createLoggerPipe();

	if (loggerPipe->err != 0) return -1;

	// int success = createPipe("LOGGER_PIPE");
	// if (success == -1) return -1;

	err = initLocking();

	// LOGGER_PID = startProcess(logger, 0, NULL);
	// if (LOGGER_PID < 0) return -1;
	LOGGER_PID = startProcess(logger, 1, (void*) loggerPipe);
	if (LOGGER_PID < 0) return -1;

	//Creating the garbage collector for threads e processes
	startThread(threadCollector, NULL, 1);
	startThread(processCollector, NULL, 1);

	return 0;
}

int clean_env() {
	kill(LOGGER_PID, SIGKILL);
	destroySharedLock();
	freeServerOptions();
	stopProcessCollector();
	destroyProcess();
	stopThreadCollector();
	destroyThreads();
	// destroyPipes();
	destroyLoggerPipe();
	return 0;
}

//Initialize the environment
// int start_env(){

// 	int err;

// 	// initPipes();

// 	int success = createPipe("LOGGER_PIPE");
// 	if (success == -1) return -1;

// 	err = initLocking();

// 	LOGGER_PID = startProcess(logger, 0, NULL);
// 	if (LOGGER_PID < 0) return -1;

// 	//Creating the garbage collector for threads e processes
// 	startThread(threadCollector, NULL, 1);
// 	startThread(processCollector, NULL, 1);

// 	return 0;
// }

// int clean_env() {
// 	kill(LOGGER_PID, SIGKILL);
// 	destroySharedLock();
// 	stopProcessCollector();
// 	destroyProcess();
// 	stopThreadCollector();
// 	destroyThreads();
// 	destroyPipes();
// 	return 0;
// }