#include <windows.h> 
#include <winsock2.h>
#include <stdio.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


int serverInit() { //WIN32 only
	WSADATA wsaData;
	int err	= WSAStartup(514, &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0){
		WSACleanup(); //Call error here
		return 1;
	}
	return 0;
}


int main(int argc, char** argv){

	serverInit();
	addPipe("LOGGER_PIPE", NULL, (HANDLE) atoi(argv[1]));
	addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[2]));
	addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[3]));
	struct ClientData cd;
	cd.sock = atoi(argv[0]);
	cd.data = argv[4];

	char* address = (char*) malloc(strlen(argv[5]) + 1);
	int* port = (int*) malloc(sizeof(int));
	char* root_path = (char*) malloc(strlen(argv[7]) + 1);

	strcpy(address, argv[5]);
	*port = atoi(argv[6]);
	strcpy(root_path, argv[7]);

	setGopherOptions(address, port, root_path);
	handler((void*) &cd, 1);

	return 0;
}