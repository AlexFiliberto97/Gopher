#include <stdlib.h>
#include <stdio.h>
#include "../include/server.h"
#include "../include/utils.h"
#include "../include/network.h"
#include "../include/gopher.h"
#include "../include/error.h"
#include "../include/const.h"
#include "../include/socket.h"

#ifndef __linux__
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include "../../win32/include/pipe.h"
	#include "../../win32/include/event.h"
	#include "../../win32/include/thread.h"
	#include "../../win32/include/process.h"
	#include "../../win32/include/utils_win32.h"
#else
	#include <unistd.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <sys/mman.h>
	#include "../../posix/include/thread.h"
	#include "../../posix/include/process.h"
	#include "../../posix/include/listener.h"
	#include "../../posix/include/utils_posix.h"
#endif

/* Global variables */
int SERV_RUNNING = 1;

/* Function: getOpts
*  Get the command line options.
*/
int getOpts(int argc, char** args) {

	int port = -1, process_mode = -1;
	
	for (int i = 0; i < argc; i++) {
		if (strcmp(args[i], "-p") == 0 && i < argc -1 && isNumeric(args[i+1]) == 0) {
			port = atoi(args[i+1]);
			if (checkPort(port) != 0) return BAD_PORT;
		} else if (strcmp(args[i], "-process") == 0) {
			process_mode = 1;
		}
	}
	if (port != -1) serverOptions[curSocket]->port = port; 
	if (process_mode != -1) serverOptions[curSocket]->process_mode = process_mode; 
	return 0;
}

/* Function: getOpts
*  Get the config file options.
*/
Opts* getConfig(int reloadCounter) {

	int err = 0;

	if ((err = checkConfigFile()) != 0) {
		throwError(err);
		return NULL;
	}

	Opts* opts = (Opts*) malloc(sizeof(Opts));
	if (opts == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	initOpts(opts);
	int config_count = 0;
	char** config_assoc_list = readlines(CONFIG_FILE_PATH, &config_count);
	if (config_assoc_list == NULL) {
		freeOpts(opts);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	Dict config_dict = buildDict(config_assoc_list, config_count);
	if (config_dict.err != 0) {
		freeOpts(opts);
		freeList(config_assoc_list, config_count);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	freeList(config_assoc_list, config_count);

	if (reloadCounter == 0) {
		char* address_v = getAssocValue("address", config_dict);
		if (address_v == NULL || ipFormatCheck(address_v) != 0) {
			freeOpts(opts);
			freeDict(config_dict);
			int err = (ipFormatCheck(address_v) != 0) ? BAD_ADDRESS : ALLOC_ERROR;
			throwError(err);
			return NULL;
		}

		address = cpyalloc(address_v);
		if (address == NULL) {
			freeOpts(opts);
			freeDict(config_dict);
			throwError(ALLOC_ERROR);
			return NULL;
		}
	}
	
	char* port_v = getAssocValue("port", config_dict);
	if (port_v == NULL) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(ALLOC_ERROR);
		return NULL;
	} 

	int port = atoi(port_v);
	if (checkPort(port) != 0) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(BAD_PORT);
		return NULL;
	}

	opts->port = port;
	char* process_mode_v = getAssocValue("process", config_dict);
	if (process_mode_v == NULL) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(ALLOC_ERROR);
		return NULL;
	} 
	int process_mode = atoi(process_mode_v);
	if (process_mode != 0 && process_mode != 1) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(BAD_PMODE);
		return NULL;
	}

	opts->process_mode = process_mode;
	char* root_path_v = getAssocValue("root", config_dict);
	if (root_path_v == NULL) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(ALLOC_ERROR);
		return NULL;
	} 
	
	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) != 0)) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(BAD_ROOT);
		return NULL;
	}

	opts->abs_root_path = cpyalloc(root_path_v);
	if (opts->abs_root_path == NULL) {
		freeOpts(opts);
		freeDict(config_dict);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	freeDict(config_dict);
	return opts;
}

/* Function: serverInit
*  Start WSA environment.
*/
int serverInit() {
	
	#ifndef __linux__
		
		WSADATA wsaData;
		int err	= WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
			return WSA_ERROR;
		}
	
	#endif
	return 0;
}

/* Function: serverStart
*  Start the server.
*/
int serverStart(int argc, char** args, int reloadCounter) {
	
	SERV_RUNNING = 1;

	Opts* opts = getConfig(reloadCounter);
	if (opts == NULL) return -1;

	int reuse = 0, err = 0;
	curSocket = nextSocket(opts->port, &reuse);
	if (reuse == 1) {
		serverOptions[curSocket]->process_mode = opts->process_mode;
		serverOptions[curSocket]->abs_root_path = opts->abs_root_path;
		free(opts);
		checkSockets();
		return 0;
	}

	serverOptions[curSocket] = opts;
	
	if (reloadCounter == 0) {
		int optsErr = getOpts(argc, args);
		if (optsErr != 0) return optsErr;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		closeSocket(sock);
		freeOpts(opts);
		return SOCK_ERROR;
	}

	#ifndef __linux__
		BOOL optval = TRUE;
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, sizeof(int));
	#else
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
		err |= setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &(int){ 1 }, sizeof(int));
	#endif

	if (err < 0) {
		closeSocket(sock);
		freeOpts(opts);
		return SOCK_ERROR; 
	}

	struct sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(address);
	service.sin_port = htons(serverOptions[curSocket]->port);
	err = bind(sock, (struct sockaddr*) &service, sizeof(service));
	if (err == -1) {
		closeSocket(sock);
		freeOpts(opts);
		return BIND_ERROR;
	}

	serverOptions[curSocket]->sock = sock;
	checkSockets();
	return 0;
}

/* Function: serverService
*  Listen for clients connections.
*/
int serverService() {

	int err = listen(serverOptions[curSocket]->sock, 100);
	if (err == -1) {
		closeSocket(serverOptions[curSocket]->sock);
		return LISTEN_ERROR;
	} 

	printlog("Server listening on port: %d\n", serverOptions[curSocket]->port, NULL);

	while (SERV_RUNNING == 1) {

		fd_set readSet;
		FD_ZERO(&readSet);
		struct timeval timeout = {1, 0};

		for (int i = 0; i < N_SOCKETS; i++) {
			if (serverOptions[i]->sock == -1) continue;
			FD_SET(serverOptions[i]->sock, &readSet);
		}
	
		err = select(serverOptions[curSocket]->sock + 1, &readSet, NULL, NULL, &timeout);
		if (err == -1) {
			#ifdef __linux__
				if (errno == 4) continue;
			#endif
			throwError(SELECT_ERROR);
			continue;
		}

		if (FD_ISSET(serverOptions[curSocket]->sock, &readSet) == 0) continue;
		
		struct sockaddr_in cli_addr  = {0};
		int cli_addr_len = sizeof(cli_addr);		
		int acceptSocket = accept(serverOptions[curSocket]->sock, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_addr_len);
		if (acceptSocket == -1) {
			throwError(ACCEPT_ERROR);
			continue;
		}
		
		char* cliData = getClientAddress(cli_addr);
		if (cliData == NULL) {
			closeSocket(acceptSocket);
			continue; 
		}

		if (serverOptions[curSocket]->process_mode == 0) {

			HandlerData* hd = (HandlerData*) malloc(sizeof(HandlerData));
			if (hd == NULL) {
				throwError(ALLOC_ERROR);
				free(cliData);
				closeSocket(acceptSocket);
				continue; 
			}

			hd->cliData = cpyalloc(cliData);
			hd->address = cpyalloc(address);
			hd->abs_root_path = cpyalloc(serverOptions[curSocket]->abs_root_path);
			if (hd->cliData == NULL || hd->address == NULL || hd->abs_root_path == NULL ) {
				throwError(ALLOC_ERROR);
				free(cliData);
				freeHandlerDataStruct(hd);
				closeSocket(acceptSocket);
				continue; 
			}

			hd->sock = acceptSocket;
			hd->port = serverOptions[curSocket]->port;
			hd->process_mode = 0;
			int th = startThread((void*) handler, (void*) hd, curSocket);
			if (th < 0) {
				throwError(th);
				free(cliData);
				freeHandlerDataStruct(hd);
				closeSocket(acceptSocket);	 
				continue;
			}
			incrementSocket(curSocket);

		} else {

			#ifndef __linux__

				char** cmd = (char**) malloc(sizeof(char*) * LISTNR_ARGC);
				if (cmd == NULL) {
					throwError(ALLOC_ERROR);
					free(cliData);
					closeSocket(acceptSocket);
					continue; 
				}		

				cmd[0] = (char*) malloc(INTSTR);
				cmd[1] = (char*) malloc(strlen(cliData) + 1);
				cmd[2] = (char*) malloc(strlen(address) + 1);
				cmd[3] = (char*) malloc(INTSTR);
				cmd[4] = (char*) malloc(strlen(serverOptions[curSocket]->abs_root_path) + 1);
				if (checkMalloc(cmd, LISTNR_ARGC) != 0) {
					freeList(cmd, LISTNR_ARGC);
					free(cliData);
					throwError(ALLOC_ERROR);
					closeSocket(acceptSocket);
					continue; 
				}

				sprintf(cmd[0], "%d", getWriter("LOGGER_PIPE"));
				sprintf(cmd[1], "%s", cliData);
				sprintf(cmd[2], "%s", address);
				sprintf(cmd[3], "%d", serverOptions[curSocket]->port);
				sprintf(cmd[4], "%s", serverOptions[curSocket]->abs_root_path);

				int procId = startProcess("bin/listener.exe", LISTNR_ARGC, cmd, curSocket);
				if (procId < 0) {
					freeList(cmd, LISTNR_ARGC);
					free(cliData);
					closeSocket(acceptSocket);
					continue;
				}

				freeList(cmd, LISTNR_ARGC);
				incrementSocket(curSocket);

				WSAPROTOCOL_INFOA info;
				err = WSADuplicateSocket((SOCKET) acceptSocket, procId, &info);
				if (err != 0) {
					free(cliData);
					throwError(DUPLICATE_SOCK_ERR);
					continue;
				}

				HANDLE hDup = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(WSAPROTOCOL_INFOA), cliData);
				if (hDup == NULL) {
					free(cliData);
					throwError(CREATE_MAPPING);
					continue;
				}

				void* dupInfo = MapViewOfFile(hDup, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(WSAPROTOCOL_INFOA));
				if (dupInfo == NULL) {
					free(cliData);
					throwError(MAP_VOF_ERROR);
					continue;
				}

				memcpy(dupInfo, (void*) &info, sizeof(info));
				int err = setEvent(hListenerEventW);
				if (err < 0) {
					free(cliData);
					UnmapViewOfFile(dupInfo);
					CloseHandle(hDup);
					closeSocket(acceptSocket);
					throwError(err);
					continue;
				}

				waitEvent(hListenerEventR, TIMEOUT);
				UnmapViewOfFile(dupInfo);
				CloseHandle(hDup);
				closeSocket(acceptSocket);
			
			#else
				
				HandlerData* hd = (HandlerData*) create_shared_memory(sizeof(HandlerData));
				if (hd == MAP_FAILED) {
					free(cliData);
					throwError(CREATE_MAPPING);
					closeSocket(acceptSocket);
					continue; 
				}

				hd->cliData = (char*) create_shared_memory(strlen(cliData) + 1);
				hd->address = (char*) create_shared_memory(strlen(address) + 1);
				hd->abs_root_path = (char*) create_shared_memory(strlen(serverOptions[curSocket]->abs_root_path) + 1);
				if (hd->cliData == MAP_FAILED || hd->address == MAP_FAILED || hd->abs_root_path == MAP_FAILED) {
					free_shared_memory(hd->cliData, sizeof(hd->cliData));
					free_shared_memory(hd->address, sizeof(hd->address));
					free_shared_memory(hd->abs_root_path, sizeof(hd->abs_root_path));
					free_shared_memory(hd, sizeof(hd));
					free(cliData);
					closeSocket(acceptSocket);
					throwError(CREATE_MAPPING);
					continue; 
				}

				strcpy(hd->cliData, cliData);
				strcpy(hd->address, address);
				strcpy(hd->abs_root_path, serverOptions[curSocket]->abs_root_path);
				hd->sock = acceptSocket;
				hd->port = serverOptions[curSocket]->port;
				hd->process_mode = 1;
				err = startProcess((void*) listener, (void*) hd, curSocket);
				if (err < 0) {
					free_shared_memory(hd->cliData, sizeof(hd->cliData));
					free_shared_memory(hd->address, sizeof(hd->address));
					free_shared_memory(hd->abs_root_path, sizeof(hd->abs_root_path));
					free_shared_memory(hd, sizeof(hd));
					free(cliData);
					closeSocket(acceptSocket);
					throwError(err);
					continue;
				}

				incrementSocket(curSocket);
				close(acceptSocket);
			
			#endif
		}
		free(cliData);
	}
	return 0;
}

/* Function: serverStop
*  Stop the server.
*/
void serverStop() {
	
	SERV_RUNNING = 0;
}