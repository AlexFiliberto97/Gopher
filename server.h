#ifndef SERVER_H_
	
	#define SERVER_H_
	#include "server.c"

	int getOpts(int, char**);
	int getConfig();
	int serverInit(int, char**);
	int serverStart();
	int serverService();
	int serverReload();
	void serverCleanup(int);
	void serverStop();

#endif