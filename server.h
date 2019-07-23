#ifndef SERVER_H_
	
	#define SERVER_H_
	
	#include "server.c"

	void freeServerOptions();
	int getOpts(int, char**);
	int getConfig();
	int serverInit(int, char**);
	int serverStart();
	int serverService();
	void serverCleanup(int);
	void serverStop();
	void serverDestroy();

#endif