#include <windows.h> 
#include <winsock2.h>
#include <stdio.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"

//Init the winsock
int serverInit() {

	WSADATA wsaData;
	int err	= WSAStartup(514, &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0){
		WSACleanup();
		return -1;
	}
	return 0;
}

//Console handler
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    
    return TRUE;
}

int main(int argc, char** argv) {

	//Setting the console event
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;

	int err = serverInit();
	if (err == -1) return -1;

	addPipe("LOGGER_PIPE", NULL, (HANDLE) atoi(argv[0]));
	addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[1]));
	addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[2]));
	struct ClientData cd;
	cd.sock = atoi(argv[5]);
	cd.data = argv[6];

	char* address = (char*) malloc(strlen(argv[3]) + 1);
	int* port = (int*) malloc(sizeof(int));
	char* root_path = (char*) malloc(strlen(argv[7]) + 1);

	strcpy(address, argv[5]);
	*port = atoi(argv[4]);
	strcpy(root_path, argv[7]);

	setGopherOptions(address, port, root_path);
	handler((void*) &cd, 1);
	return 0;
}