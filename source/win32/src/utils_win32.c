#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/utils_win32.h"
#include "../../common/include/error.h"
#include "../../common/include/utils.h"

/* Function: getFileSize
*  Return the size of a given file (large files are supported).
*/
long long getFileSize(char* path) {
    
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(path, GetFileExInfoStandard, &fad)) return FILE_SIZE_ERROR;

    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return (long long) size.QuadPart;
}

/* Function: countDirElements
*  Return the number of elements into a specific directory.
*/
int countDirElements(char* path) {

	WIN32_FIND_DATA data;
	char* nPath = (char*) malloc(strlen(path) + 4);
	if (nPath == NULL) return ALLOC_ERROR;

	sprintf(nPath, "%s*.*", path);
	HANDLE hFind = FindFirstFile(nPath, &data);
	int c = 0;
	do {

		if (hFind == INVALID_HANDLE_VALUE) {
			free(nPath);
			return INVALID_HANDLE;
		}

		if (strcmp(data.cFileName, ".") != 0 && 
			strcmp(data.cFileName, "..") != 0 && 
			strcmp(data.cFileName, "_dispnames") != 0) {
			c++;
		}
	} while (FindNextFile(hFind, &data));

	FindClose(hFind);
	free(nPath);
	return c;
}

/* Function: readFile
*  Return the content of a gven file.
*/
char* readFile(char* fileName, int* ignore) {

	int err;
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((err = GetLastError()) != 0 || hFile == INVALID_HANDLE_VALUE) {
		throwError(INVALID_HANDLE);
		return NULL;
	}

	size_t sz = GetFileSize(hFile, NULL);
	if (sz == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		throwError(FILE_SIZE_ERROR);
		return NULL;
	}

	char *buf = (char*) malloc(sz + 1);
	if (buf == NULL) {
		CloseHandle(hFile);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	DWORD bytes_read;
	BOOL rf = ReadFile(hFile, buf, sz, &bytes_read, NULL);
	if ((err = GetLastError()) != 0 || rf == 0) {
		CloseHandle(hFile);
		free(buf);
		throwError(READ_FILE_ERROR);
		return NULL;
	}

	CloseHandle(hFile);
	buf[sz] = '\0';
	char *data = (char*) malloc(sizeof(char) * sz);
	if (data == NULL) {
		free(buf);
		throwError(ALLOC_ERROR);
		return NULL;
	}
	
	int idx = 0;
	for (int i = 0; i < strlen(buf); i++) {
		if (buf[i] == '\r' && buf[i+1] == '\n' && i + 1 < strlen(buf)) continue;
		data[idx++] = buf[i];
	}
	data[idx] = '\0';
	data = (char*) realloc(data, strlen(data) + 1);
	if (data == NULL) {
		free(buf);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	free(buf);
	return data;
}

/* Function: listDir
*  Return the elements of a specific directory.
*/
char** listDir(char* path, int* count) {

	int n = countDirElements(path);
	if (n < 0) {
		throwError(n);
		return NULL;
	} 

	*count = n;
	char** list = (char**) malloc(sizeof(char *) * n);
	if(list == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	WIN32_FIND_DATA data;
	char *nPath;
	nPath = (char*) malloc(strlen(path) + 4);
	if(nPath == NULL) {
		free(list);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	sprintf(nPath, "%s*.*", path);
	HANDLE hFind = FindFirstFile(nPath, &data);
	int c = 0;
	int len;
	do {

		if (hFind == INVALID_HANDLE_VALUE) {
			freeList(list, n);
			free(nPath);
			throwError(INVALID_HANDLE);
			return NULL;
		}

		if (strcmp(data.cFileName, ".") != 0 && 
			strcmp(data.cFileName, "..") != 0 && 
			strcmp(data.cFileName, "_dispnames") != 0) {

			len = strlen(data.cFileName);
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				list[c] = (char*) malloc(len + 2);
				if (list[c] == NULL) {
					freeList(list, n);
					FindClose(hFind);
					free(nPath);
					throwError(ALLOC_ERROR);
					return NULL;
				}
				sprintf(list[c], "%s/", data.cFileName);
			} else {
				list[c] = (char*) malloc(len + 1);
				if (list[c] == NULL) {
					freeList(list, n);
					FindClose(hFind);
					free(nPath);
					throwError(INVALID_HANDLE);
					return NULL;
				}
				sprintf(list[c], "%s", data.cFileName);
			}
			c++;
		}
	} while (FindNextFile(hFind, &data));

	FindClose(hFind);
	free(nPath);
	return list;
}

/* Function: existsDir
*  Return 0 if the given directory exists.
*/
int existsDir(char* path) {

	DWORD attributes = GetFileAttributes(path);
	if ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return 0;
	}
	return -1;
}

/* Function: existsFile
*  Return 0 if the given file exists.
*/
int existsFile(char* path) {

	DWORD attributes = GetFileAttributes(path);
	if (attributes != INVALID_FILE_ATTRIBUTES) {
		return 0;
	}
	return -1;
}

/* Function: appendToFile
*  Append a text to a given file.
*/
int appendToFile(char* path, char* text) {

	HANDLE hFile;
    DWORD dwBytesToWrite = strlen(text), dwBytesWritten;

    hFile = CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
    	return INVALID_HANDLE;
    }

    BOOL succ = WriteFile(hFile, text, dwBytesToWrite, &dwBytesWritten, NULL);
    if (!succ){
    	CloseHandle(hFile);
    	return WRITE_FILE_ERROR;
    }

    CloseHandle(hFile);
    return 0;
}

/* Function: getExtension
*  Returns the extension of a given file
*/
char* getExtension(char* filename, char* ignore) {
		
	char* ext;
	if (filename[strlen(filename) - 1] == '/') {
		ext = (char*) malloc(5);
		if (ext == NULL) {
			throwError(ALLOC_ERROR);
			return NULL;
		}
		strcpy(ext, "menu");
		return ext;
	}

	int lastDot = lastOccurrence(filename, '.', 0);
	if (lastDot < 0) return NULL;

	ext = slice(filename, lastDot + 1, strlen(filename));
	if(ext == NULL) return NULL;
	return ext;
}