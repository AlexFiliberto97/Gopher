#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "../include/process.h"
#include "../include/mutex.h"
#include "../../common/include/utils.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"
#include "../../common/include/socket.h"

/* Global variables */
Process Processes[MAX_PROCESS];
MutexCV* mcvProcess;
int PROCESS_COLLECTOR_ALIVE = 1;

/* Function: initProcess
*  Initialize processes.
*/
int initProcess() {

	for (int i = 0; i < MAX_PROCESS; i++) {
		Processes[i].running = 0;
		Processes[i].pid = -1;
	}

	mcvProcess = createMutexCV();
	if (mcvProcess == NULL) return INIT_PROCESS_ERR;

	return 0;
}

/* Function: processIndex
*  Return the first available index in Processes vector.
*/
int processIndex() {
	
	for (int i = 0; i < MAX_PROCESS; i++) {
		if (Processes[i].running == 0) {
			return i;
		}
	}
	return PROCESS_UNAVAILABLE;
}

/* Function: startProcess
*  Start a new process.
*/
int startProcess(void* (*f)(void*), void* data, int sock) {

	pid_t pid = fork();
	if (pid < 0) {
		return FORK_ERROR;
	} else if (pid > 0) {
		mutexLock(mcvProcess->mutex);
			
			int index = processIndex();
			if (index < 0) return index;
			Processes[index].pid = pid;
			Processes[index].running = 1;
			Processes[index].socketIndex = sock;
		
		mutexUnlock(mcvProcess->mutex);
		return pid;
	}

	signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
	f((void*) data);
	exit(0);
}

/* Function: processCollector
*  Collect the ended processes.
*/
void* processCollector(void* input) {

	int status;
	while (PROCESS_COLLECTOR_ALIVE == 1) {
		mutexLock(mcvProcess->mutex);
		for (int i = 0; i < MAX_PROCESS; i++) {
			if (Processes[i].running == 1 && waitpid(Processes[i].pid, &status, WNOHANG) == -1) {
				//printlog("Process with id %d is now collected\n", i, NULL);
				Processes[i].running = 0;
				decrementSocket(Processes[i].socketIndex);
				Processes[i].socketIndex = -1;
			}
		}
		mutexUnlock(mcvProcess->mutex);
		usleep(100000);
	}
	return NULL;
}

/* Function: stopProcessCollector
*  Stop process collector.
*/
void stopProcessCollector() {
	
	PROCESS_COLLECTOR_ALIVE = 0;
}

/* Function: destroyProcess
*  Destroy all processes.
*/
void destroyProcess() {
	
	int status = 0, err = 0;
	for (int i = 1; i < MAX_PROCESS; i++) {
		if (Processes[i].pid > 0) {
			err = waitpid(Processes[i].pid, &status, 0);
			if (err == -1) throwError(WAITPID_ERROR);
			Processes[i].pid = -1;
		}
	}
	destroyMutexCV(mcvProcess);
}