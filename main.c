#include <stdlib.h>
#include <stdio.h>
#include "server.h"
#include "utils.h"
//#include "error.h"
#include <stdarg.h>

#ifndef __linux__
	#include "win32/environment.h"
#else
	#include "posix/environment.h"
#endif

int main(int argc, char** argv) {

	init_env();	
	int err = start_env();
	if (err != 0){
		throwError(err);
		return -1;
	}

	err = serverInit(argc, argv);
	if (err != 0){
		throwError(err);
		return -1;
	}

	while (SERVER_ALIVE == 1) {
		
		err = serverStart();
		if (err != 0) {
			throwError(err);
			return -1;
		}
		
		err = serverService();
		if (err != 0) {
			throwError(err);
			return -1;
		}
	}
	clean_env();
	return 0;
}