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

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    return TRUE;
}

int setKeyboardEvent() {
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;
	return 0;
}

int main(int argc, char** argv) {

	serverInit();
	int error;

	error = setKeyboardEvent();
	if (error != 0) {
		throwError(3, SERVER_ERROR_H, error, LISTENER_ERROR);
		return -1;
	}

	addPipe(NULL, (HANDLE) atoi(argv[1]));
	error = addEvent("WRITE_LOG_EVENT", (HANDLE) atoi(argv[2]));
	if (error < 0) {
		throwError(3, SERVER_ERROR_H, error, LISTENER_ERROR);
		return -1;
	}
	
	error = addEvent("READ_LOG_EVENT", (HANDLE) atoi(argv[3]));
	if (error < 0) {
		throwError(3, SERVER_ERROR_H, error, LISTENER_ERROR);
		return -1;
	}

	struct HandlerData* hd = (struct HandlerData*) malloc(sizeof(struct HandlerData));
	if (hd == NULL) return ALLOC_ERROR;

	hd->sock = atoi(argv[0]);
	hd->port = atoi(argv[6]);
	hd->cli_data = (char*) malloc(strlen(argv[4]) + 1);
	if (hd->cli_data == NULL) return ALLOC_ERROR;
	
	hd->address = (char*) malloc(strlen(argv[5]) + 1);
	if (hd->address == NULL) return ALLOC_ERROR;
	
	hd->root_path = (char*) malloc(strlen(argv[7]) + 1);
	if (hd->root_path == NULL) return ALLOC_ERROR;
	
	hd->abs_root_path = (char*) malloc(strlen(argv[8]) + 1);
	if (hd->abs_root_path == NULL) return ALLOC_ERROR;

	strcpy(hd->cli_data, argv[4]);
	strcpy(hd->address, argv[5]);
	strcpy(hd->root_path, argv[7]);
	strcpy(hd->abs_root_path, argv[8]);

	error = handler((void*) hd, 1);
	if (error < 0) {
		throwError(3, SERVER_ERROR_H, error, LISTENER_ERROR);
		return -1;
	}

	free(hd->cli_data);
	free(hd->address);
	free(hd->root_path);
	free(hd->abs_root_path);
	free(hd);

	return 0;
}