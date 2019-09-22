#include <windows.h>
#include "../include/thread.h"
#include "../include/mutex.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"
#include "../../common/include/socket.h"

/* Global variables */
Thread Threads[MAX_THREADS];
BOOL THREAD_COLLECTOR_ALIVE = TRUE;
MutexCV* mcvThread;

/* Function: initThread
*  Initialize threads.
*/
int initThread() {
	
	for (int i = 0; i < MAX_THREADS; i++) {
		Threads[i].Th = NULL;
		Threads[i].running = FALSE;
	}

	mcvThread = createMutexCV();
	if (mcvThread == NULL) return INIT_THREAD_ERR;

	return 0;
}

/* Function: joinThread
*  Wait for a given thread and collect it.
*/
void joinThread(HANDLE hThread) {
	
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

/* Function: startThread
*  Start a new thread.
*/
int startThread(void* (*f)(void *), void* data, int sock){
	
	mutexLock(mcvThread->mutex);
		for (int i = 0; i < MAX_THREADS; i++) {
			if (!Threads[i].running) {
				Threads[i].Th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) f, data, 0, NULL);
				if (Threads[i].Th == NULL) {
					mutexUnlock(mcvThread->mutex);
					return THREAD_ERROR;
				} else {
					Threads[i].running = TRUE;
					Threads[i].socketIndex = sock;
					mutexUnlock(mcvThread->mutex);
					return i;
				}
			}
		}
	mutexUnlock(mcvThread->mutex);
	return THREAD_UNAVAILABLE;
}

/* Function: startResponseThread
*  Start a new thread.
*/
HANDLE startResponseThread(void* (*f)(void *), void* data) {
	
	return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) f, data, 0, NULL);
}

/* Function: threadIsEnded
*  Check if a thread is alive.
*/
BOOL threadIsAlive(HANDLE hThread) {

	DWORD status = 0;
	if (!GetExitCodeThread(hThread, &status)) return TRUE;
	if (status == STILL_ACTIVE) return TRUE;
	return FALSE;
}

/* Function: threadCollector
*  Collect the ended threads.
*/
void* threadCollector(void *input) {

	while (THREAD_COLLECTOR_ALIVE) {
		mutexLock(mcvThread->mutex);
		for(int i = 0; i < MAX_THREADS; i++) {
			if(Threads[i].running && !threadIsAlive(Threads[i].Th)) {
				CloseHandle(Threads[i].Th);
				Threads[i].Th = NULL;
				Threads[i].running = FALSE;
				decrementSocket(Threads[i].socketIndex);
				Threads[i].socketIndex = -1;
				//printlog("Thread with id %d is now collected\n", i, NULL);
			} 
		}
		mutexUnlock(mcvThread->mutex);
		Sleep(100);
	}	
	return NULL;
}

/* Function: stopThreadCollector
*  Stop thread collector.
*/
void stopThreadCollector() {

	THREAD_COLLECTOR_ALIVE = FALSE;
}

/* Function: destroyThreads
*  Destroy all thread.
*/
void destroyThreads() {

	for(int i = 0; i < MAX_THREADS; i++) {
		if (Threads[i].running) {
			WaitForSingleObject(Threads[i].Th, INFINITE);
		}
		CloseHandle(Threads[i].Th);
	}
	destroyMutexCV(mcvThread);
}