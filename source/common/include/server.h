#ifndef SERVER_H_
	
	#define SERVER_H_
	#include "socket.h"

	void decrementSocket(int);
	int getOpts(int, char**);
	Opts* getConfig(int);
	int serverInit();
	int serverStart(int, char**, int);
	int serverService();
	void serverCleanup(int);
	void serverStop();
	int reloadServer();

#endif