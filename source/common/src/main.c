#include <stdlib.h>
#include <stdio.h>
#include "../include/utils.h"
#include "../include/server.h"
#include "../include/error.h"

#ifndef __linux__
	#include "../../win32/include/environment.h"
#else
    #include <unistd.h>
	#include "../../posix/include/environment.h"
#endif

/* Global variables */
extern int SERVER_ALIVE;

int main(int argc, char** argv) {

	int err = 0, reloadCounter = 0;
	
	err = init_env();
	if (err != 0) {
		throwError(err);
		return -1;
	}
	
	err = start_env();
	if (err != 0) {
		throwError(err);
		return -1;
	}

	err = serverInit();
	if (err != 0) {
		clean_env();
		throwError(err);
		return -1;
	}

	while (SERVER_ALIVE == 1) {

		err = serverStart(argc, argv, reloadCounter);
		if (err != 0) {
			clean_env();
			throwError(err);
			return -1;
		}

		err = serverService();
		if (err != 0) {
			clean_env();
			serverStop();
			throwError(err);
			return -1;
		}

		reloadCounter++;
	}
	clean_env();
	return 0;
}