#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "network.h"
#include "gopher.h"
#include "utils.h"
#include "error.h"

#define CONFIG_FILE_PATH "config/config.txt"

#ifndef __linux__
	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include "win32/thread.h"
	#include "win32/process.h"
#else
	#include "posix/thread.h"
	#include "posix/listener.c"
	#include <unistd.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
#endif

int getOpts(int, char**);
int getConfig();
int serverInit(int, char**);
int serverStart();
int serverService();
int serverReload();
void closeSocket(int);
void serverStop();

struct Opts {
	
	int sock;
	int port;
	int process_mode;
	char* address;
	char* root_path;
	char* abs_root_path;
};

static struct Opts serverOptions;
static int SERV_RUNNING = 1;

void freeServerOptions() {

	serverOptions.sock = -1;
	serverOptions.port = 7070;
	serverOptions.process_mode = 0;
	if (serverOptions.address != NULL) free(serverOptions.address);
	if (serverOptions.root_path != NULL) free(serverOptions.root_path);
	if (serverOptions.abs_root_path != NULL) free(serverOptions.abs_root_path);
	serverOptions.address = NULL;
	serverOptions.root_path = NULL;
	serverOptions.abs_root_path = NULL;
}

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
	if (port != -1) serverOptions.port = port; 
	if (process_mode != -1) serverOptions.process_mode = process_mode; 
	return 0;

}

int getConfig() {

	int config_count;
	char** config_assoc_list;
	config_assoc_list = readlines(CONFIG_FILE_PATH, &config_count);
	if (config_assoc_list == NULL) return ALLOC_ERROR;

	struct Dict config_dict = buildDict(config_assoc_list, config_count);
	if (config_dict.err != 0) return ALLOC_ERROR;
	freeList(config_assoc_list, config_count);

	char* address_v = getAssocValue("address", config_dict);
	if (address_v == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	} else if (ipFormatCheck(address_v) != 0) {
		freeDict(config_dict);
		return BAD_ADDRESS;
	}

	serverOptions.address = cpyalloc(address_v);
	if (serverOptions.address == NULL) { 
		freeDict(config_dict);
		return ALLOC_ERROR;
	}

	char* port_v = getAssocValue("port", config_dict);
	if (port_v == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	} 

	int port = atoi(port_v);
	if (checkPort(port) != 0) {
		freeDict(config_dict);
		return BAD_PORT;
	}
	
	serverOptions.port = port;
	char* process_mode_v = getAssocValue("process", config_dict);
	if (process_mode_v == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	} 

	int process_mode = atoi(process_mode_v);
	if (process_mode != 0 && process_mode != 1) {
		freeDict(config_dict);
		return BAD_PMODE;
	}

	serverOptions.process_mode = process_mode;
	char* root_path_v = getAssocValue("root", config_dict);
	if (root_path_v == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	} 

	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) != 0)) {
		freeDict(config_dict);
		return BAD_ROOT;
	}

	serverOptions.abs_root_path = cpyalloc(root_path_v);
	if (serverOptions.abs_root_path == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	}

	if (strcmp(root_path_v, "/") == 0 || strcmp(&root_path_v[1], ":/") == 0) {
		serverOptions.root_path = (char*) malloc(strlen(root_path_v) + 1);
		strcpy(serverOptions.root_path, root_path_v);
	} else {

		int last_occ = lastOccurrence(root_path_v, '/', -1);
		if (last_occ == -1) {
			freeDict(config_dict);
			return -1;
		}

		char* rel_root_path = slice(root_path_v, last_occ + 1, strlen(root_path_v));
		if (rel_root_path == NULL) {
			freeDict(config_dict);
			return -1;
		}

		serverOptions.root_path = cpyalloc(rel_root_path);
		if (serverOptions.root_path == NULL) {
			free(rel_root_path);
			freeDict(config_dict);
			return ALLOC_ERROR;
		}
		free(rel_root_path);
	}
	freeDict(config_dict);
	return 0;
}

int serverInit(int argc, char** args) {

	freeServerOptions();
	int confErr = getConfig();
	if (confErr != 0) {
		return confErr;
	}
	
	int optsErr = getOpts(argc, args);
	if (confErr != 0 && optsErr != 0) {
		return confErr;
	} else {
		if (optsErr != 0) throwError(1, optsErr);
		if (confErr != 0) throwError(1, confErr);
	}
	
	#ifndef __linux__
		WSADATA wsaData;
		int err	= WSAStartup(514, &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
			throwError(1, SERVER_ERROR_S);
			return WSA_ERROR;
		}
	#endif
	return 0;
}

int serverStart() {

	int err;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		closeSocket(sock);
		throwError(1, SERVER_ERROR_S); 
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
		throwError(1, SERVER_ERROR_S); 
		return SOCK_ERROR; 
	}

	struct sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(serverOptions.address);
	service.sin_port = htons(serverOptions.port);
	err = bind(sock, (struct sockaddr*) &service, sizeof(service));
	if (err == -1) {
		closeSocket(sock);
		throwError(1, SERVER_ERROR_S); 
		return BIND_ERROR;
	}
	serverOptions.sock = sock;
	return 0;
}

int serverService() {

	int err = listen(serverOptions.sock, 10);
	if (err == -1) {
		closeSocket(serverOptions.sock);
		throwError(1, SERVER_ERROR_S); 
		return LISTEN_ERROR;
	} 

	printlog("Server listening on port: %d\n", serverOptions.port, NULL);

	while (SERV_RUNNING == 1) {

		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverOptions.sock, &readSet);
		struct timeval timeout = {1, 0};

		err = select(serverOptions.sock + 1, &readSet, NULL, NULL, &timeout);
		if (err == -1) {
			#ifdef __linux__
				if (errno == 4) continue;
			#endif
			closeSocket(serverOptions.sock);
			throwError(2, SERVER_ERROR_H, SELECT_ERROR);
			continue;
		}
		
		if (FD_ISSET(serverOptions.sock, &readSet) == 0) continue;
		
		struct sockaddr_in cli_addr  = {0};
		int cli_addr_len = sizeof(cli_addr);		
		int acceptSocket = accept(serverOptions.sock, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_addr_len);
		if (acceptSocket == -1) {
			throwError(2, SERVER_ERROR_H, ACCEPT_ERROR);
			continue;
		}
		
		char* cli_data = getClientAddress(cli_addr);
		if (cli_data == NULL) {
			throwError(2, SERVER_ERROR_H, ALLOC_ERROR);
			closeSocket(acceptSocket);
			continue; 
		}

		if (serverOptions.process_mode == 0) {
			struct HandlerData* hd = (struct HandlerData*) malloc(sizeof(struct HandlerData));
			if (hd == NULL) {
				throwError(2, SERVER_ERROR_H, ALLOC_ERROR);
				closeSocket(acceptSocket);
				continue; 
			}

			hd->cli_data = cpyalloc(cli_data);
			hd->address = cpyalloc(serverOptions.address);
			hd->root_path = cpyalloc(serverOptions.root_path);
			hd->abs_root_path = cpyalloc(serverOptions.abs_root_path);
			if (hd->cli_data == NULL || hd->address == NULL || hd->root_path == NULL || hd->abs_root_path == NULL ) {
				throwError(2, SERVER_ERROR_H, ALLOC_ERROR);
				closeSocket(acceptSocket);
				continue; 
			}
			
			hd->sock = acceptSocket;
			hd->port = serverOptions.port;
			hd->process_mode = 0;
			int th = startThread((void*) handler, (void*) hd, 1);
			if (th < 0) {
				throwError(2, SERVER_ERROR_H, th);
				closeSocket(acceptSocket);	 
				continue;
			}
		} else {

			#ifndef __linux__
				char** cmd = (char**) malloc(sizeof(char*) * 9);
				if (cmd == NULL) {
					throwError(2, SERVER_ERROR_H, ALLOC_ERROR);
					closeSocket(acceptSocket);
					continue; 
				}		
	   			cmd[0] = (char*) malloc(11);
	   			cmd[1] = (char*) malloc(11);
	   			cmd[2] = (char*) malloc(11);
	   			cmd[3] = (char*) malloc(11);
	   			cmd[4] = (char*) malloc(strlen(cli_data) + 1);
	   			cmd[5] = (char*) malloc(strlen(serverOptions.address) + 1);
	   			cmd[6] = (char*) malloc(11);
	   			cmd[7] = (char*) malloc(strlen(serverOptions.root_path) + 1);
	   			cmd[8] = (char*) malloc(strlen(serverOptions.abs_root_path) + 1);
	   			if (checkMalloc(cmd, 9) != 0) {
					throwError(2, SERVER_ERROR_H, ALLOC_ERROR);
					closeSocket(acceptSocket);
					continue; 
				}	
	   			sprintf(cmd[0], "%d", acceptSocket);
	   			sprintf(cmd[1], "%d", getWriter("LOGGER_PIPE"));
	   			sprintf(cmd[2], "%d", eventHandler("WRITE_LOG_EVENT"));
	   			sprintf(cmd[3], "%d", eventHandler("READ_LOG_EVENT"));
	   			sprintf(cmd[4], "%s", cli_data);
	   			sprintf(cmd[5], "%s", serverOptions.address);
	   			sprintf(cmd[6], "%d", serverOptions.port);
	   			sprintf(cmd[7], "%s", serverOptions.root_path);
	   			sprintf(cmd[8], "%s", serverOptions.abs_root_path);
				err = startProcess("win32/listener.exe", 9, cmd);
				freeList(cmd, 9);
				// close socket ?
			#else
				struct HandlerData* hd = (struct HandlerData*) create_shared_memory(sizeof(struct HandlerData));
				if (hd == MAP_FAILED) {
					throwError(2, SERVER_ERROR_H, CREATE_MAPPING);
					closeSocket(acceptSocket);
					continue; 
				}
				hd->cli_data = (char*) create_shared_memory(strlen(cli_data) + 1);
				hd->address = (char*) create_shared_memory(strlen(serverOptions.address) + 1);
				hd->root_path = (char*) create_shared_memory(strlen(serverOptions.root_path) + 1);
				hd->abs_root_path = (char*) create_shared_memory(strlen(serverOptions.abs_root_path) + 1);
				if (hd->cli_data == MAP_FAILED || hd->address == MAP_FAILED || hd->root_path == MAP_FAILED || hd->abs_root_path == MAP_FAILED) {
					throwError(2, SERVER_ERROR_H, CREATE_MAPPING);
					closeSocket(acceptSocket);
					continue; 
				}

				strcpy(hd->cli_data, cli_data);
				strcpy(hd->address, serverOptions.address);
				strcpy(hd->root_path, serverOptions.root_path);
				strcpy(hd->abs_root_path, serverOptions.abs_root_path);
				hd->sock = acceptSocket;
				hd->port = serverOptions.port;
				hd->process_mode = 1;
				err = startProcess((void*) listener, (void*) hd);
				close(acceptSocket);
			#endif

			if (err < 0) {
				throwError(2, SERVER_ERROR_H, err);
				continue;
			}
		}
		free(cli_data);
	}

	SERV_RUNNING = 1;
	closeSocket(serverOptions.sock);
	return 0;
}


void closeSocket(int sock) {
	#ifndef __linux__
		closesocket(sock);
	#else
		close(sock);
	#endif
}


void serverStop() {
	SERV_RUNNING = 0;
}

void serverDestroy() {
	#ifndef __linux__
		WSACleanup();
	#endif
}