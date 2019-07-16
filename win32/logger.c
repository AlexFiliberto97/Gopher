#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "pipe.h"
#include "utils_win32.h"

//Init the environment for logger
void init_env() {
	
	initPipes();	
	initEvents();
}


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    
    return TRUE;
}


int main(int argc, char** argv) {

	init_env();

	//Setting the console event
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;
	
	//Adding global objects
	addPipe("LOGGER_PIPE", (HANDLE)atoi(argv[0]), NULL);
	addEvent("WRITE_LOG_EVENT", (HANDLE)atoi(argv[1]));
	addEvent("READ_LOG_EVENT", (HANDLE)atoi(argv[2]));
	
	while(TRUE){

		waitEvent("READ_LOG_EVENT"); 

		char* text = readPipe("LOGGER_PIPE");

		if (text == NULL) {
			appendToFile("log.txt", "Errore nella lettura della pipe\n");			
		}

		appendToFile("log.txt", text);
		setEvent("WRITE_LOG_EVENT");
		free(text);
	}

	return 0;

}