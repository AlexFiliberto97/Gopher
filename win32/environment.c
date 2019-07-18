#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "pipe.h"
#include "event.h"
#include "thread.h"
#include "process.h"
#include "utils_win32.h"
#include "../gopher.h"
#include "../utils.h"
#include "../server.h"
#include "../error.h"

//Global variables
int SERVER_ALIVE = 0;

//Indentare il codice
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {

	switch (fdwCtrlType) {
		case CTRL_C_EVENT:
	        
	        printf("\nCtrl-C event detected\n\n");
		    printf("Choose an action:\n    cancel: c\n    reload: r\n    quit: q\n>>> ");

		    char choice[2];
		    scanf("%s", &choice);

		    if (strcmp(choice, "c") == 0) {
		    	
		    } else if (strcmp(choice, "r") == 0) {

		    	printf("Reloading server...\n");
		    	serverStop();
		        int err = serverReload();
		        if (err == -1) return TRUE;
		   
		    } else if (strcmp(choice, "q") == 0) {
		   		
		   		printf("Quitting server...\n");
		        SERVER_ALIVE = 0;

		        serverStop();
		    
		    } 
		 
		    return TRUE;
	}
}


void consoleDecoration(){

	int columns;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int span = (columns -40)/2;
  
    system("cls");
	system("color a");
    for(int i = 0; i < span; i++) printf(" ");
    printf(" _____             _               \n");
	for(int i = 0; i < span; i++) printf(" ");
	printf("|  __ \\           | |              \n");
	for(int i = 0; i < span; i++) printf(" ");
	printf("| |  \\/ ___  _ __ | |__   ___ _ __ \n");
	for(int i = 0; i < span; i++) printf(" ");
	printf("| | __ / _ \\| '_ \\| '_ \\ / _ \\ '__|\n");
	for(int i = 0; i < span; i++) printf(" ");
	printf("| |_\\ \\ (_) | |_) | | | |  __/ |   \n");
	for(int i = 0; i < span; i++) printf(" ");
	printf(" \\____/\\___/| .__/|_| |_|\\___|_|   \n");
	for(int i = 0; i < span; i++) printf(" ");
    printf("            | |                    \n");
	for(int i = 0; i < span; i++) printf(" ");
    printf("            |_|                    \n");
}


void init_env() {

	SERVER_ALIVE = 1;
	initPipes();
	initEvents();
	initThread();
	initProcess();
}


int start_env() {

	consoleDecoration();

	//Setting the console event
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return CONSOLE_HANDLER;
	
	//Creating new pipe for logger process
	int success = createPipe("LOGGER_PIPE");
	if (success != 0) return success;

	//Creating events for LOGGER_PIPE
	success = createEvent("WRITE_LOG_EVENT", TRUE);
	if (success != 0) return success;
	
	success = createEvent("READ_LOG_EVENT", FALSE);
	if (success != 0) return success;

   	//Creating new logger process
   	int argc = 3;
   	char** cmd = (char**) malloc(sizeof(char*) * argc);
   	if (cmd == NULL) return ALLOC_ERROR;

   	for (int i = 0; i < argc; i++) {
   		cmd[i] = (char*) malloc(11);
   		if (cmd[i] == NULL) return ALLOC_ERROR;
   	}

   	sprintf(cmd[0], "%d", (int)getReader("LOGGER_PIPE"));
   	sprintf(cmd[1], "%d", (int)eventHandler("WRITE_LOG_EVENT"));
   	sprintf(cmd[2], "%d", (int)eventHandler("READ_LOG_EVENT"));
	
	success = startProcess("win32/logger.exe", 3, cmd);
	if (success != 0) {
		freeList(cmd, argc);
		return success;
	} 
	freeList(cmd, argc);

	//Setting default gopher values
	//setDefaultGopherOptions();
	//setDefaultServerOptions();

	//Starting garbage collector for threads e processes
	success = startThread(threadCollector, NULL);
	if (success < 0) return success;

	success = startThread(processCollector, NULL);
	if (success < 0) return success;
	
	return 0;
}

void clean_env() {

	//destroyDefaultGopherOptions();
	//destroyDefaultServerOptions();
	destroyProcess();
	destroyThreads();
	destroyPipes();
	destroyEvents();
}