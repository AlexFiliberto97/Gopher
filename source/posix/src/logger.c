#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "../include/pipe.h"
#include "../include/mutex.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

void* logger(void* input) {

	Pipe* loggerPipe = (Pipe*) input;
	FILE* logFile;
	char* msg = NULL;

	while (1) {

		mutexLock(mcvLogger->mutex);

            while (mcvLogger->full == 0) pthread_cond_wait(mcvLogger->cond2, mcvLogger->mutex);

	        msg = readPipe(loggerPipe);
			if (msg == NULL) continue;
			if (strcmp(msg, "TERMINATE_LOGGER") == 0) {
				free(msg);
				break;
			}

			logFile = fopen(LOG_FILE_PATH, "a+b");
			fwrite(msg, 1, strlen(msg), logFile);
			fclose(logFile);
			free(msg);
            mcvLogger->full = 0;
            pthread_cond_signal(mcvLogger->cond1); 

        mutexUnlock(mcvLogger->mutex);
	}
	return NULL;
}
