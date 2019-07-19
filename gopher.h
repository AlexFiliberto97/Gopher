#ifndef GOPHER_H_

	#define GOPHER_H_
	
	#include "gopher.c"

	char getGopherType(char*, struct Dict);
	int setRootPath(char*);
	char **getDispNamesAssoc(char*, int*);
	char *getItem(char*, int*);
	char **gopherListDir(char*, char*, int*, struct HandlerData*);
	int checkEmptyRequest(char*);
	char *handleRequest(char*, size_t*, int*, struct HandlerData*, int);
	void* sendFile(void* input);
	int handler(void*, int);

#endif









