#ifndef UTILS_POSIX_H_

	#define UTILS_POSIX_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <string.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include "utils_posix.c"

	int isDirectory(const char*);
	int isRegularFile(const char*);
	int existsDir(char*);
	int existsFile(char*);
	void freeArray(char **, int);
	int countDirElements(char*);
	char** listDir(char*, int*);
	size_t getFileSize(char*);
	char* readFile(char*, size_t*);
	void* create_shared_memory(size_t);
	int free_shared_memory(void*, size_t);

#endif
