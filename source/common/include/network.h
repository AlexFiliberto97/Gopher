#ifndef NETWORK_H_

	#define NETWORK_H_

	#ifndef __linux__
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#else
		#include <netinet/in.h>
	#endif

	typedef struct _SendFileData {
		
		int sock;
		char* response;
		char* file;
		long long size;
		int process_mode;
		int err;
	} SendFileData;

	char* getClientAddress(struct sockaddr_in);
	char* recvAll(int, size_t*);
	int sendAll(int, char*, long long);
	int sendFileWinProc(SendFileData*);
	int sendFile(SendFileData*);
	int checkPort(int);
	int ipFormatCheck(char*);

#endif