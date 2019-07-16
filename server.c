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
	#define _GNU_SOURCE 
	#include "posix/thread.h"
	#include "posix/listener.c"
	#include <unistd.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

//Prototipes
int getOpts(int, char**);
int getConfig();
int serverInit(int, char**);
int serverStart();
int serverService();
int serverReload();
void serverCleanup(int);
void serverStop();


/*
	NB: la connessione resta aperta se fallisce malloc realloc o continue in generale
	NB: la root_path va controllata prima di tutto (creare una gerarchia tra root)
*/

//Global structs
struct Opts {

	int sock;
	char* address;
	int port;
	int process_mode;
	char* root_path;
};


//Global variables
static struct Opts serverOptions = {-1, "127.0.0.1", 7070, 0, "./"}; //Fai la funzione che assegna i valori di default
static int SERV_RUNNING = 1;


//Get server config from command line
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

//Get server config from config file
int getConfig() {

	int config_count;
	char **config_assoc_list;
	config_assoc_list = readlines(CONFIG_FILE_PATH, &config_count);
	if (config_assoc_list == NULL) return ALLOC_ERROR;
	
	struct Dict config_dict = buildDict(config_assoc_list, config_count);
	if (config_dict.err != 0) return ALLOC_ERROR;

	freeList(config_assoc_list, config_count);
	char* address_v = getAssocValue("address", config_dict);
	char* port_v = getAssocValue("port", config_dict);
	char* process_mode_v = getAssocValue("process", config_dict);
	char* root_path_v = getAssocValue("root", config_dict);

	if (address_v == NULL || port_v == NULL || root_path_v == NULL || process_mode_v == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	} 

	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) != 0)) {
		freeDict(config_dict);
		return BAD_ROOT;
	}

	int port = atoi(port_v);
	int process_mode = atoi(process_mode_v);

	if (checkPort(port) != 0) {
		freeDict(config_dict);
		return BAD_PORT;
	}

	if (process_mode != 0 && process_mode != 1) {
		freeDict(config_dict);
		return BAD_PMODE;
	}

	serverOptions.address = (char*) malloc(strlen(address_v) + 1);
	if (serverOptions.address == NULL) { 
		freeDict(config_dict);
		return ALLOC_ERROR;
	}
	
	strcpy(serverOptions.address, address_v);
	serverOptions.port = port;
	serverOptions.process_mode = process_mode;
	serverOptions.root_path = (char*) malloc(strlen(root_path_v) + 1);
	if (serverOptions.root_path == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	}
	strcpy(serverOptions.root_path, root_path_v);
	freeDict(config_dict);
	return 0;
}

//Init the winsock
int serverInit(int argc, char** args) {

	int confErr = getConfig();
	int optsErr = getOpts(argc, args);
	if(confErr != 0 && optsErr != 0) {
		return confErr;
	}else{
		if (optsErr != 0) throwError(optsErr);
		if (confErr != 0) throwError(confErr);
	}
	
	#ifndef __linux__
		WSADATA wsaData;
		int err	= WSAStartup(514, &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
			serverCleanup(-1);
			return errorCode(2, SERVER_ERROR_S, WSA_ERROR);
		}
		return 0;
	#else
		return 0;
	#endif
}

//Start the server
int serverStart() {

	int err;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) { 
		serverCleanup(sock);
		return errorCode(2, SERVER_ERROR_S, SOCK_ERROR); 
	}

	#ifndef __linux__
		BOOL optval = TRUE;
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, sizeof(int));
		if (err < 0) {
			serverCleanup(sock);
			return errorCode(2, SERVER_ERROR_S, SOCK_ERROR); 
		}
	#else
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
		if (err < 0) {
			serverCleanup(sock);
			return errorCode(2, SERVER_ERROR_S, SOCK_ERROR); 
		}
	#endif

	setGopherOptions(serverOptions.address, &serverOptions.port ,serverOptions.root_path);
	struct sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(serverOptions.address);
	service.sin_port = htons(serverOptions.port);
	err = bind(sock, (struct sockaddr*) &service, sizeof(service));
	if (err == -1) {
		serverCleanup(sock);
		return errorCode(2, SERVER_ERROR_S, BIND_ERROR); 
	}
	serverOptions.sock = sock;
	return 0;
}

//Handle the incoming requests
int serverService() {

	int err = listen(serverOptions.sock, 10);
	if (err == -1) {
		serverCleanup(serverOptions.sock);
		return errorCode(2, SERVER_ERROR_S, LISTEN_ERROR); 
	} 

	fd_set readSet;
	struct timeval timeout = {1, 0};
	printf("Server listening on port: %d\n", serverOptions.port);

	while (SERV_RUNNING == 1) {

		FD_ZERO(&readSet);
		FD_SET(serverOptions.sock, &readSet);

		err = select(serverOptions.sock + 1, &readSet, NULL, NULL, &timeout);
		if (err == -1) {
			serverCleanup(serverOptions.sock);
			throwError(errorCode(2, SERVER_ERROR_H, SELECT_ERROR));
			continue;
		}

		if (FD_ISSET(serverOptions.sock, &readSet) == 0) continue;
		
		struct sockaddr_in cli_addr  = {0};
		int cli_addr_len = sizeof(cli_addr);		
		int acceptSocket = accept(serverOptions.sock, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_addr_len);
		if (acceptSocket == -1) {
			throwError(errorCode(2, SERVER_ERROR_H, ACCEPT_ERROR));
			continue;
		}
		
		char* cli_data = getClientAddress(cli_addr);
		if (cli_data == NULL) {
			throwError(errorCode(2, SERVER_ERROR_H, ALLOC_ERROR));
			continue; 
		}

		if (serverOptions.process_mode == 0) {

			struct ClientData cd;
			cd.sock = acceptSocket;
			cd.data = cli_data;
			int th = startThread((void*) handler, (void*) &cd);
			if (th < 0) {
				throwError(errorCode(2, SERVER_ERROR_H, th));
				continue;
			}
		} else {

			int argc = 3, arg_i = 0, arg_s = 0;
			char** cmd = (char**) malloc(sizeof(char*) * argc);
			if (cmd == NULL) {
				throwError(errorCode(2, SERVER_ERROR_H, ALLOC_ERROR));
				continue;
			}
			
			#ifndef __linux__
				argc += 5;
				cmd = (char**) realloc(cmd, argc);
				if (cmd == NULL) {
					throwError(errorCode(2, SERVER_ERROR_H, ALLOC_ERROR));
					continue;
				}

				cmd[arg_i++] = (char*) malloc(11);
				cmd[arg_i++] = (char*) malloc(11);
				cmd[arg_i++] = (char*) malloc(11);
				cmd[arg_i++] = (char*) malloc(strlen(serverOptions.address) + 1);
				cmd[arg_i++] = (char*) malloc(11);
				err = checkMalloc(cmd, arg_i);
				if (err == ALLOC_ERROR) {
					freeList(cmd, arg_i);
					throwError(errorCode(2, SERVER_ERROR_H, ALLOC_ERROR));
					continue;
				}
				sprintf(cmd[arg_s++], "%d", getWriter("LOGGER_PIPE"));
				sprintf(cmd[arg_s++], "%d", eventHandler("WRITE_LOG_EVENT"));
				sprintf(cmd[arg_s++], "%d", eventHandler("READ_LOG_EVENT"));
				sprintf(cmd[arg_s++], "%s", serverOptions.address);
				sprintf(cmd[arg_s++], "%d", serverOptions.port);
			#endif
			
			cmd[arg_i++] = (char*) malloc(11);
			cmd[arg_i++] = (char*) malloc(strlen(cli_data) + 1);
			cmd[arg_i++] = (char*) malloc(strlen(serverOptions.root_path) + 1);
			err = checkMalloc(cmd, arg_i);
	   		if (err == ALLOC_ERROR) {
	   			freeList(cmd, arg_i);
	   			throwError(errorCode(2, SERVER_ERROR_H, ALLOC_ERROR));
	   			continue;
	   		}
			sprintf(cmd[arg_s++], "%d", acceptSocket);
			sprintf(cmd[arg_s++], "%s", cli_data);
			sprintf(cmd[arg_s++], "%s", serverOptions.root_path);

			#ifndef __linux__
				err = startProcess("win32/listener.exe", argc, cmd);
			#else
				err = startProcess((void*) listener, 3, cmd);
			#endif

			if (err < 0) {
				throwError(errorCode(2, SERVER_ERROR_H, err));
				continue;
			}
			freeList(cmd, argc);
		}
	}
	SERV_RUNNING = 1;
	serverCleanup(serverOptions.sock);
	return 0;
}

//Reload the server
int serverReload() {

	int err;
	#ifndef __linux__
		WSADATA wsaData;
		err	= WSAStartup(514, &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
			serverCleanup(-1);
			return errorCode(2, SERVER_ERROR_R, WSA_ERROR);
		}
	#endif

	err = getConfig();
	if (err != 0) return errorCode(2, SERVER_ERROR_R, err);
	return 0;
}

//Clean the server
void serverCleanup(int sock) {

	#ifndef __linux__
		closesocket(sock);
		WSACleanup();
	#else
		close(sock);
	#endif
}

//Stop and destroy the server
void serverStop() {

	SERV_RUNNING = 0;
}