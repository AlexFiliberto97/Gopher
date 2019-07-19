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
	char* abs_root_path;
};


//Global variables
static struct Opts serverOptions;
static int SERV_RUNNING = 1;


//Set default server options
/*
void setDefaultServerOptions() {

	//Forse vanno allocati con malloc chiedi a giorgio
	serverOptions.sock = -1;
	serverOptions.address = "127.0.0.1"; 
	serverOptions.port = 7070;
	serverOptions.process_mode = 0;
}
*/

void freeServerOptions() {
	
	serverOptions.sock = -1;
	serverOptions.port = -1;
	serverOptions.process_mode = 0;
	free(serverOptions.address);
	free(serverOptions.root_path);
	free(serverOptions.abs_root_path);
}


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
	char** config_assoc_list;
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

	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) == 0)) {
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
	serverOptions.abs_root_path = (char*) malloc(strlen(root_path_v) + 1);
	if (serverOptions.abs_root_path == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	}

	strcpy(serverOptions.abs_root_path, root_path_v);
	int last_occ = lastOccurrence(root_path_v, '/', -1);
	if (last_occ == -1) {
		freeDict(config_dict);
		return -1;
	}//CHECK LATER


	char* rel_root_path = slice(root_path_v, last_occ + 1, strlen(root_path_v));
	if (rel_root_path == NULL) {
		freeDict(config_dict);
		return -1;
	}//CHECK LATER

	serverOptions.root_path = (char*) malloc(strlen(rel_root_path) + 1);
	if (serverOptions.root_path == NULL) {
		freeDict(config_dict);
		return ALLOC_ERROR;
	}
	
	strcpy(serverOptions.root_path, rel_root_path);
	free(rel_root_path);
	freeDict(config_dict);
	return 0;
}

//Init the winsock
int serverInit(int argc, char** args) {

	// freeServerOptions();

	int confErr = getConfig();
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
			serverCleanup(-1);
			throwError(1, SERVER_ERROR_S);
			return WSA_ERROR;
		}
	#endif

	return 0;
}

//Start the server
int serverStart() {

	int err;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) { 
		serverCleanup(sock);
		throwError(1, SERVER_ERROR_S); 
		return SOCK_ERROR;
	}

	#ifndef __linux__
		BOOL optval = TRUE;
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &optval, sizeof(int));
	#else
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
		
	#endif

	if (err < 0) {
		serverCleanup(sock);
		throwError(1, SERVER_ERROR_S); 
		return SOCK_ERROR; 
	}

	//setGopherOptions(serverOptions.address, &serverOptions.port ,serverOptions.root_path);
	
	struct sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(serverOptions.address);
	service.sin_port = htons(serverOptions.port);
	err = bind(sock, (struct sockaddr*) &service, sizeof(service));
	if (err == -1) {
		serverCleanup(sock);
		throwError(1, SERVER_ERROR_S); 
		return BIND_ERROR;
	}
	serverOptions.sock = sock;
	return 0;
}


//Handle the incoming requests
int serverService() {

	int err = listen(serverOptions.sock, 10);
	if (err == -1) {
		serverCleanup(serverOptions.sock);
		throwError(1, SERVER_ERROR_S); 
		return LISTEN_ERROR;
	} 

	//fd_set readSet;
	//struct timeval timeout = {1, 0};
	printf("Server listening on port: %d\n", serverOptions.port);

	while (SERV_RUNNING == 1) {

		//printf("eanfjajkfaejkn 1 \n");
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(serverOptions.sock, &readSet);
		struct timeval timeout = {2, 0};
		//printf("eanfjajkfaejkn 2 \n");

		err = select(serverOptions.sock + 1, &readSet, NULL, NULL, &timeout);
		//printf("eanfjajkfaejkn 3 \n");
		if (err == -1) {
			#ifdef __linux__
				if (errno == 4) continue;
			#endif
			serverCleanup(serverOptions.sock);
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
			continue; 
		}

		
		if (serverOptions.process_mode == 0) {

			
			struct HandlerData* hd = (struct HandlerData*) malloc(sizeof(struct HandlerData));
			hd->cli_data = (char*) malloc(strlen(cli_data) + 1);
			hd->address = (char*) malloc(strlen(serverOptions.address) + 1);
			hd->root_path = (char*) malloc(strlen(serverOptions.root_path) + 1);
			hd->abs_root_path = (char*) malloc(strlen(serverOptions.abs_root_path) + 1);
			strcpy(hd->cli_data, cli_data);
			strcpy(hd->address, serverOptions.address);
			strcpy(hd->root_path, serverOptions.root_path);
			strcpy(hd->abs_root_path, serverOptions.abs_root_path);
			hd->sock = acceptSocket;
			hd->port = serverOptions.port;
		
			#ifndef __linux__
			
				int th = startThread((void*) handler, (void*) hd);
				if (th < 0) {
					//printf("Errore nell'avvio del thread handler\n");
					continue;
				}
			#else
				pthread_t th = startThread((void*) handler, (void*) hd, 1);
				if (th < 0) {
					//printf("Errore nell'avvio del thread handler\n");
					continue;
				}
			#endif

		} else {

			#ifndef __linux__
				int argc = 8;
				char** cmd = (char**) malloc(sizeof(char*) * 8);
	   			cmd[0] = (char*) malloc(11);
	   			cmd[1] = (char*) malloc(11);
	   			cmd[2] = (char*) malloc(11);
	   			cmd[3] = (char*) malloc(11);
	   			cmd[4] = (char*) malloc(strlen(cli_data) + 1);
	   			cmd[5] = (char*) malloc(strlen(serverOptions.address) + 1);
	   			cmd[6] = (char*) malloc(11);
	   			cmd[7] = (char*) malloc(strlen(serverOptions.root_path) + 1);
	   			cmd[8] = (char*) malloc(strlen(serverOptions.abs_root_path) + 1);

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
			#else
				struct HandlerData* hd = (struct HandlerData*) create_shared_memory(sizeof(struct HandlerData));
				hd->cli_data = (char*) create_shared_memory(strlen(cli_data) + 1);
				hd->address = (char*) create_shared_memory(strlen(serverOptions.address) + 1);
				hd->root_path = (char*) create_shared_memory(strlen(serverOptions.root_path) + 1);
				hd->abs_root_path = (char*) create_shared_memory(strlen(serverOptions.abs_root_path) + 1);
				strcpy(hd->cli_data, cli_data);
				strcpy(hd->address, serverOptions.address);
				strcpy(hd->root_path, serverOptions.root_path);
				strcpy(hd->abs_root_path, serverOptions.abs_root_path);
				hd->sock = acceptSocket;
				hd->port = serverOptions.port;
				err = startProcess((void*) listener, 1, (void*) hd);
			#endif

			if (err < 0) {
				throwError(2, SERVER_ERROR_H, err);
				continue;
			}

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
			throwError(1, SERVER_ERROR_R);
			return WSA_ERROR;
		}
	#endif

	err = getConfig();
	if (err != 0) {
		throwError(1, SERVER_ERROR_R);
		return err;
	}
	return 0;
}


//Clean the server
void serverCleanup(int sock) {
	freeServerOptions();
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