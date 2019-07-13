#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winnt.h>
#include <winbase.h>

int countDirElements(char *path) {

	WIN32_FIND_DATA data;
	char *nPath = (char *) malloc(strlen(path) + 4);

	if(nPath == NULL) {
		printf("Errore in countDirElements - malloc\n");
		return -1;
	}
	sprintf(nPath, "%s*.*", path);

	HANDLE hFind = FindFirstFile(nPath, &data);
	int c = 0;

	do {

		if (hFind == INVALID_HANDLE_VALUE) {
			printf("Errore in countDirElements - Find(First|Next)File\n");
			return -1;
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

char *readFile(char *fileName) {

	int err;

	HANDLE hFile = CreateFile(
					   fileName, // LPCTSTR lpFileName
				       GENERIC_READ, // DWORD dwDesiredAccess
				       0, // DWORD dwShareMode
				       NULL, // LPSECURITY_ATTRIBUTES lpSecurityAttributes
				       OPEN_EXISTING, //DWORD dwCreationDisposition
				       FILE_ATTRIBUTE_NORMAL, // DWORD dwFlagsAndAttributes,
					   NULL // HANDLE hTemplateFile
				   );

	if ((err = GetLastError()) != 0 | hFile == INVALID_HANDLE_VALUE) {
		printf("Errore in readFileWin32 - CreateFile: %d\n", err);
		return NULL;
	}

	size_t sz = GetFileSize(hFile, NULL);

	if (sz == INVALID_FILE_SIZE) {
		printf("Errore in readFileWin32 - GetFileSize: %d\n", GetLastError());
		return NULL;
	}

	char *buf = (char *) malloc(sz + 1);

	if (buf == NULL) {
		printf("Errore in readFileWin32 - malloc\n");
		return NULL;
	}

	DWORD bytes_read;

	BOOL rf = ReadFile(
			      hFile, // HANDLE hFile,
				  buf, // LPVOID lpBuffer,
				  sz, // DWORD nNumberOfBytesToRead,
				  &bytes_read, // LPDWORD lpNumberOfBytesRead,
				  NULL // LPOVERLAPPED lpOverlapped
			  );

	if ((err = GetLastError()) != 0 | rf == 0) {
		printf("Errore in readFileWin32 - ReadFile: %d\n", err);
		return NULL;
	}

	CloseHandle(hFile);

	buf[sz] = '\0';

	char *data = (char *) malloc(sizeof(char) * sz);

	int idx = 0;

	for (int i = 0; i < strlen(buf); i++) {
		if (buf[i+1] == '\n' && i + 1 < strlen(buf)) continue;
		data[idx++] = buf[i];
	}
	data[idx] = '\0';

	data = (char *) realloc(data, strlen(data) + 1);

	free(buf);
	return data;

} 


/* Function: listDir
* -------------------------------
*  
*/
char **listDir(char *path, int *count) {

	int n = countDirElements(path);

	if (n == -1) {
		printf("Errore in listDir - countDirElements\n");
		return NULL;
	}

	*count = n;
	char **list = (char **) malloc(sizeof(char *) * n);

	if(list == NULL) {
		printf("Errore in listDir - malloc\n");
		return NULL;
	}

	WIN32_FIND_DATA data;
	char *nPath;

	nPath = (char *) malloc(strlen(path) + 4);
	if(nPath == NULL) {
		printf("Errore in listDir - malloc\n");
		return NULL;
	}

	sprintf(nPath, "%s*.*", path);

	HANDLE hFind = FindFirstFile(nPath, &data);

	int c = 0;
	int len;

	do {

		if (hFind == INVALID_HANDLE_VALUE) {
			printf("Errore in listDir - Find(First|Next)File\n");
			return NULL;
		}

		if (strcmp(data.cFileName, ".") != 0 && 
			strcmp(data.cFileName, "..") != 0 && 
			strcmp(data.cFileName, "_dispnames") != 0) {

			len = strlen(data.cFileName);

			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				list[c] = (char *) malloc(len + 2);
				if (list[c] == NULL) {
					printf("Errore in listDir - malloc - dir\n");
					return NULL;
				}
				sprintf(list[c], "%s/", data.cFileName);
			} else {
				list[c] = (char *) malloc(len + 1);
				if (list[c] == NULL) {
					printf("Errore in listDir - malloc - file\n");
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

int existsDir(char* path) {

	DWORD attributes = GetFileAttributes(path);

	if ((attributes != INVALID_FILE_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
		return 0;
	}
	return 1;
}

int appendToFile(char* path, char* text) {

	HANDLE hFile;
    DWORD dwBytesToWrite = strlen(text), dwBytesWritten;

    hFile = CreateFile(path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) return -1;

    BOOL succ = WriteFile(hFile, text, dwBytesToWrite, &dwBytesWritten, NULL);
    if (!succ) return -1;

    CloseHandle(hFile);
    return 0;
}