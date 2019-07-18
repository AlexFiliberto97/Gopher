#include <windows.h> 
#include <winsock2.h>
#include <stdio.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"
#include "../error.h"


int serverInit() {
	WSADATA wsaData;
	int err	= WSAStartup(514, &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0){
		WSACleanup();
		return 1;
	}
	return 0;
}


int main(int argc, char** argv) {

	//printf("------------- Sono DIO (pro)cesso -------------\n");

	serverInit();
	addPipe("LOGGER_PIPE", NULL, (HANDLE) atoi(argv[1]));
	addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[2]));
	addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[3]));

	struct HandlerData* hd = (struct HandlerData*) malloc(sizeof(struct HandlerData));

	hd->sock = atoi(argv[0]);
	hd->port = atoi(argv[6]);

	hd->cli_data = (char*) malloc(strlen(argv[4]) + 1);
	hd->address = (char*) malloc(strlen(argv[5]) + 1);
	hd->root_path = (char*) malloc(strlen(argv[7]) + 1);
	hd->abs_root_path = (char*) malloc(strlen(argv[8]) + 1);

	strcpy(hd->cli_data, argv[4]);
	strcpy(hd->address, argv[5]);
	strcpy(hd->root_path, argv[7]);
	strcpy(hd->abs_root_path, argv[8]);

	handler((void*) hd, 1);
	printf("ejfejbkdceedchbjwvjhbw 1\n");

	free(hd->cli_data);
	printf("ejfejbkdceedchbjwvjhbw 2\n");
	free(hd->address);
	printf("ejfejbkdceedchbjwvjhbw 3\n");
	free(hd->root_path);
	printf("ejfejbkdceedchbjwvjhbw 4\n");
	free(hd->abs_root_path);
	printf("ejfejbkdceedchbjwvjhbw 4.5\n");
	free(hd);

	printf("ejfejbkdceedchbjwvjhbw 5\n");

	return 0;

	/*
	serverInit();
	addPipe("LOGGER_PIPE", NULL, (HANDLE) atoi(argv[0]));
	addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[1]));
	addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[2]));
	struct ClientData cd;
	cd.sock = atoi(argv[5]);
	cd.data = argv[6];

	char* address = (char*) malloc(strlen(argv[3]) + 1);
	int* port = (int*) malloc(sizeof(int));
	char* root_path = (char*) malloc(strlen(argv[7]) + 1);

	strcpy(address, argv[3]);
	*port = atoi(argv[4]);
	strcpy(root_path, argv[7]);

	setGopherOptions(address, port, root_path);
	handler((void*) &cd, 1);
	return 0;
	*/
}