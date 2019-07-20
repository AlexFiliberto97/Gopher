#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "server.h"
#include "error.h"


#ifndef __linux__
	#include "win32/environment.h"
#else
    #include <unistd.h>
	#include "posix/environment.h"
    #include <pthread.h>
    #include "posix/thread.h"
#endif


// void* testfunction(void* input) {
//     sleep(10);
//     sigint_handler(0);
//     sleep(1);
// }


int main(int argc, char** argv) {

	int err;

    // pthread_t th = startThread(testfunction, NULL, 0);

    no_daemon(); 
    
	init_env();
	
	err = start_env();
	if (err != 0) {
		throwError(2, err, -1);
		return -1;
	}
	

	while (SERVER_ALIVE == 1) {

        err = serverInit(argc, argv);
    	if (err != 0) {
    		clean_env();
    		throwError(2, err, -1);
    		return -1;
    	}

		err = serverStart();
		if (err != 0) {
			clean_env();
			throwError(2, err, -1);
			return -1;
		}
		
		err = serverService();
		if (err != 0) {
			clean_env();
			serverStop();
			throwError(2, err, -1);
			return -1;
		}
		
	}

	clean_env();

    // joinCollect(th);

	return 0;

}