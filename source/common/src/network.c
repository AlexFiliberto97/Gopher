#include <stdio.h>
#include <stdlib.h>
#include "../include/network.h"
#include "../include/error.h"
#include "../include/const.h"
#include "../include/utils.h"

#ifndef __linux__
	#include <windows.h> 
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include "../../win32/include/caching.h"
	#include "../../win32/include/utils_win32.h"
	#include "../../win32/include/locking.h"
	#include "../../win32/include/network.h"
#else
	#include <unistd.h>
	#include <string.h>
	#include <errno.h>
	#include <netinet/in.h>
	#include <netinet/ip.h>
	#include <sys/socket.h>
	#include <sys/file.h>
	#include <sys/mman.h>
	#include "../../posix/include/utils_posix.h"
	#include "../../posix/include/caching.h"
	#include "../../posix/include/locking.h"
	#include "../../posix/include/network.h"
#endif

/* Function: getClientAddress
*  Return the client ip address and port.
*/
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

	char* client_address = (char *) malloc(strlen(b1) +strlen(b2) +strlen(b3) +strlen(b4) +strlen(port) +5);
	if (client_address == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	sprintf(client_address, "%s.%s.%s.%s:%s", b1, b2, b3, b4, port);	
	return client_address;
}

/* Function: checkPort
*  Check if a given port is valid.
*/
int checkPort(int port) {

	return (port >= 1024 && port < 65535) ? 0 : -1;
}

/* Function: recvAll
*  Receive data from client.
*/
char* recvAll(int sock, size_t* len) {

	char* msg = (char *) malloc(1);
	if (msg == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	msg[0] = '\0';
	char recvbuf[RECVBUF_SIZE];
    int r = 0;
    size_t r_tot = 0;

    while ((r = recv(sock, recvbuf, RECVBUF_SIZE, 0)) > 0) {
    	r_tot += r;
    	msg = (char *) realloc(msg, r_tot);
    	if (msg == NULL) {
			throwError(ALLOC_ERROR);
			return NULL;
		}
		memcpy((void *) &msg[r_tot-r], (void *) recvbuf, r);
    	if (memcmp((void *) &msg[r_tot-2], (void *) "\r\n", 2) == 0) break;
    }

    *len = r_tot;
    return msg;
}

/* Function: sendAll
*  Send data to client.
*/
int sendAll(int sock, char* data, long long sz) {
	
	void* sendbuf = (void*) malloc(SENDBUF_SIZE);
	if (sendbuf == NULL) {
		throwError(ALLOC_ERROR);
		return SEND_ERROR;
	}

	long long bytes_sent = 0, bytes_left = sz, n_packet = 0; 
	int n_bytes;

	while (bytes_left > 0) {

		n_bytes = (bytes_left < SENDBUF_SIZE) ? bytes_left : SENDBUF_SIZE;
		memcpy((void*) sendbuf, (void*) &data[SENDBUF_SIZE * n_packet++], n_bytes);
		bytes_sent = send(sock, sendbuf, n_bytes, 0);
		if (bytes_sent == -1) {
			free(sendbuf);
			return SEND_ERROR;
		}

		bytes_left -= bytes_sent;
	}

	free(sendbuf);
	if (bytes_left > 0) return SEND_ERROR;
	return 0;
}

/* Function: sendFile
*  Send file to client.
*/
int sendFile(SendFileData* sfd) {

	if (sfd->process_mode == 1) return sendFileProc(sfd);

	void* view_ptr = NULL;
	long long offset = 0;
	int n_bytes = 0, cache_index = -1, err = 0;

	sfd->size = getFileSize(sfd->file);
	if (sfd->size < 0) return sfd->size;

	while (offset < sfd->size) {

		view_ptr = readMapping(sfd->file, sfd->size, offset, &n_bytes, &cache_index);
		if (view_ptr == NULL) return SEND_FILE_ERROR;

		err = sendAll(sfd->sock, (char*) view_ptr, n_bytes);
		decrementUsed(cache_index);
		if (err != 0) {
			throwError(err);
			return SEND_FILE_ERROR;
		}
		offset += n_bytes;
	}
	return 0;
}

/* Function: ipFormatCheck
*  Check if a given ip address is valid.
*/
int ipFormatCheck(char* address) {
	
	if (strlen(address) < MIN_IP_LEN || strlen(address) > MAX_IP_LEN) return BAD_ADDRESS;

	int count;
	char** list = split(address, '.', &count);
	if (list == NULL) return ALLOC_ERROR;
	if (count != 4) return BAD_ADDRESS;

	int err = 0;
	for (int i = 0; i < 4; i++) {
		if (strlen(list[i]) == 0 || strlen(list[i]) > 3 || isNumeric(list[i]) != 0 || atoi(list[i]) < 0 || atoi(list[i]) > 255) {
			err = BAD_ADDRESS;
			break;
		}
	}

	freeList(list, count);
	return err;
}