#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winnt.h>
#include <winbase.h>
#include "../error.h"


void freeArray(char** list, int c) {
	
	for (int i = 0; i < c; i++) {
		if(list[i] != NULL) {
			free(list[i]);
		}
	}
	free(list);
}


//Count the elements into a directory
int countDirElements(char *path) {

	WIN32_FIND_DATA data;
	char *nPath = (char *) malloc(strlen(path) + 4);
	if(nPath == NULL) return ALLOC_ERROR;
	
	sprintf(nPath, "%s*.*", path);
	HANDLE hFind = FindFirstFile(nPath, &data);
	int c = 0;
	do {

		if (hFind == INVALID_HANDLE_VALUE) {
			free(nPath);
			return ALLOC_ERROR;
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

//Read a file using win32 APIs
char* readFile(char* fileName) {

	int err;
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((err = GetLastError()) != 0 | hFile == INVALID_HANDLE_VALUE) return NULL;
	
	size_t sz = GetFileSize(hFile, NULL);
	if (sz == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		return NULL;
	}

	char *buf = (char *) malloc(sz + 1);
	if (buf == NULL) {
		CloseHandle(hFile);
		return NULL;
	}

	DWORD bytes_read;
	BOOL rf = ReadFile(hFile, buf, sz, &bytes_read, NULL);
	if ((err = GetLastError()) != 0 | rf == 0) {
		CloseHandle(hFile);
		free(buf);
		return NULL;
	}
	
	CloseHandle(hFile);
	buf[sz] = '\0';
	char *data = (char *) malloc(sizeof(char) * sz);
	if (data == NULL) {
		free(buf);
		return NULL;
	}

	int idx = 0;
	for (int i = 0; i < strlen(buf); i++) {
		if (buf[i+1] == '\n' && i + 1 < strlen(buf)) continue;
		data[idx++] = buf[i];
	}

	data[idx] = '\0';
	data = (char *) realloc(data, strlen(data) + 1);
	if (data == NULL) {
		free(buf);
		return NULL;
	}
	free(buf);
	return data;
} 

//List directory
char** listDir(char* path, int* count) {

	int n = countDirElements(path);
	if (n != 0) return NULL;

	*count = n;
	char **list = (char **) malloc(sizeof(char *) * n);
	if(list == NULL) return NULL;

	WIN32_FIND_DATA data;
	char *nPath;
	nPath = (char *) malloc(strlen(path) + 4);
	if(nPath == NULL) {
		freeArray(list, n);
		return NULL;
	}

	sprintf(nPath, "%s*.*", path);
	int c = 0, len;
	HANDLE hFind = FindFirstFile(nPath, &data);
	do {

		if (hFind == INVALID_HANDLE_VALUE) return NULL;
		if (strcmp(data.cFileName, ".") != 0 && 
			strcmp(data.cFileName, "..") != 0 && 
			strcmp(data.cFileName, "_dispnames") != 0) {

			len = strlen(data.cFileName);

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				list[c] = (char *) malloc(len + 2);
				if (list[c] == NULL) {
					freeArray(list, n);
					CloseHandle(hFind);
					return NULL;
				}
				sprintf(list[c], "%s/", data.cFileName);
			} else {
				list[c] = (char *) malloc(len + 1);
				if (list[c] == NULL) {
					freeArray(list, n);
					CloseHandle(hFind);
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

//Check if a directory exists
int existsDir(char* path) {

	DWORD attributes = GetFileAttributes(path);
	if ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return 0;
	}
	return NOT_FOUND;
}

//Append a text to file
int appendToFile(char* path, char* text) {

	HANDLE hFile;
    DWORD dwBytesToWrite = strlen(text), dwBytesWritten;

    hFile = CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return -1;

    BOOL succ = WriteFile(hFile, text, dwBytesToWrite, &dwBytesWritten, NULL);
    if (!succ) {
    	CloseHandle(hFile);
    	return ALLOC_ERROR;
    } 
    CloseHandle(hFile);
    return 0;
}

//Check if a file exists
int existsFile(char* path) {

	DWORD attributes = GetFileAttributes(path);
	if (attributes != INVALID_FILE_ATTRIBUTES) {
		return 0;
	}
	return NOT_FOUND;
}


