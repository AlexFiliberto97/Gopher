#ifndef MAPPING_H_

	#define MAPPING_H_

	#include "mapping.c"

	void* createAndOpenMapping(char*, size_t*, int);
	int deleteMapping(void*, size_t);

#endif