#include <stdio.h>
#include <windows.h> 
#include "../include/pipe.h"
#include "../include/thread.h"
#include "../include/event.h"
#include "../../common/include/network.h"
#include "../../common/include/error.h"
#include "../../common/include/gopher.h"
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
	
	BOOL succ = SetConsoleCtrlHandler(CtrlHandler, TRUE);
	if (!succ) return -1;
	return 0;
}

int main(int argc, char** argv) {

	int err = initThread();
	if (err < 0) {
		throwError(err);
		return -1;
	}

	WSADATA wsaData;
	err	= WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
		throwError(WSA_ERROR);
		WSACleanup();
		return -1;
	}
	
	err = setKeyboardEvent();
	if (err != 0) {
		throwError(err);
		return -1;
	}

	setPipe(NULL, (HANDLE) atoi(argv[0]));

	HandlerData* hd = (HandlerData*) malloc(sizeof(HandlerData));
	if (hd == NULL) {
		throwError(ALLOC_ERROR);
		return -1;
	}
	
	hd->port = atoi(argv[3]);
	hd->cliData = (char*) malloc(strlen(argv[1]) + 1);
	hd->address = (char*) malloc(strlen(argv[2]) + 1);
	hd->abs_root_path = (char*) malloc(strlen(argv[4]) + 1);
	hd->process_mode = 1;
	
	if (hd->cliData == NULL || hd->address == NULL || hd->abs_root_path == NULL) {
		if (hd->cliData != NULL) free(hd->cliData);
		if (hd->address != NULL) free(hd->address);
		if (hd->abs_root_path != NULL) free(hd->abs_root_path);
		throwError(ALLOC_ERROR);
		return -1;
	}

	strcpy(hd->cliData, argv[1]);
	strcpy(hd->address, argv[2]);
	strcpy(hd->abs_root_path, argv[4]);

	HANDLE hEventW = openEvent("LISTENER_EVENT_W");
	HANDLE hEventR = openEvent("LISTENER_EVENT_R");
	if (hEventW == NULL || hEventR == NULL) return -1;

	waitEvent(hEventW, TIMEOUT);

	HANDLE hDup = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, hd->cliData);
	if (hDup == INVALID_HANDLE_VALUE) return -1;

	WSAPROTOCOL_INFOA* dupInfo = (WSAPROTOCOL_INFOA*) MapViewOfFile(hDup, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(WSAPROTOCOL_INFOA));
	if (dupInfo == NULL) return -1;
	
	hd->sock = (int) WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, dupInfo, 0, 0);
	if (hd->sock == INVALID_SOCKET) return -1;

	err = setEvent(hEventR);
	if( err < 0) throwError(err);
	
	UnmapViewOfFile(dupInfo);
	CloseHandle(hDup);

	err = handler((void*) hd);
	if (err < 0) {
		throwError(err);
		return -1;
	}

	return 0;
}