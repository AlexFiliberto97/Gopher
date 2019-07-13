#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "network.h"
#include "gopher.h"

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

#define CONFIG_FILE_PATH "config.txt"


void serverCleanup();

//Server serverOptions

struct Opts {

	int sock;
	char* address;
	int port;
	int process_mode;
	char* root_path;
};

static struct Opts serverOptions = {-1, "127.0.0.1", 7070, 0, "./"};

//Get server config from command line
int getOpts(int argc, char** args) {

	int port = -1, process_mode = -1;

	for (int i = 0; i < argc; i++) {
		if (strcmp(args[i], "-p") == 0 && i < argc -1 && isNumeric(args[i+1])) {
			port = atoi(args[i+1]);
			if (checkPort(port) == -1) return -1;
		} else if (strcmp(args[i], "-process") == 0) {
			process_mode = 1;
		}
	}
	if (port != -1) serverOptions.port = port; 
	if (process_mode != -1) serverOptions.process_mode = process_mode; 
	return 0;
}

//Get server config from file
int getConfig() {

	int config_count;
	char **config_assoc_list;
	config_assoc_list = readlines(CONFIG_FILE_PATH, &config_count);
	if (config_assoc_list == NULL) return -1;
	
	struct Dict config_dict = buildDict(config_assoc_list, config_count);
	if (config_dict.err != 0) return -1;

	freeList(config_assoc_list, config_count);
	char* address_v = getAssocValue("address", config_dict);
	char* port_v = getAssocValue("port", config_dict);
	char* process_mode_v = getAssocValue("process", config_dict);
	char* root_path_v = getAssocValue("root", config_dict);

	if (address_v == NULL || port_v == NULL || root_path_v == NULL || process_mode_v == NULL) return -1;
	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) != 0)) {
		freeDict(config_dict);
		return -1;
	}

	int port = atoi(port_v);
	int process_mode = atoi(process_mode_v);

	if (checkPort(port) != 0) {
		freeDict(config_dict);
		return -1;
	}

	if (process_mode != 0 && process_mode != 1) {
		freeDict(config_dict);
		return -1;
	}

	serverOptions.address = (char*) malloc(strlen(address_v) + 1);
	if (serverOptions.address == NULL) return -1;
	
	strcpy(serverOptions.address, address_v);
	serverOptions.port = port;
	serverOptions.process_mode = process_mode;
	serverOptions.root_path = (char*) malloc(strlen(root_path_v) + 1);
	if (serverOptions.root_path == NULL) return -1;
	
	strcpy(serverOptions.root_path, root_path_v);
	freeDict(config_dict);
	return 0;
}

//Initialize the server 
int serverInit(int argc, char** args) {

	int err = getConfig();
	if (err == -1) {
		return -1;
	} else {
		err = getOpts(argc, args);
		if (err == -1) return -1;
	}
	
	#ifndef __linux__
		WSADATA wsaData;
		err	= WSAStartup(514, &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2 || err != 0) {
			serverCleanup(-1);
			return -1;
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
		return -1; 
	}

	#ifdef __linux__
		err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
		if (err < 0) {
			serverCleanup(sock);
			return -1;
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
		return -1;
	}
	serverOptions.sock = sock;
	return 0;
}



int serverService() {

	int err = listen(serverOptions.sock, 10);
	if (err == -1) {
		serverCleanup(serverOptions.sock);
		return -1;
	} 

	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(serverOptions.sock, &readSet);
	printf("Server listening on port: %d\n", serverOptions.port);

	while (1) {

		err = select(serverOptions.sock +1, &readSet, NULL, NULL, NULL);
		if (err == -1) {
			serverCleanup(serverOptions.sock);
			return -1;
		}

		struct sockaddr_in cli_addr  = {0};
		int cli_addr_len = sizeof(cli_addr);
		
		int acceptSocket = accept(serverOptions.sock, (struct sockaddr*) &cli_addr, (socklen_t*) &cli_addr_len);
		if (acceptSocket == -1) {
			serverCleanup(serverOptions.sock);
			return -1;
		}
		
		char* cli_data = getClientAddress(cli_addr);
		if (cli_data == NULL) { 
			printf("client address not allocated\n"); 
			continue; 
		}

		printf("New connection from %s\n", getClientAddress(cli_addr));
		printf("New connection on descriptor: %d\n", acceptSocket);

		if (serverOptions.process_mode == 0) {

			struct ClientData cd;
			cd.sock = acceptSocket;
			cd.data = cli_data;
			//#ifndef __linux__
			startThread((void*)handler, (void*) &cd);
			//#else
				//startThread((void*) handler, (void*) &cd);
			//#endif
		} else {
   			#ifndef __linux__
				char** cmd = (char**) malloc(sizeof(char*) * 8);
	   			cmd[0] = (char*) malloc(11);
	   			cmd[1] = (char*) malloc(11);
	   			cmd[2] = (char*) malloc(11);
	   			cmd[3] = (char*) malloc(11);
	   			cmd[4] = (char*) malloc(strlen(cli_data) + 1);
	   			cmd[5] = (char*) malloc(strlen(serverOptions.address) + 1);
	   			cmd[6] = (char*) malloc(11);
	   			cmd[7] = (char*) malloc(strlen(serverOptions.root_path) + 1);

	   			sprintf(cmd[0], "%d", acceptSocket);
	   			sprintf(cmd[1], "%d", getWriter("LOGGER_PIPE"));
	   			sprintf(cmd[2], "%d", eventHandler("WRITE_LOG_EVENT"));
	   			sprintf(cmd[3], "%d", eventHandler("READ_LOG_EVENT"));
	   			sprintf(cmd[4], "%s", cli_data);
	   			sprintf(cmd[5], "%s", serverOptions.address);
	   			sprintf(cmd[6], "%d", serverOptions.port);
	   			sprintf(cmd[7], "%s", serverOptions.root_path);

				startProcess("win32/listener.exe", 8, cmd);
				freeList(cmd, 8);
			#else
				char** cmd = (char**) malloc(sizeof(char*) * 3);
	   			cmd[0] = (char*) malloc(11);
	   			cmd[1] = (char*) malloc(strlen(cli_data) + 1);
	   			cmd[2] = (char*) malloc(strlen(serverOptions.root_path) + 1);
	   			sprintf(cmd[0], "%d", acceptSocket);
	   			sprintf(cmd[1], "%s", cli_data);
	   			sprintf(cmd[2], "%s", serverOptions.root_path);
				startProcess((void*) listener, 3, cmd);
				freeList(cmd, 3);
			#endif

		
		}
	}
    return 0;
}

void serverCleanup(int sock) {

	#ifndef __linux__
		closesocket(sock);
		WSACleanup();
	#else
		close(sock);
	#endif
}