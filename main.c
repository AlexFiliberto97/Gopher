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


#ifdef __linux__

    void sighup_handler(int s) {
        log_output("Sighup detected", 0);
        log_output("Reloading server...", 0);
        serverStop();
        // serverReload();
    }

    void sigint_handler(int s) {
        log_output("Sigint detected", 0);
        log_output("Quitting server...", 0);
        SERVER_ALIVE = 0;
        serverStop();
    }

    int setSighupEvent() {
        int err;
        struct sigaction sighupHandler;
        sighupHandler.sa_handler = sighup_handler;
        err = sigemptyset(&sighupHandler.sa_mask);
        if (err != 0) return -1;
        sighupHandler.sa_flags = SA_RESTART;
        sigaction(SIGHUP, &sighupHandler, NULL);
        if (err != 0) return -1;
        return 0;
    }

    int setSigintEvent() {
        int err;
        struct sigaction sigintHandler;
        sigintHandler.sa_handler = sigint_handler;
        err = sigemptyset(&sigintHandler.sa_mask);
        if (err != 0) return -1;
        sigintHandler.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sigintHandler, NULL);
        if (err != 0) return -1;
        return 0;
    }

    int daemon_skeleton() {

        pid_t daem1 = fork();

        if (daem1 < 0) {
            printf("Could not start 1\n");
            return -1;
        } else if (daem1 > 0) {
            exit(0);
        }

        pid_t ss = setsid();
        if (ss < 0) return -1;

        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGINT, SIG_IGN);

        pid_t daem2 = fork();

        if (daem2 < 0) {
            printf("Could not start 2\n");
            return -1;
        } else if (daem2 > 0) {
            exit(0);
        }

        signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGINT, SIG_IGN);

        int err;

        err = setSighupEvent();
        if (err != 0) {
            printf("\nERROR: could not set control handler\n");
            return -1;
        }
        err = setSigintEvent();
        if (err != 0) {
            printf("\nERROR: could not set control handler\n");
            return -1;
        }

        // Set new file permissions
        umask(0);

        // Change the working directory
        // chdir("/home/giorgio/Programming/SoIIProject");
        // chdir("/");

        // Close all open file descriptors
        int x;
        for (int x = 0; x < sysconf(_SC_OPEN_MAX); x++) {
            close(x);
        }

        for (int fd = 0; fd < 3; fd++) {
            int nfd = open("/dev/null", O_RDWR);
            if (nfd < 0 || nfd == fd) continue;
            dup2(nfd, fd);
            if (nfd > 2) close(nfd);
        }

        // Open the log file
        openlog("gopher-server", LOG_PID | LOG_CONS, LOG_USER);
        syslog(LOG_INFO, "%s", "-----------------------");

        return 0;

    }

#endif


void* testfunction(void* input) {
    sleep(10);
    sigint_handler(0);
    sleep(1);
}


int main(int argc, char** argv) {

	int err;

	#ifndef __linux__
		SetCurrentDirectory("C:");
   	#else
        err = daemon_skeleton();

        if (err != 0) {
            log_output("ERROR: daemon\n", 0);
            exit(1);
        }
    #endif

    // pthread_t th = startThread(testfunction, NULL, 0);

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

	#ifdef __linux__
        closelog();
    #endif

	clean_env();

    // joinCollect(th);

	return 0;

}