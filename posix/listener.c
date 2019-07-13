#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#ifndef __linux__
	#include <windows.h> 
	#include <winsock2.h>
#else
	#include <unistd.h>
#endif

#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


/* Function: serverInit / WIN32
* -------------------------------
*  
*/
// int serverInit() { //WIN32 only
// 	WSADATA wsaData;
// 	int err	= WSAStartup(514, &wsaData);
// 	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0){
// 		WSACleanup(); //Call error here
// 		return 1;
// 	}
// 	return 0;
// }


/* Function: processHandler
* -------------------------------
*  
*/
int processHandler(void* input) {
	return handler(input, 1);
}


void* listener(int argc, char** argv){
	// serverInit();
	// addPipe("LOGGER_PIPE", NULL, (HANDLE) atoi(argv[2]));
	// addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[3]));
	// addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[4]));
	struct ClientData cd;
	cd.sock = atoi(argv[1]);
	cd.data = argv[1];
	setRootPath(argv[2]);
	processHandler((void*) &cd);

}