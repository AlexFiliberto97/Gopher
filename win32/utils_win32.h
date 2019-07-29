#ifndef UTILS_WIN32_H_

	#define UTILS_WIN32_H_

	#include <windows.h>
	#include "utils_win32.c"
	
	void freeArray(char**, int);
	long long getFileSize(char*);
	int countDirElements(char*);
	char* readFile(char*, int*);
	char** listDir(char*, int*);
	int existsDir(char*);
	int existsFile(char*);
	int appendToFile(char*, char*);

#endif