#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "pipe.h"
#include "utils_win32.h"


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    return TRUE;
}


int setKeyboardEvent() {
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;
	return 0;
}


void init_env() {
	initPipes();	
	initEvents();
}


int main(int argc, char** argv) {

	int err;

	err = setKeyboardEvent();

	if (err != 0) {
		printf("\nERROR: could not set control handler\n");
		return -1;
	}

	init_env();
	
	//Adding global objects
	addPipe("LOGGER_PIPE", (HANDLE) atoi(argv[0]), NULL);
	addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[1]));
	addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[2]));
	
	while (1) {

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