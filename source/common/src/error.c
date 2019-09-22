#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../include/error.h"
#include "../include/const.h"
#include "../include/utils.h"

#ifndef __linux__
	#include <windows.h>
#else
	#include <syslog.h>
	#include <errno.h>
#endif

/* Function: errorCode
*  Return the error message for a given error code.
*/
char* errorCode(int err) {

	char* msg = (char*) malloc(MAX_ERROR_LENGTH);
	if (msg == NULL) return NULL;

	switch(err) {
		case -11:
			return strcpy(msg, "\n-> Error initializing WSA environment"); 
		case -12:
			return strcpy(msg, "\n-> Error creating the socket"); 
		case -13:
			return strcpy(msg, "\n-> Error binding the socket");
		case -14:
			return strcpy(msg, "\n-> Error listening on socket");
		case -15:
			return strcpy(msg, "\n-> Error on socket select");
		case -16:
			return strcpy(msg, "\n-> Error on socket accept");
		case -17:
			return strcpy(msg, "\n-> Memory allocation failed (malloc/realloc)");
		case -18:
			return strcpy(msg, "\n-> Error setting the console handler");
		case -19:
			return strcpy(msg, "\n-> Bad port specified in: cmd parameters, config file or both");
		case -20:
			return strcpy(msg, "\n-> Bad root specified in config file");
		case -21:
			return strcpy(msg, "\n-> Bad process-mode specified in config file (can only assume 0 or 1)");
		case -22:
			return strcpy(msg, "\n-> Server can't handle the request");
		case -23:
			return strcpy(msg, "\n-> Server can't reload");
		case -24:
			return strcpy(msg, "\n-> Server can't start");
		case -25:
			return strcpy(msg, "\n->Error grabbing the size of specified file");
		case -26:
			return strcpy(msg, "\n->Element not found");
		case -27:
			return strcpy(msg, "\n->Invalid handle value");
		case -28:
			return strcpy(msg, "\n-> Error grabbing the size of specified file");
		case -29:
			return strcpy(msg, "\n-> Cannot find the specified folder");
		case -30:
			return strcpy(msg, "\n-> Cannot find the specified file");
		case -31:
			return strcpy(msg, "\n-> Error writing on file");
		case -32:
			return strcpy(msg, "\n-> Max number of threads that can be created has been reached");
		case -33:
			return strcpy(msg, "\n-> Cannot start the thread");
		case -34:
			return strcpy(msg, "\n-> Error in listener process");
		case -35:
			return strcpy(msg, "\n-> Error in logger process");
		case -36:
			return strcpy(msg, "\n-> Error creating the pipe");
		case -38:
			return strcpy(msg, "\n-> Error writing on pipe");
		case -39:
			return strcpy(msg, "\n-> Error reading from pipe");
		case -40:
			return strcpy(msg, "\n-> Error creating file mapping");
		case -41:
			return strcpy(msg, "\n-> Error opening file mapping");
		case -42:
			return strcpy(msg, "\n-> Error reading from file mapping");
		case -43:
			return strcpy(msg, "\n-> Error deleting file mapping");
		case -44:
			return strcpy(msg, "\n-> Cannot find the specified event");
		case -45:
			return strcpy(msg, "\n-> Max number of events that can be created has been reached");
		case -46:
			return strcpy(msg, "\n-> Error creating event");
		case -47:
			return strcpy(msg, "\n-> Error setting the specified event");
		case -48:
			return strcpy(msg, "\n-> Max number of processes that can be created has been reached");
		case -49:
			return strcpy(msg, "\n-> Error creating process");
		case -50:
			return strcpy(msg, "\n-> Error grabbing client's ip address and/or port");
		case -51:
			return strcpy(msg, "\n-> Error receiving data from client");
		case -52:
			return strcpy(msg, "\n-> Error sending data to client");
		case -53:
			return strcpy(msg, "\n-> Error locking the specified file");
		case -54:
			return strcpy(msg, "\n-> Error unlocking the specified file");
		case -55:
			return strcpy(msg, "\n-> Error reading from file");
		case -56:
			return strcpy(msg, "\n-> The specified ip address is not valid");
		case -57:
			return strcpy(msg, "\n-> Environment can't start");
		case -58:
			return strcpy(msg, "\n-> Error joining thread");
		case -59:
			return strcpy(msg, "\n-> Cannot open folder");
		case -60:
			return strcpy(msg, "\n-> Cannot open file");
		case -61:
			return strcpy(msg, "\n-> Error forking process");
		case -62:
			return strcpy(msg, "\n-> Error waitpid");
		case -63:
			return strcpy(msg, "\n-> Error closing file descriptor");
		case -64:
			return strcpy(msg, "\n-> Error setting the signal handler");
		case -65:
			return strcpy(msg, "\n-> Error setting up daemon");
		case -66:
			return strcpy(msg, "\n-> Error sending file");
		case -67:
			return strcpy(msg, "\n-> Error creating map view of file");
		case -68:
			return strcpy(msg, "\n-> Error Initializing the cache");
		case -69:
			return strcpy(msg, "\n-> Error pulsing an ecent");
		case -70:
			return strcpy(msg, "\n-> Error unmapping map view of file");
		case -71:
			return strcpy(msg, "\n-> Error while waiting for a mutex");
		case -72:
			return strcpy(msg, "\n-> Error releasing a mutex");
		case -73:
			return strcpy(msg, "\n-> Error initializing threads");
		case -74:
			return strcpy(msg, "\n-> Error terminating logger");
		case -75:
			return strcpy(msg, "\n-> Error duplicating the socket");
		case -76:
			return strcpy(msg, "\n-> Error creating a mutex");
		case -77:
			return strcpy(msg, "\n-> Error initializing process");
		case -78:
			return strcpy(msg, "\n-> Create file error");
		case -79:
			return strcpy(msg, "\n-> Bad config file");
	}
	return NULL;
}

/* Function: printlog
*  Print on daemon and windows both.
*/
void printlog(char* format, int n, char* s) {
	
	if (s == NULL) printf(format, n);
	else printf(format, s);
    
    #ifdef __linux__
		
		FILE* logFile = fopen(SYSLOG_PATH, "a+b");
		char* log;
		if (s == NULL) {
			//syslog(LOG_INFO, format, n);
			log = (char*) malloc(strlen(format) + 12);
			if (log == NULL) {
				fclose(logFile);
				return;
			}
			sprintf(log, format, n);
		} else {
			//syslog(LOG_INFO, format, s);
			log = (char*) malloc(strlen(format) + strlen(s) + 1);
			if (log == NULL) {
				fclose(logFile);
				return;
			}
			sprintf(log, format, s);
		}

		fwrite(log, 1, strlen(log), logFile);
		fclose(logFile);
		free(log);
    
    #endif
}

/* Function: throwError
*  Throw an error code.
*/
void throwError(int err) {

	char* msg = errorCode(err);
	if (msg != NULL) {
		#ifndef __linux__
			writeErr(msg, GetLastError());
		#else
			writeErr(msg, errno);
		#endif
	}
}