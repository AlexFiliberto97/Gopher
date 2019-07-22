#ifndef UTILS_H_

	#define UTILS_H_

	#include "utils.c"

	char* cpyalloc(char*);
	char* slice(char*, int, int);
	int countChar(char*, char, int**);
	char** splitPos(char*, int*, int);
	char** split(char*, char, int*);
	char* getKeyValue(char *, char **, char **, int);
	char** readlines(char*, int*);
	void printStringList(char**, int);
	void freeList(char**, int);
	char* getExtension(char*);
	struct Dict buildDict(char**, int);
	char* getAssocValueDict(char*, struct Dict);
	void freeDict(struct Dict);
	char* fixPath(char*);
	int searchList(char*, char**, int);
	int isNumeric(char*);

#endif