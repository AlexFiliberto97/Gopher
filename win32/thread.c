#include <windows.h>
#include "../error.h"

#define MAX_THREADS 1024

struct Thread {
	HANDLE Th;
	BOOL running;
};

static struct Thread Threads[MAX_THREADS];

void initThread() {
	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = NULL;
		Threads[i].running = FALSE;
	}
}


int joinCollect(int id) {

	int err;
	WaitForSingleObject(Threads[id].Th, INFINITE);
	
	Threads[id].Th = 0;
	Threads[id].running = 0;	
	return 0;

}


int startThread(void* (*f)(void *), void* data, int ignore){
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
	return THREAD_UNAVAILABLE;
}

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


void destroyThreads() {

	for(int i = 0; i < MAX_THREADS; i++) {
		TerminateThread(Threads[i].Th, 0);
		CloseHandle(Threads[i].Th);
	}
}