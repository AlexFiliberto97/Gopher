#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "../utils.h"
#include "../error.h"

#define MAX_PROCESS 1024

struct Process {
	
	pid_t pid;
	int running;
};

static struct Process Processes[MAX_PROCESS];
static int PROCESS_COLLECTOR_ALIVE = 1;

void initProcess() {
	
	for (int i = 0; i < MAX_PROCESS; i++) {
		Processes[i].running = 0;
		Processes[i].pid = -1;
	}
}

int processIndex() {
	
	for (int i = 0; i < MAX_PROCESS; i++) {
		if (Processes[i].running == 0) {
			return i;
		}
	}
	return PROCESS_UNAVAILABLE;
}

int startProcess(void* (*f)(void*), void* data) {

	int index = processIndex();
	if (index == PROCESS_UNAVAILABLE) return index;

	pid_t pid = fork();
	if (pid < 0) {
		return FORK_ERROR;
	} else if (pid > 0) {
		Processes[index].pid = pid;
		Processes[index].running = 1;
		return pid;
	}

	signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
	f((void*) data);
	exit(0);
}

void* processCollector(void* input) {

	int status;
	while (PROCESS_COLLECTOR_ALIVE == 1) {
		for (int i = 0; i < MAX_PROCESS; i++) {
			if (Processes[i].running == 1 && waitpid(Processes[i].pid, &status, WNOHANG) == -1) {
				Processes[i].running = 0;
			}
		}
	}
	return NULL;
}

void stopProcessCollector() {
	
	PROCESS_COLLECTOR_ALIVE = 0;
}

void destroyProcess() {
	
	int status, err;
	for (int i = 1; i < MAX_PROCESS; i++) {
		if (Processes[i].pid > 0) {
			err = waitpid(Processes[i].pid, &status, 0);
			if (err == -1) throwError(1, WAITPID_ERROR);
			Processes[i].pid = -1;
		}
	}
}