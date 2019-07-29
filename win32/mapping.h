
#ifndef MAPPING_H_

	#define MAPPING_H_

	#include <windows.h>
	#include "mapping.c"

	int createMapping(char*, char*, struct FileMap*);
	HANDLE openMapping(char*);
	void* readMapping(struct FileMap*, long long, int*, int*);
	int deleteMapping(char*, int);

#endif