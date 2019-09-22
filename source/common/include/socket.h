#ifndef SOCKET_H_

    #define SOCKET_H_
	
	#ifndef __linux__
		#include "../../win32/include/mutex.h"
	#else
		#include "../../posix/include/mutex.h"
	#endif

	typedef struct _Opts {
		
		int sock;
		int used;
		int port;
		int process_mode;
		char* abs_root_path;
	} Opts;

	/* Global variables */
	extern Opts** serverOptions;
	extern int curSocket;
	extern char* address;
	MutexCV* sockMCV;

	void initOpts(Opts*);
	void freeOpts(Opts*);
	void destroyServerOpts();
	int initServerOpts();
	void closeSocket(int);
	void incrementSocket(int);
	void decrementSocket(int);
	int nextSocket(int, int*);
	void checkSockets();
	int setAddress(char*);
	
#endif