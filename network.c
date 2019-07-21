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
	#include <errno.h>
	#include "posix/mapping.h"
#endif


#define SENDBUF_SIZE 1024
#define RECVBUF_SIZE 64

struct SendFileData {
	u_int sock;
	char* response;
	void** maps;
	long long response_sz;
	int err;
};


//jkkdfjebfkjwebcfkew
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
	if (client_address == NULL) {
		throwError(2, CLIENT_DATA, ALLOC_ERROR);
		return NULL;
	}

	sprintf(client_address, "%s.%s.%s.%s:%s", b1, b2, b3, b4, port);	
	printf("%s\n", client_address);
	return client_address;
}

//kjedjbwekcbwk
int checkPort(int port) {
	
	if(port >= 1024 && port < 65535) {
		return 0;
	}
	return -1;
}

//iuewfkjwefnewjfkwejfweb
char* recvAll(int sock, size_t* len) {
	
	char *msg = (char *) malloc(1);
	if (msg == NULL) {
		throwError(2, RECV_ERROR, ALLOC_ERROR);
		return NULL;
	}

	msg[0] = '\0';
	char recvbuf[RECVBUF_SIZE];
    int r;
    size_t r_tot = 0;

    while ((r = recv(sock, recvbuf, RECVBUF_SIZE, 0)) > 0) {
    	r_tot += r;
    	msg = (char *) realloc(msg, r_tot);
    	if (msg == NULL) {
			throwError(2, RECV_ERROR, ALLOC_ERROR);
			return NULL;
		}
		memcpy((void *) &msg[r_tot-r], (void *) recvbuf, r);
    	if (memcmp((void *) &msg[r_tot-2], (void *) "\r\n", 2) == 0) break;
    }

    *len = r_tot;
    return msg;
}


int sendAll(int sock, char* data, long long file_sz) {
	void *sendbuf = (void *) malloc(SENDBUF_SIZE);
	if (sendbuf == NULL) {
		throwError(1, ALLOC_ERROR);
		return SEND_ERROR;
	}

	long long bytes_sent;
	long long bytes_left = file_sz;
	long long n_packet = 0;

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

		if (bytes_sent == -1) {
			free(sendbuf);
			return SEND_ERROR;
		}
	}

	free(sendbuf);
	if (bytes_left > 0) return SEND_ERROR;
	return 0;

}
    
// int sendFile(int sock, void** maps, long long file_sz) {

	// int n_maps = file_sz / MAX_MAP_SIZE;
	// if (file_sz % MAX_MAP_SIZE > 0) n_maps++;

	// void *sendbuf = (void *) malloc(SENDBUF_SIZE);
	// if (sendbuf == NULL) {
	// 	printf("Errore in fun - malloc\n");
	// 	return 1;
	// }

	// for (int i = 0; i < n_maps; i++) {

	// 	printf("INVIO UNA BANANA %d\n", n_maps);

	// 	void* cur_map = maps[i];

	// 	long long bytes_sent, bytes_left;

	// 	if (i == n_maps - 1) {
	// 		bytes_left = file_sz % MAX_MAP_SIZE;
	// 	} else {
	// 		bytes_left = MAX_MAP_SIZE;
	// 	}

	// 	int n_packet = 0;

	// 	while (bytes_left > 0) {

	// 		if (bytes_left < SENDBUF_SIZE) {
	// 			memcpy((void*) sendbuf, &cur_map[SENDBUF_SIZE * n_packet++], bytes_left);
	// 			bytes_sent = send(sock, sendbuf, bytes_left, 0);
	// 			bytes_left -= bytes_sent;
	// 		} else {
	// 			memcpy((void*) sendbuf, &cur_map[SENDBUF_SIZE * n_packet++], SENDBUF_SIZE);
	// 			bytes_sent = send(sock, sendbuf, SENDBUF_SIZE, 0);
	// 			bytes_left -= bytes_sent;
	// 		}

	// 		if (bytes_sent == -1) {
	// 			free(sendbuf);
	// 			return SEND_ERROR;
	// 		}

	// 	}

	// 	if (bytes_left > 0) {
	// 		free(sendbuf);
	// 		printf("Errore in sendAll\n");
	// 		return 2;
	// 	}

	// }	

	// free(sendbuf);

	// return 0;

// }


int ipFormatCheck(char* address) {
	
	if (strlen(address) < 7 || strlen(address) > 15) return -1;

	int count;
	char** list = split(address, '.', &count);
	if (list == NULL) return ALLOC_ERROR;

	if (count != 4) return -1;

	int err = 0;

	for (int i = 0; i < 4; i++) {
		if (strlen(list[i]) == 0 || strlen(list[i]) > 3 || isNumeric(list[i]) != 0 || atoi(list[i]) < 0 || atoi(list[i]) > 255) {
			err = -1;
			break;
		}
	}

	freeList(list, count);
	return err;

}