#ifndef NETWORK_H_

	#define NETWORK_H_

	#include "network.c"

	char* getClientAddress(struct sockaddr_in);
	char* recvAll(int, size_t*);
	int sendAll(int, char*, size_t);
	int checkPort(int);

#endif