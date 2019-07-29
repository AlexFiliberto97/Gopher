#ifndef MAPPING_H_

	#define MAPPING_H_

	#include "mapping.c"

	void freeFileMapStruct(struct FileMap*);
	int createMapping(char*, char*, struct FileMap*);
	void* readMapping(struct FileMap*, long long, int*, int*);
	int deleteView(char* view_ptr, int, int);
	void closeMapping(struct FileMap*);

#endif