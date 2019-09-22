#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/environment.h"
#include "../include/pipe.h"
#include "../include/event.h"
#include "../include/thread.h"
#include "../include/process.h"
#include "../include/caching.h"
#include "../include/utils_win32.h"
#include "../../common/include/gopher.h"
#include "../../common/include/utils.h"
#include "../../common/include/server.h"
#include "../../common/include/socket.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

/* Global variables */
int SERVER_ALIVE = 0;

/* Function: killLogger
*  Kill the logger process.
*/
void killLogger() {

	HANDLE hLogger = getLoggerHandle();
	waitEvent("WRITE_LOG_EVENT", -1);
	int err = writePipe("TERMINATE_LOGGER");
	if (err != 0) {
		throwError(err);
		if(!TerminateProcess(hLogger, 0)) {
			throwError(TERMINATE_LOGGER_ERR);
		}
	}
	setEvent("READ_LOG_EVENT");
	CloseHandle(hLogger);
}

/* Function: CtrlHandler
*  Set the console event handler
*/
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {

	switch (fdwCtrlType) {
		case CTRL_C_EVENT:
		{
		    printf(">>> Choose an action:\n    reload: r\n    quit: q\n<<< ");
		    char choice[2];
		    fgets(choice, 2, stdin);

		    char c;
			while ((c = getchar()) != '\n' && c != EOF);

		    if (strcmp(choice, "r") == 0) {
		    	printf("Reloading server...\n");
		    	serverStop();
		   		return TRUE;
		    } else if (strcmp(choice, "q") == 0) {
		   		printf("Stopping server...\n");
		        SERVER_ALIVE = 0;
		        serverStop();
		        return TRUE;
		    } else {
		    	return TRUE;
		    }
		}
	}	
	return FALSE;
}

/* Function: consoleDecoration
*  Console decoration for windows.
*/
void consoleDecoration() {

	int columns;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int span = (columns -40)/2;
  
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

/* Function: init_env
*  Initialize the environment for win32.
*/
int init_env() {

	SERVER_ALIVE = 1;
	int err = initCache();
	if (err != 0) return err;

	err = initServerOpts();
	if (err != 0) return err;

	err = createLoggerPipe();
	if (err != 0) return err;

	err = initProcess();
	if (err != 0) return err;

	err = initThread();
	if (err != 0) return err;

	err = initEvents();
	if (err != 0) return err;

	return 0;
}

/* Function: start_env
*  Initialize the environment for win32.
*/
int start_env() {

	consoleDecoration();
	
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return CONSOLE_HANDLER;

	int err = initEvents();
	if (err != 0) return err;

   	char** cmd = (char**) malloc(sizeof(char*) * LOGGER_ARGC);
   	if (cmd == NULL) return ALLOC_ERROR;

   	cmd[0] = (char*) malloc(INTSTR);
   	if (cmd[0] == NULL) {
   		free(cmd);
   		return ALLOC_ERROR;
   	}
 
   	sprintf(cmd[0], "%d", (int) getReader("LOGGER_PIPE"));

	err = startProcess("bin/logger.exe", LOGGER_ARGC, cmd, -1);
	if (err < 0) {
		freeList(cmd, LOGGER_ARGC); 
		return err;
	}

	freeList(cmd, LOGGER_ARGC);
	err = startThread(threadCollector, NULL, -1);
	if (err < 0) return err;

	err = startThread(processCollector, NULL, -1);
	if (err < 0) return err;
	
	return 0;
}

/* Function: clean_env
*  Clean the environment.
*/
void clean_env() {

	stopThreadCollector();
	stopProcessCollector();
	serverStop();
	destroyServerOpts();
	destroyThreads();
	destroyCache();
	destroyProcess();
	killLogger();
	destroyEvents();
	destroyLoggerPipe();
	WSACleanup();
}