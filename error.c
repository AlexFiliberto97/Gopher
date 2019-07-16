#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "utils.h"

char* getMessage(int err) {

	/*
	switch(err) {
		case -10:
			return "malloc realloc problem ";
		case -11:
			return "Server can't start";
		case -12:
			return "Server can't handle the client request";
		case -102:
			return "The specified process mode is not valid.";
		case -103:
			return "The specified root path is not valid.";
		case -104:
			return "Generic socket error. Server can't start.";
		case -105:
			return "Bind function failed. Server can't start.";
		case -106:
			return "Listen function failed. Server can't start.";
		case -107:
			return "Select function failed. Server stopped.";
		case -108:
			return "Accept function failed. Request can't be handled.";
		default:
			return NULL;

	}
	*/
	return NULL;
}


int errorCode(int num, ...){

	va_list valist;
   	va_start(valist, num);
   	char errCode[11];
   	errCode[0] = '\0';
   	int index = 0, errNum = num;

   	for (int i = 0; i < num; i++) {

   		char temp[11], tcode[11];
   		temp[0] = '\0';
   		sprintf(temp, "%d", -va_arg(valist, int));
   		if(strlen(temp) > 2) {
   			for(int i = 1; i < 11; i++) tcode[i] = temp[i];
   			sprintf(errCode, "%s%s", errCode, tcode);
   			index += strlen(temp);
   			tcode[0] = temp[0];
   			tcode[1] = '\0';
   			errNum += atoi(tcode);
   		} else {
   			sprintf(errCode, "%s%s", errCode, temp);
   			index+=2;
   		}

   		errCode[index] = '\0';
   	}
   	va_end(valist);
   	sprintf(errCode, "%s%d", errCode, errNum);
   	return -atoi(errCode);
}

void throwError(int errCode){

	printf("error: %d\n", errCode);
}