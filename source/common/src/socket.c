#include <stdlib.h>
#include <stdio.h>
#include "../include/socket.h"
#include "../include/error.h"
#include "../include/const.h"
#include "../include/utils.h"

#ifndef __linux__
	#include <windows.h>
	#include "../../win32/include/mutex.h"
#else
	#include <unistd.h>
	#include "../../posix/include/mutex.h"
#endif

/* Global variables */
Opts** serverOptions = NULL;
MutexCV* sockMCV = NULL;
char* address = NULL;
int curSocket = -1;

/* Function: initOpts
*  Initialize a opts struct pointer.
*/
void initOpts(Opts* opts) {
	
	opts->sock = -1;
	opts->used = 0;
	opts->port = -1;
	opts->process_mode = 0;
	opts->abs_root_path = NULL;
}

/* Function: freeOpts
*  Free a opts struct pointer.
*/
void freeOpts(Opts* opts) {

	if (opts->abs_root_path != NULL) free(opts->abs_root_path);
	free(opts); 
}

/* Function: destroyServerOpts
*  Destroy server options.
*/
void destroyServerOpts() {

	for (int i = 0; i < N_SOCKETS; i++) {
		freeOpts(serverOptions[i]);
	}
	if (address != NULL) free(address);
	
	free(serverOptions);
	destroyMutexCV(sockMCV);
}

/* Function: initServerOpts
*  Initialize the server options.
*/
int initServerOpts() {

	curSocket = -1;
	serverOptions = (Opts**) malloc(sizeof(Opts*) * N_SOCKETS);
	if (serverOptions == NULL) return ALLOC_ERROR;

	for (int i = 0; i < N_SOCKETS; i++) {
		serverOptions[i] = (Opts*) malloc(sizeof(Opts));
		if (serverOptions[i] == NULL) {
			for (int k = 0; k < i; k++) {
				free(serverOptions[k]);
			}
			free(serverOptions);
			return ALLOC_ERROR;
		}
		initOpts(serverOptions[i]);
	}
	
	sockMCV = createMutexCV();
	if (sockMCV == NULL) {
		destroyServerOpts();
		return -1;
	}
	return 0;
}

/* Function: closeSocket
*  Close the given docket.
*/
void closeSocket(int sock) {
	
	#ifndef __linux__
		closesocket(sock);
	#else
		close(sock);
	#endif
}

/* Function: incrementSocket
*  Increment the number of consumers for the given socket.
*/
void incrementSocket(int index) {
	
	mutexLock(sockMCV->mutex);
	if (index != -1) serverOptions[index]->used++;
	mutexUnlock(sockMCV->mutex);
}

/* Function: incrementSocket
*  Decrement the number of consumers for the given socket.
*/
void decrementSocket(int index) {
	
	mutexLock(sockMCV->mutex);
		if (index != -1) {
			serverOptions[index]->used--;
			if (serverOptions[index]->used == 0 && index != curSocket) {
				closeSocket(serverOptions[index]->sock);
				serverOptions[index]->sock = -1;
				if (serverOptions[index]->abs_root_path != NULL) free(serverOptions[index]->abs_root_path);
				initOpts(serverOptions[index]);
			}
		}
	mutexUnlock(sockMCV->mutex);
}

/* Function: nextSocket
*  Return the index for next socket.
*/
int nextSocket(int port, int* reuse) {
	
	mutexLock(sockMCV->mutex);
		for (int i = 0; i < N_SOCKETS; i++) {
			if (serverOptions[i]->port == port) {
				*reuse = 1;
				mutexUnlock(sockMCV->mutex);
				return i;
			}
		}
	
		for (int i = 0; i < N_SOCKETS; i++) {
			if (serverOptions[i]->sock == -1) {
				*reuse = 0;
				mutexUnlock(sockMCV->mutex);
				return i;
			}
		}
	mutexUnlock(sockMCV->mutex);
	return -1;
}

/* Function: checkSockets
*  Free sockets with no consumers.
*/
void checkSockets() {
    
    mutexLock(sockMCV->mutex);
    	for (int i = 0; i < N_SOCKETS; i++) {
			if (serverOptions[i]->sock != -1 && serverOptions[i]->used == 0 && i != curSocket) {
				closeSocket(serverOptions[i]->sock);
				if (serverOptions[i]->abs_root_path != NULL) free(serverOptions[i]->abs_root_path);
				initOpts(serverOptions[i]);
			}
		}
    mutexUnlock(sockMCV->mutex);
}