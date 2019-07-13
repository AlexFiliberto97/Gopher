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
	int countDirElements(char*);
	size_t getFileSize2(char* file_name);
	char* readFile(char*, size_t*);
	char** listDir(char*, int*);
	void* create_shared_memory(size_t);

#endif
