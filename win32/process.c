#include <windows.h>
#include "../utils.h"

#define MAX_PROCESS 1024

struct Process{

	HANDLE hProcess;
	BOOL running;
};

static struct Process Processes[MAX_PROCESS];

//Initialize the process environment 
void initProcess() {

	for (int i = 0; i < MAX_PROCESS; i++) {
		Processes[i].running = FALSE;
		Processes[i].hProcess = NULL;
	}
}

//Return the first available index for a new process
int processIndex() {

	for (int i = 0; i < MAX_PROCESS; i++){
		if (!Processes[i].running){
			return i;
		}
	}
	return -1;
}

//Start a new win32 process
int startProcess(char* name, int argc, char** args) {

	char* cmd = concatList(args, argc, ' ');
	if (cmd == NULL) return -1;
 	
	int index = processIndex();
	if (index == -1) return -1;

	STARTUPINFO start_info;
    PROCESS_INFORMATION proc_info;
    ZeroMemory(&start_info, sizeof(start_info));
    ZeroMemory(&proc_info, sizeof(proc_info));
    GetStartupInfo(&start_info);
    BOOL success = CreateProcessA(name, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &start_info, &proc_info);

    if (!success){
    	printf("not success %d\n", GetLastError());
    	return -1;
    } 
	
	free(cmd);
	Processes[index].hProcess = proc_info.hProcess;
	Processes[index].running = TRUE;
	return 0;
}

//Return TRUE if the process hProcess is running
BOOL processIsRunning(HANDLE hProcess) {

    return WaitForSingleObject(hProcess, 0) == WAIT_TIMEOUT;
}

//Collect the ended processes
void* processCollector(void *input){

	while (TRUE) {
		for (int i = 0; i < MAX_PROCESS; i++) {
			if (Processes[i].running && !processIsRunning(Processes[i].hProcess)) {
				CloseHandle(Processes[i].hProcess);
				Processes[i].hProcess = NULL;
				Processes[i].running = FALSE;
			} 
		}
	}
}

//Destroy the process environment
void destroyProcess() {

	for (int i = 0; i < MAX_PROCESS; i++) {
		if (Processes[i].hProcess != NULL) {
			TerminateProcess(Processes[i].hProcess, 0);
			CloseHandle(Processes[i].hProcess);
		}
	}
}