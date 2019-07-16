#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <signal.h>
#include "pipe.h"
#include "locking.h"


#define PIPE_PACKET_LENGTH 32


void* logger(void* input) {

	FILE* logFile;
	char* msg;

	while (1) {

		pthread_mutex_lock(shared_lock->mutex);

            while (*(shared_lock->full) == 0) pthread_cond_wait(shared_lock->cond2, shared_lock->mutex);

	        msg = readPipe("LOGGER_PIPE");

			if (msg != NULL) {
				logFile = fopen("log.txt", "a+b");
				fwrite(msg, 1, strlen(msg), logFile);
				fclose(logFile);
				free(msg);
			}

            *(shared_lock->full) = 0;

            pthread_cond_signal(shared_lock->cond1); 

        pthread_mutex_unlock(shared_lock->mutex);
		
	}

	return NULL;

}