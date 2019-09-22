#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/event.h"
#include "../include/pipe.h"
#include "../include/utils_win32.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

/* Function: CtrlHandler
*  Ignore (return True).
*/
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    
    return TRUE;
}

/* Function: setKeyboardEvent
*  Set a console event.
*/
int setKeyboardEvent() {
	
	return SetConsoleCtrlHandler(CtrlHandler, TRUE) ? 0 : -1;
}

int main(int argc, char** argv) {

	initEvents();

	int err;
	err = setKeyboardEvent();
	if (err != 0) {
		throwError(err);
		return -1;
	}
	
	setPipe((HANDLE) atoi(argv[0]), NULL);

	HANDLE hEventW = openEvent("LOGGER_EVENT_W");
	HANDLE hEventR = openEvent("LOGGER_EVENT_R");
	if (hEventW == NULL || hEventR == NULL) return -1;
	
	while (1) {

		waitEvent(hEventR, -1); 

		char* text = readPipe("LOGGER_PIPE");
		if (text == NULL) {
			err = appendToFile(LOG_FILE_PATH, "Errore nella lettura della pipe\n");
			if (err != 0) throwError(err);
			if (setEvent(hEventW) != 0) throwError(err);
			continue;
		}

		if (strcmp(text, "TERMINATE_LOGGER") == 0) {
			if (setEvent(hEventW) != 0) throwError(err);
			free(text);
			break;
		}

		if (appendToFile(LOG_FILE_PATH, text) != 0) throwError(err);
		if (setEvent(hEventW) != 0) throwError(err);
		
		free(text);
	}

	CloseHandle(hEventW);
	CloseHandle(hEventR);

	return 0;
}