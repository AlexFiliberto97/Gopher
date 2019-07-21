#ifndef MAPPING_H_

	#define MAPPING_H_

	#include "mapping.c"

	// void** createAndOpenMapping(char*, long long*, int);
	void* createAndOpenMapping(char*, long long*, int);
	int deleteMapping(void*, size_t);

#endif