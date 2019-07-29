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
#include "mapping.h"

int SERVER_ALIVE = 0;
int LOGGER_PID;
static int daemonize = 1;


void sighup_handler(int);
void sigint_handler(int);
int setSighupEvent();
int setSigintEvent();
int setup_daemon();
int killLogger();


void no_daemon() {
	daemonize = 0;
}


void init_env() {
    if (daemonize == 1 && setup_daemon() != 0) {
        printlog("ERROR: daemon\n", 0, NULL);
        exit(1);
    }
	SERVER_ALIVE = 1;
	initThread();
	initProcess();
}


int start_env(){

	int err;

	struct Pipe* loggerPipe = createLoggerPipe();

	if (loggerPipe->err != 0) {
        printlog("ERROR: createLoggerPipe", 0, NULL);
        // free_shared_memory(loggerPipe, sizeof(loggerPipe));
        return -1;
    }

	err = initLocking();

    if (err != 0) {
        printlog("ERROR: initLocking", 0, NULL);
        // free_shared_memory(loggerPipe, sizeof(loggerPipe));
        return -1;
    }

    initCache();

    LOGGER_PID = startProcess(logger, (void*) loggerPipe);
	if (LOGGER_PID < 0) {
        return LOGGER_PID;
    }

	//Creating the garbage collector for threads e processes
	err = startThread(threadCollector, NULL, 1);
    if (err < 0) return err;

	err = startThread(processCollector, NULL, 1);
    if (err < 0) return err;


	return 0;
}

int clean_env() {
    killLogger();
	destroySharedLock();
	freeServerOptions();
	stopProcessCollector();
	destroyProcess();
	stopThreadCollector();
	destroyThreads();
	destroyLoggerPipe();
	closelog();
	return 0;
}


int killLogger() {
    
    int err;
    char* msg = "TERMINATE_LOGGER";

    char* pipe_msg = (char*) malloc(strlen(msg) + 1);
     if (pipe_msg == NULL) return ALLOC_ERROR;

    strcpy(pipe_msg, msg);

    pthread_mutex_lock(shared_lock->mutex);

        while (*(shared_lock->full) == 1) pthread_cond_wait(shared_lock->cond1, shared_lock->mutex);

        err = writePipe(loggerPipe, pipe_msg);
        if (err != 0) {
            throwError(1, err);
        }
        
        *(shared_lock->full) = 1;
        pthread_cond_signal(shared_lock->cond2); 
        
    pthread_mutex_unlock(shared_lock->mutex);

    int status;
    waitpid(LOGGER_PID, &status, 0);
    free(pipe_msg);

    return 0;

}


// SIGNAL HANDLERS
void sighup_handler(int s) {
    printlog("Sighup detected", 0, NULL);
    printlog("Reloading server...", 0, NULL);
    serverStop();
}

void sigint_handler(int s) {
    printlog("Sigint detected", 0, NULL);
    printlog("Quitting server...", 0, NULL);
    SERVER_ALIVE = 0;
    serverStop();
}

int setSighupEvent() {
    int err;
    struct sigaction sighupHandler;
    sighupHandler.sa_handler = sighup_handler;
    err = sigemptyset(&sighupHandler.sa_mask);
    if (err != 0) return SIGEVENT_ERROR;
    sighupHandler.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sighupHandler, NULL);
    if (err != 0) return SIGEVENT_ERROR;
    return 0;
}

int setSigintEvent() {
    int err;
    struct sigaction sigintHandler;
    sigintHandler.sa_handler = sigint_handler;
    err = sigemptyset(&sigintHandler.sa_mask);
    if (err != 0) return SIGEVENT_ERROR;
    sigintHandler.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigintHandler, NULL);
    if (err != 0) return SIGEVENT_ERROR;
    return 0;
}

int setup_daemon() {

    int err;

    pid_t daem1 = fork();

    if (daem1 < 0) {
        printf("Could not start 1\n");
        return -1;
    } else if (daem1 > 0) {
        exit(0);
    }

    pid_t ss = setsid();
    if (ss < 0) return -1;

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    pid_t daem2 = fork();

    if (daem2 < 0) {
        printf("Could not start 2\n");
        return -1;
    } else if (daem2 > 0) {
        exit(0);
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        throwError(1, DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    err = setSighupEvent();
    if (err != 0) {
        throwError(1, SIGEVENT_ERROR);
        exit(SIGEVENT_ERROR);
    }

    err = setSigintEvent();
    if (err != 0) {
        throwError(1, SIGEVENT_ERROR);
        exit(SIGEVENT_ERROR);
    }

    // Set new file permissions
    umask(0);

    // Close all open file descriptors
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