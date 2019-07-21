#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


void throwError(int num, ...) {

	va_list valist;
   	va_start(valist, num);
   	//char errCode[11];
   	//errCode[0] = '\0';
   	//int index = 0, errNum = num;

   	printlog("Error:", 0, NULL);

   	for (int i = 0; i < num; i++) {

   		printlog(" %d ", va_arg(valist, int), NULL);

         //printf("");
   	}

	printf("\n");
}


int errorCode(int num, ...) {

	
	return -1;
}