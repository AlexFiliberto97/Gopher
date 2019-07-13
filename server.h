#ifndef SERVER_H_
	
	#define SERVER_H_
	
	#include "server.c"

	int getOpts(int, char**);
	int getConfig();
	int serverInit(int, char**);
	int serverService();
	int serverStart();
	void serverCleanup(int);

#endif