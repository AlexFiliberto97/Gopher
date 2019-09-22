#ifndef MAPPING_H_

	#define MAPPING_H_

	typedef struct _CachePage {
		
		long long h_item;
		int nPage;
		void* view_ptr;
		long long size;
		int used;
		int lru;
		int fd;
		struct flock lock;
	} CachePage;
	
	int initCache();
	int createMapping(char*, long long, long long, int);
	int isFull();
	int incrementUsed(int, long long);
	void decrementUsed(int);
	int checkCache(long long, int);
	int newPageIndex(long long, int);
	void* readMapping(char*, long long, long long, int*, int*);
	void destroyCache();

#endif