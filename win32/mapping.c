#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../error.h"

//Create a new file mapping
HANDLE createMapping(char* path, char* mapName, long long* size, int process_mode) {

	HANDLE hFile, hMap;
	BOOL succ;

	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		throwError(1, INVALID_HANDLE);
		return NULL;
	}

	*size = GetFileSize(hFile, NULL);
	OVERLAPPED overlapped = {0};
	succ = LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0, *size, 0, &overlapped);
	if (!succ) {
		throwError(1, LOCK_FILE);
		CloseHandle(hFile);
		return NULL;
	}

	hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, mapName);
	if (hMap == NULL) {
		CloseHandle(hFile);
		return NULL;
	}

	succ = UnlockFileEx(hFile, 0, *size, 0, &overlapped);
	if (!succ) {
		throwError(1, UNLOCK_FILE);
		CloseHandle(hFile);
		return NULL;
	}

	CloseHandle(hFile);
	return hMap;
}

//Open an existingfile mapping
HANDLE openMapping(char *mapName) {
	
	HANDLE hMap;
	hMap = OpenFileMapping(FILE_MAP_READ, TRUE, mapName);
	if (hMap == NULL) return NULL;
	return hMap;
}

//Read an existing file mapping
char* readMapping(HANDLE hMap) {

	char *pMap;
	pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (pMap == NULL) return NULL;
	return pMap;
}

//Delete an existing file mapping
int deleteMapping(char *pMap) {

	BOOL succ;
	succ = UnmapViewOfFile(pMap);
	if (!succ) return DELETE_MAPPING;
	return 0;
}