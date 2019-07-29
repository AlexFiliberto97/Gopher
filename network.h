#ifndef NETWORK_H_

	#define NETWORK_H_

	#include "network.c"

	char* getClientAddress(struct sockaddr_in);
	char* recvAll(int, size_t*);
	int sendAll(int, char*, long long);
	int sendFile(int, struct FileMap*);
	int checkPort(int);

#endif