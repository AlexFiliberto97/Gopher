#include <windows.h>
#include "../error.h"

#define MAX_THREADS 1024

struct Thread {
	
	HANDLE Th;
	BOOL running;
};

static struct Thread Threads[MAX_THREADS];

//Init threads environment
void initThread() {

	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = NULL;
		Threads[i].running = FALSE;
	}
}

//Start a new thread
int startThread(void* (*f)(void *), void* data){

	for (int i = 0; i < MAX_THREADS; i++) {
		if (!Threads[i].running) {
			Threads[i].Th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)f, data, 0, NULL);
			if (Threads[i].Th == NULL) {
				return THREAD_ERROR;
			} else {
				Threads[i].running = TRUE;
				return (int) Threads[i].Th;
			}
		}
	}
	return errorCode(2, THREAD_ERROR, THREAD_UNAVAILABLE);
}

//Check if  athread is ended
int threadIsEnded(HANDLE hThread) {

	DWORD status;
	int success = GetExitCodeThread(hThread, &status);

	if(success == 0) return -1;
	if(status == STILL_ACTIVE) {
		return 0;
	} else if (status == 0) {
		return -1;
	}
}

//Collect running threads
void* threadCollector(void *input) {

	while (TRUE) {
		for(int i = 0; i < MAX_THREADS; i++) {
			if(Threads[i].running && threadIsEnded(Threads[i].Th) == -1) {
				CloseHandle(Threads[i].Th);
				Threads[i].Th = NULL;
				Threads[i].running = FALSE;
			} 
		}	
	}	
}

//Destroy threads
void destroyThreads() {

	for(int i = 0; i < MAX_THREADS; i++) {
		TerminateThread(Threads[i].Th, 0);
		CloseHandle(Threads[i].Th);
	}
}