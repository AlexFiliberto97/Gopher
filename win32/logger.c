#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.h"
#include "pipe.h"
#include "utils_win32.h"

void init_env();

void main(int argc, char** argv) {

	init_env();
	
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
}

void init_env() {

	initPipes();	
	initEvents();
}