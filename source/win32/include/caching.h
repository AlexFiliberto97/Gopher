#ifndef CACHING_H_
	
	#define CACHING_H_
	#include <windows.h>

	typedef struct _CachePage {
		
		long long h_item;
		HANDLE hFile;
		HANDLE hMap;
		OVERLAPPED* overlapped;
		void* view_ptr;
		int nPage;
		long long size;
		int used;
		int lru;
	} CachePage;

	int initCache();
	int createMapping(char*, long long, HANDLE*, HANDLE*, OVERLAPPED*);
	BOOL isFull();
	int incrementUsed(int);
	void decrementUsed(int);
	int checkCache(long long, int);
	int newPageIndex(long long, int);
	void* readMapping(char*, long long, long long, int*, int*);
	void destroyCache();

#endif