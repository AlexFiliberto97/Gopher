#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pipe.h"
#include "event.h"
#include "thread.h"
#include "process.h"
#include "../gopher.h"
#include "../utils.h"

void init_env() {

	initPipes();
	initEvents();
	initThread();
	initProcess();
}

int start_env() {
	
	//Creating new pipe for logger process
	int success = createPipe("LOGGER_PIPE");
	if (success == -1) return -1;

	//Creating events for LOGGER_PIPE
	success = createEvent("WRITE_LOG_EVENT", TRUE);
	if (success == -1) return -1;
	
	success = createEvent("READ_LOG_EVENT", FALSE);
	if (success == -1) return -1;

   	//Creating new logger process
   	int argc = 3;
   	char** cmd = (char**) malloc(sizeof(char*) * argc);
   	if (cmd == NULL) return -1;

   	for (int i = 0; i < argc; i++) {
   		cmd[i] = (char*) malloc(11);
   		if (cmd[i] == NULL) return -1;
   	}

   	sprintf(cmd[0], "%d", (int)getReader("LOGGER_PIPE"));
   	sprintf(cmd[1], "%d", (int)eventHandler("WRITE_LOG_EVENT"));
   	sprintf(cmd[2], "%d", (int)eventHandler("READ_LOG_EVENT"));
	
	success = startProcess("win32/logger.exe", 3, cmd);
	if(success == -1) return -1;
	freeList(cmd, argc);

	//Setting default gopher values
	setDefaultGopherOptions();

	//Starting garbage collector for threads e processes
	startThread(threadCollector, NULL);
	startThread(processCollector, NULL);
	return 0;
}




int clean_env(){


	//destroyPipes();
	//destroyEvents();
	

	return 0;
}