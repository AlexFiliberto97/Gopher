#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#ifndef __linux__
	#include <windows.h> 
	#include <winsock2.h>
#else
	#include <unistd.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/ip.h>
#endif

#define SENDBUF_SIZE 1024
#define RECVBUF_SIZE 64


//Structures
struct SendFileData {
	
	u_int sock;
	char* response;
	size_t response_sz;
};

struct ClientData {
	
	int sock;
	char* data;
};


//return the client address
char* getClientAddress(struct sockaddr_in cli_addr) {

	char port[6], b1[4], b2[4], b3[4], b4[4];
	sprintf(port, "%d", (int) cli_addr.sin_port);
	struct in_addr cli_addr_ip = cli_addr.sin_addr;

	#ifndef __linux__
		sprintf(b1, "%d", (int) cli_addr_ip.S_un.S_un_b.s_b1);
		sprintf(b2, "%d", (int) cli_addr_ip.S_un.S_un_b.s_b2);
		sprintf(b3, "%d", (int) cli_addr_ip.S_un.S_un_b.s_b3);
		sprintf(b4, "%d", (int) cli_addr_ip.S_un.S_un_b.s_b4);
	#else
		unsigned int addr = cli_addr_ip.s_addr;
		unsigned int bb1 = addr;
		bb1 = bb1 >> 24;
		unsigned int bb2 = addr;
		bb2 = bb2 << 8;
		bb2 = bb2 >> 24;
		unsigned int bb3 = addr;
		bb3 = bb3 << 16;
		bb3 = bb3 >> 24;
		unsigned int bb4 = addr;
		bb4 = bb4 << 24;
		bb4 = bb4 >> 24;
		sprintf(b1, "%u", bb4);
		sprintf(b2, "%u", bb3);
		sprintf(b3, "%u", bb2);
		sprintf(b4, "%u", bb1);
	#endif

	char* client_address = (char *) malloc(strlen(b1) + strlen(b2) + strlen(b3) + strlen(b4) + strlen(port) + 5);
	if (client_address == NULL) return NULL;

	sprintf(client_address, "%s.%s.%s.%s:%s", b1, b2, b3, b4, port);	
	return client_address;
}

//Chek if a port is valid
int checkPort(int port) {
	
	if(port >= 1024 && port < 65535) {
		return 0;
	}
	return -1;
}

//Receive all function
char* recvAll(int sock, size_t* len) {
	
	char *msg = (char *) malloc(1);
	if (msg == NULL) return NULL;
	
	msg[0] = '\0';
	char recvbuf[RECVBUF_SIZE];
    size_t r_tot = 0, r;
    while ((r = recv(sock, recvbuf, RECVBUF_SIZE, 0)) > 0) {
    	r_tot += r;
    	msg = (char *) realloc(msg, r_tot);
    	if (msg == NULL) return NULL;
	
		memcpy((void *) &msg[r_tot-r], (void *) recvbuf, r);
    	if (memcmp((void *) &msg[r_tot-2], (void *) "\r\n", 2) == 0) break;
    }
    *len = r_tot;
    return msg;
}

//Send all function
int sendAll(int sock, char* data, size_t data_sz) {

	void *sendbuf = (void *) malloc(SENDBUF_SIZE);
	if (sendbuf == NULL) return ALLOC_ERROR;

	size_t bytes_sent;
	size_t bytes_left = data_sz;
	int n_packet = 0;
	while (bytes_left > 0) {
		if (bytes_left < SENDBUF_SIZE) {
			memcpy((void *) sendbuf, (void *) &data[SENDBUF_SIZE * n_packet++], bytes_left);
			bytes_sent = send(sock, sendbuf, bytes_left, 0);
			bytes_left -= bytes_sent;
		} else {
			memcpy((void *) sendbuf, (void *) &data[SENDBUF_SIZE * n_packet++], SENDBUF_SIZE);
			bytes_sent = send(sock, sendbuf, SENDBUF_SIZE, 0);
			bytes_left -= bytes_sent;
		}
	}
	free(sendbuf);
	if (bytes_left > 0) return 2;

	return 0;
}