#ifndef UTILS_WIN32_H_

	#define UTILS_WIN32_H_

	#include <stdio.h>
	#include "utils_win32.c"

	int setKeyboardEvent();
	int countDirElements(char*) ;
	char* readFile(char*, int*);
	char** listDir(char*, int*);
	int existsDir(char*);
	int appendToFile(char*, char*);

#endif