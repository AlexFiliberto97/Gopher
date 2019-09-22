#ifndef GOPHER_H_

	#define GOPHER_H_
	#include "../include/utils.h"
	#include "../include/network.h"

	typedef struct _HandlerData {
		
		int sock;
		int port;
		char* cliData;
		char* address;
		char* abs_root_path;
		int process_mode;
	} HandlerData;

	void freeHandlerDataStruct(HandlerData*);
	char getGopherType(char*, char*, Dict);
	char** getDispNamesAssoc(char*, int*);
	char* getItem(char*, int*);
	char** gopherListDir(char*, char*, int*, HandlerData*);
	int handleRequest(char*, SendFileData*, HandlerData*);
	void* sendResponse(void*);
	int responseThread(SendFileData*);
	int handler(void*);

#endif