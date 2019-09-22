#include <windows.h>
#include "../include/process.h"
#include "../include/mutex.h"
#include "../../common/include/error.h"
#include "../../common/include/utils.h"
#include "../../common/include/const.h"
#include "../../common/include/socket.h"

/* Global variables */
Process Processes[MAX_PROCESS];
BOOL PROCESS_COLLECTOR_ALIVE = TRUE;
MutexCV* mcvProcess;

/* Function: initProcess
*  Initialize processes.
*/
int initProcess() {

	for (int i = 0; i < MAX_PROCESS; i++) {
		Processes[i].running = FALSE;
		Processes[i].hProcess = NULL;
		Processes[i].socketIndex = -1;
	}
	
	mcvProcess = createMutexCV();
	if (mcvProcess == NULL) return INIT_PROCESS_ERR;

	return 0;
}

/* Function: processIndex
*  Return the first available index in Processes vector.
*/
int processIndex() {

	for (int i = 0; i < MAX_PROCESS; i++){
		if (!Processes[i].running){
			return i;
		}
	}
	return PROCESS_UNAVAILABLE;
}

/* Function: startProcess
*  Start a new process.
*/
int startProcess(char* name, int argc, char** args, int sock) {

	char* cmd = concatList(args, argc, ' ');
	if (cmd == NULL) {
		return ALLOC_ERROR;
	}

	mutexLock(mcvProcess->mutex);

		int index = processIndex();
		if (index < 0) {
			mutexUnlock(mcvProcess->mutex);
			return index;
		}
		
	mutexUnlock(mcvProcess->mutex);

	STARTUPINFO start_info;
	PROCESS_INFORMATION proc_info;
	ZeroMemory(&start_info, sizeof(start_info));
	ZeroMemory(&proc_info, sizeof(proc_info));
	GetStartupInfo(&start_info);
	BOOL success = CreateProcessA(name, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info);
	if (!success) {
		return PROCESS_ERROR;
	}
	
	free(cmd);
	Processes[index].hProcess = proc_info.hProcess;
	Processes[index].running = TRUE;
	Processes[index].socketIndex = sock;
	
	return proc_info.dwProcessId;
}

/* Function: processIsRunning
*  Check if a given process is still running.
*/
BOOL processIsRunning(HANDLE hProcess) {

    return WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT;
}

/* Function: processCollector
*  Collect the ended processes.
*/
void* processCollector(void *input) {

	while (PROCESS_COLLECTOR_ALIVE) {
		mutexLock(mcvProcess->mutex);
			for (int i = 0; i < MAX_PROCESS; i++) {
				if (Processes[i].running && !processIsRunning(Processes[i].hProcess)) {
					CloseHandle(Processes[i].hProcess);
					Processes[i].hProcess = NULL;
					Processes[i].running = FALSE;
					decrementSocket(Processes[i].socketIndex);
					Processes[i].socketIndex = -1;
				} 
			}
		mutexUnlock(mcvProcess->mutex);
		Sleep(100);
	}
	return NULL;
}

/* Function: stopProcessCollector		
*  Stop process collector.		
*/		
void stopProcessCollector() {		
			
	PROCESS_COLLECTOR_ALIVE = 0;		
}

/* Function: getLoggerHandle
*  Return the logger handle.
*/
HANDLE getLoggerHandle() {
	
	return Processes[0].hProcess;
}

/* Function: destroyProcess
*  Destroy all processes.
*/
void destroyProcess() {

	for (int i = 1; i < MAX_PROCESS; i++) {
		if (Processes[i].hProcess != NULL) {
			WaitForSingleObject(Processes[i].hProcess, INFINITE);
		}
		CloseHandle(Processes[i].hProcess);
	}
	destroyMutexCV(mcvProcess);
}