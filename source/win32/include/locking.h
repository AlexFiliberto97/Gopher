#ifndef LOCKING_H
	
	#define LOCKING_H
	
	BOOL lockFile(HANDLE, long long, OVERLAPPED*);
	BOOL unlockFile(HANDLE, long long, OVERLAPPED*);

#endif