#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PROCESS 1024


struct Process {
	pid_t pid;
	int running;
};


static struct Process Processes[MAX_PROCESS];


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
	} else if (pid == 0) {
		// PROCESSO FIGLIO
		f((void*) argv);
		exit(0);
	} else {
		Processes[index].pid = pid;
		Processes[index].running = 1;
	}

	return pid;

}


void* processCollector(void* input) {

	int status;

	while (1) {
		for (int i = 0; i < MAX_PROCESS; i++) {
			if (Processes[i].running == 1 && waitpid(Processes[i].pid, &status, WNOHANG) == -1) {
				Processes[i].running = 0;
				printf("Process with id %d is now collected\n", i);
			}
		}
		usleep(500000);
	}
}