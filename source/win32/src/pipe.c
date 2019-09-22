#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/pipe.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

/* Global variables */
Pipe loggerPipe;

/* Function: createLoggerPipe
*  create the logger pipe.
*/
int createLoggerPipe(){

	HANDLE hRead, hWrite;
	SECURITY_ATTRIBUTES pipeSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
	if (CreatePipe(&hRead, &hWrite, &pipeSA, 0) == FALSE) return PIPE_ERROR;

	loggerPipe.hRead = hRead;
	loggerPipe.hWrite = hWrite;
	return 0;
}

/* Function: getReader
*  Return the reader handle for logger pipe.
*/
HANDLE getReader() {

	return loggerPipe.hRead;
}

/* Function: getReader
*  Return the writer handle for logger pipe.
*/
HANDLE getWriter() {
		
	return loggerPipe.hWrite;
}

/* Function: addPipe
*  Set the logger pipe for an external process.
*/
void setPipe(HANDLE hRead, HANDLE hWrite) {

	loggerPipe.hRead = hRead;
	loggerPipe.hWrite = hWrite;
}

/* Function: writePipe
*  Write on the logger pipe.
*/
int writePipe(char* msg) {

	char sz[INTSTR], size[INTSTR];
	sprintf(sz, "%d", (int) strlen(msg) +1);
	for (int i = 0; i < INTSTR; i++) {
		size[i] = (i < INTSTR -strlen(sz)) ? '0' : sz[i -INTSTR +strlen(sz)];
	}
	size[INTSTR] = '\0';

	char* comp_msg = (char*) malloc(strlen(msg) +strlen(size) +1);
	if (comp_msg == NULL) return ALLOC_ERROR;

	sprintf(comp_msg, "%s%s", size, msg);
	DWORD written = 0;
	BOOL succ = WriteFile(loggerPipe.hWrite, comp_msg, strlen(comp_msg) +1, &written, NULL);
	if (succ == FALSE) printf("SUCC DIO\n");
	if (written < strlen(comp_msg) + 1) printf("STRLEN DIO\n");
	if (written < strlen(comp_msg) + 1 || succ == FALSE) {
		free(comp_msg);
		return WRITE_PIPE;
	}

	free(comp_msg);
	return 0;
}

/* Function: readPipe
*  Read from logger pipe.
*/
char* readPipe() {

	char size[INTSTR];
	DWORD bytes_read;
	BOOL succ = ReadFile(loggerPipe.hRead, size, INTSTR, &bytes_read, NULL);
	if (bytes_read < INTSTR || succ == FALSE) {
		throwError(READ_PIPE);
		return NULL;
	}

	size[INTSTR] = '\0';
	int sz = atoi(size);
	char* msg = (char*) malloc(sz);
	if (msg == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	succ = ReadFile(loggerPipe.hRead, msg, sz, &bytes_read, NULL);
	if (bytes_read < sz || succ == FALSE) {
		free(msg);
		throwError(READ_PIPE);
		return NULL;
	}
	return msg;
}

/* Function: destroyLoggerPipe
*  Destroy the logger pipe.
*/
void destroyLoggerPipe() {

	CloseHandle(loggerPipe.hRead);
	CloseHandle(loggerPipe.hWrite);
}