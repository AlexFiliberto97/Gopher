#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "pipe.h"
#include "utils_win32.h"
#include "../error.h"

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    return TRUE;
}

int setKeyboardEvent() {
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;
	return 0;
}

int main(int argc, char** argv) {

	initEvents();

	int err;
	err = setKeyboardEvent();
	if (err != 0) {
		throwError(3, SERVER_ERROR_H, err, LOGGER_ERROR);
		return -1;
	}
	
	addPipe((HANDLE) atoi(argv[0]), NULL);
	err = addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[1]));
	if (err != 0) {
		throwError(3, SERVER_ERROR_H, err, LOGGER_ERROR);
		return -1;
	}
	
	err = addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[2]));
	if (err != 0) {
		throwError(3, SERVER_ERROR_H, err, LOGGER_ERROR);
		return -1;
	}
	
	while (1) {

		waitEvent("READ_LOG_EVENT"); 
		char* text = readPipe("LOGGER_PIPE");

		if (strcmp(text, "TERMINATE_LOGGER") == 0) {
				free(text);
				break;
			}

		if (text == NULL) {
			appendToFile("log.txt", "Errore nella lettura della pipe\n");			
		}

		err = appendToFile("log.txt", text);
		if (err != 0) {
			throwError(2, err, LOGGER_ERROR);
		}

		setEvent("WRITE_LOG_EVENT");
		free(text);
	}
	return 0;
}