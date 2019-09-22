#ifndef UTILS_H_

	#define UTILS_H_

	typedef struct _Dict {
		
		char** keys;
		char** values;
		int length;
		int err;
	} Dict;

	char* cpyalloc(char*);
	char* slice(char*, int, int);
	int countChar(char*, char, int**);
	char** splitPos(char*, int*, int);
	char** split(char*, char, int*);
	char* getKeyValue(char *, char **, char **, int);
	int lastOccurrence(char*, char, int);
	char** readlines(char*, int*);
	void printStringList(char**, int);
	char* concatList(char**, int, char); 
	void freeList(char**, int);
	char* getExtension(char*, char*);
	Dict buildDict(char**, int);
	char* getAssocValue(char*, Dict);
	void freeDict(Dict);
	char* fixPath(char*);
	int searchList(char*, char**, int);
	int isNumeric(char*);
	int isValidPath(char*);
	long long hash_item(char*);
	int checkMalloc(char**, int);
	void writeErr(char*, int);
	int checkConfigFile();

#endif