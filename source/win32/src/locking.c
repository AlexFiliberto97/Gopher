#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/locking.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

/* Function: lockFile
*  File locking on a given file.
*/
BOOL lockFile(HANDLE hFile, long long size, OVERLAPPED* overlapped) {
	
	DWORD sizeHigh = (DWORD) (size >> 32);
	DWORD sizeLow = (DWORD) ((size << 32) >> 32);
	BOOL succ = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, sizeLow, sizeHigh, overlapped);
	if (!succ) {
		throwError(LOCK_FILE);
		CloseHandle(hFile);
	}
	return succ;
}

/* Function: unlockFile
*  File unlocking on a given file.
*/
BOOL unlockFile(HANDLE hFile, long long size, OVERLAPPED* overlapped) {
	
	DWORD sizeHigh = (DWORD) (size >> 32);
	DWORD sizeLow = (DWORD) ((size << 32) >> 32);
	BOOL succ = UnlockFileEx(hFile, 0, sizeLow, sizeHigh, overlapped);
	if (!succ) {
		throwError(UNLOCK_FILE);
		CloseHandle(hFile);
	}
	return succ;
}