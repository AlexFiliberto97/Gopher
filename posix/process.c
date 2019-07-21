// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

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
	return -1;
}


int startProcess(void* (*f)(void*), int argc, char** argv) {

	int index = processIndex();

	if (index == -1) {
		return -1;
	}

	pid_t pid = fork();

	if (pid < 0) {
		perror("Errore in fork()\n");
		return -1;
	} else if (pid > 0) {
		Processes[index].pid = pid;
		Processes[index].running = 1;
		return pid;
	}

	signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    
	f((void*) argv);
	exit(0);

	return pid;

}


void* processCollector(void* input) {

	int status;

	while (PROCESS_COLLECTOR_ALIVE == 1) {
		for (int i = 0; i < MAX_PROCESS; i++) {
			if (Processes[i].running == 1 && waitpid(Processes[i].pid, &status, WNOHANG) == -1) {
				Processes[i].running = 0;
				printlog("Process with id %d is now collected\n", i, NULL);
			}
		}
	}

	return NULL;
	
}

void stopProcessCollector() {
	PROCESS_COLLECTOR_ALIVE = 0;
}


//Destroy the process environment
void destroyProcess() {
	int status;
	for (int i = 1; i < MAX_PROCESS; i++) {
		if (Processes[i].pid > 0) {
			waitpid(Processes[i].pid, &status, 0);
			Processes[i].pid = -1;
		}
	}
}