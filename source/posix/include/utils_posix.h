#ifndef UTILS_POSIX_H_

	#define UTILS_POSIX_H_
		
	int isDirectory(char*);
	int isRegularFile(char*);
	int existsDir(char*);
	int existsFile(char*);
	int countDirElements(char*);
	char** listDir(char*, int*);
	size_t getFileSize(char*);
	char* readFile(char*, int*);
	void* create_shared_memory(size_t);
	void free_shared_memory(void*, size_t);
	void lowerCase(char*);
	char* getExtension(char*, char*);

#endif