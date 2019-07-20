#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h> 
#include <signal.h>
#include "pipe.h"
#include "thread.h"
#include "process.h"
#include "logger.h"


int SERVER_ALIVE = 0;
int LOGGER_PID;
static int daemonize = 1;


void sighup_handler(int s);
void sigint_handler(int s);
int setSighupEvent();
int setSigintEvent();
int daemon_skeleton();


void no_daemon() {
	daemonize = 0;
}


void init_env() {
    if (daemonize == 1 && daemon_skeleton() != 0) {
        log_output("ERROR: daemon\n", 0);
        exit(1);
    }
	SERVER_ALIVE = 1;
	// initLoggerPipe();
	initThread();
	initProcess();
}


int start_env(){

	int err;

	struct Pipe* loggerPipe = createLoggerPipe();

	if (loggerPipe->err != 0) return -1;

	err = initLocking();

	LOGGER_PID = startProcess(logger, 1, (void*) loggerPipe);
	if (LOGGER_PID < 0) return -1;

	//Creating the garbage collector for threads e processes
	startThread(threadCollector, NULL, 1);
	startThread(processCollector, NULL, 1);

	return 0;
}

int clean_env() {
	kill(LOGGER_PID, SIGKILL);
	destroySharedLock();
	freeServerOptions();
	stopProcessCollector();
	destroyProcess();
	stopThreadCollector();
	destroyThreads();
	// destroyPipes();
	destroyLoggerPipe();
	closelog();
	return 0;
}


// SIGNAL HANDLERS
void sighup_handler(int s) {
    log_output("Sighup detected", 0);
    log_output("Reloading server...", 0);
    serverStop();
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