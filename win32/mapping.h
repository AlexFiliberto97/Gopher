
#ifndef MAPPING_H_

	#define MAPPING_H_

	#include <windows.h>
	#include "mapping.c"

	HANDLE createMapping(char*, char*, size_t*, int);
	HANDLE openMapping(char*);
	char* readMapping(HANDLE);
	int deleteMapping(char*);

#endif