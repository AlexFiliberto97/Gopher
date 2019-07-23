#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAX_ERROR_LENGTH 90

#ifndef __linux__
	#include <windows.h>
#else
	#include <errno.h>
#endif

int getSystemError() {
	
	#ifndef __linux__
		return GetLastError();
	#else
		return errno;
	#endif
}

char* errorCode(int err) {

	char* msg = (char*) malloc (MAX_ERROR_LENGTH);
	if (msg == NULL) {
		return NULL;
	}

	switch(err) {
		case -11:
			return strcpy(msg, "-> Error initializing WSA environment.\n"); 
		case -12:
			return strcpy(msg, "-> Error creating the socket.\n"); 
		case -13:
			return strcpy(msg, "-> Error binding the socket.\n");
		case -14:
			return strcpy(msg, "-> Error listening on socket.\n");
		case -15:
			return strcpy(msg, "-> Error on socket select.\n");
		case -16:
			return strcpy(msg, "-> Error on socket accept.\n");
		case -17:
			return strcpy(msg, "-> Memory allocation failed (malloc/realloc).\n");
		case -18:
			return strcpy(msg, "-> Error setting the console handler.\n");
		case -19:
			return strcpy(msg, "-> Bad port specified in: cmd parameters, config file or both.\n");
		case -20:
			return strcpy(msg, "-> Bad root specified in config file.\n");
		case -21:
			return strcpy(msg, "-> Bad process-mode specified in config file (can only assume 0 or 1).\n");
		case -22:
			return strcpy(msg, "-> Server can't handle the request.\n");
		case -23:
			return strcpy(msg, "-> Server can't reload.\n");
		case -24:
			return strcpy(msg, "-> Server can't start.\n");
		case -25:
			return strcpy(msg, "->Error grabbing the size of specified file.\n"); //Duplicate of -28
		case -26:
			return strcpy(msg, "->Element not found.\n");
		case -27:
			return strcpy(msg, "->Invalid handle value.\n");
		case -28:
			return strcpy(msg, "-> Error grabbing the size of specified file.\n");
		case -29:
			return strcpy(msg, "-> Cannot find the specified folder.\n");
		case -30:
			return strcpy(msg, "-> Cannot find the specified file.\n");
		case -31:
			return strcpy(msg, "-> Error writing on file.\n");
		case -32:
			return strcpy(msg, "-> Max number of threads that can be created has been reached.\n");
		case -33:
			return strcpy(msg, "-> Cannot start the thread.\n");
		case -34:
			return strcpy(msg, "-> Error in listener process.\n");
		case -35:
			return strcpy(msg, "-> Error in logger process.\n");
		case -36:
			return strcpy(msg, "Error creating the pipe.\n");
		case -38:
			return strcpy(msg, "-> Error writing on pipe.\n");
		case -39:
			return strcpy(msg, "-> Error reading from pipe.\n");
		case -40:
			return strcpy(msg, "-> Error creating file mapping or shared memory.\n");
		case -41:
			return strcpy(msg, "-> Error opening file mapping.\n");
		case -42:
			return strcpy(msg, "-> Error reading from file mapping.\n");
		case -43:
			return strcpy(msg, "-> Error deleting file mapping.\n");
		case -44:
			return strcpy(msg, "-> Cannot find the specified event.\n");
		case -45:
			return strcpy(msg, "-> Max number of events that can be created has been reached.\n");
		case -46:
			return strcpy(msg, "-> Error creating event.\n");
		case -47:
			return strcpy(msg, "-> Error setting the specified event.\n");
		case -48:
			return strcpy(msg, "-> Max number of processes that can be created has been reached.\n");
		case -49:
			return strcpy(msg, "-> Error creating process.\n");
		case -50:
			return strcpy(msg, "-> Error grabbing client's ip address and/or port.\n");
		case -51:
			return strcpy(msg, "-> Error receiving data from client.\n");
		case -52:
			return strcpy(msg, "-> Error sending data to client.\n");
		case -53:
			return strcpy(msg, "-> Error locking the specified file.\n");
		case -54:
			return strcpy(msg, "-> Error unlocking the specified file.\n");
		case -55:
			return strcpy(msg, "-> Error reading from file.\n");
		case -56:
			return strcpy(msg, "-> The specified ip address is not valid.\n");
		case -57:
			return strcpy(msg, "-> Environment can't start.\n");
		case -58:
			return strcpy(msg, "-> Error joining thread.\n");
		case -59:
			return strcpy(msg, "-> Cannot open folder.\n");
		case -60:
			return strcpy(msg, "-> Cannot open file.\n");
		case -61:
			return strcpy(msg, "-> Error forking process.\n");
		case -62:
			return strcpy(msg, "-> Error waitpid.\n");
		case -63:
			return strcpy(msg, "-> Error closing file descriptor.\n");
		case -64:
			return strcpy(msg, "-> Error setting the signal handler.\n");
		case -65:
			return strcpy(msg, "-> Error setting the signal handler.\n");
	}
	return NULL;
}

void throwError(int num, ...) {

	va_list valist;
   	va_start(valist, num);

   	FILE* errLog = fopen("error.log", "a+b");
   	if (errLog ==  NULL) {
   		return;
   	}

   	fwrite("Error >>>\n", 1, strlen("Error >>>\n"), errLog);
   
   	for (int i = 0; i < num; i++) {

        int errCode = va_arg(valist, int);
   		
   		char* msg = errorCode(errCode);
   		if (msg != NULL) {
   			fwrite(msg, 1, strlen(msg), errLog);
   			free(msg);
   		}
   	}

   	int sysErr = getSystemError();
   	if(sysErr != 0 && sysErr != 183) {

   		char err[11];
   		sprintf(err, "%d", sysErr);
   		fwrite(err, 1, strlen(err), errLog);

   	}
   	fwrite(" >>>\n\n", 1, strlen(">>>\n\n"), errLog);
   	fclose(errLog);
}