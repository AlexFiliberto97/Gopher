#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <sys/file.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include "../include/environment.h"
#include "../include/pipe.h"
#include "../include/mutex.h"
#include "../include/thread.h"
#include "../include/process.h"
#include "../include/logger.h"
#include "../include/caching.h"
#include "../../common/include/error.h"
#include "../../common/include/socket.h"
#include "../../common/include/server.h"

/* Global variables */
int SERVER_ALIVE = 0;
int LOGGER_PID = -1;
int daemonize = 1;

/* Function: no_daemon
*  Start with no daemon on posix.

void no_daemon() {
	
    daemonize = 0;
}
*/

/* Function: init_env
*  Initialize the environment for posix.
*/
int init_env() {

    if (daemonize == 1 && setup_daemon() != 0) return -1;

    SERVER_ALIVE = 1;
    int err = initCache();
    if (err != 0) return err;

    err = initServerOpts();
    if (err != 0) return err;

    err = initThread();
    if (err != 0) return err;

    err = initProcess();
    if (err != 0) return err;

    return 0;
}

/* Function: start_env
*  Start the environment for posix.
*/
int start_env(){

    mcvLogger = createMutexCV();
    if (mcvLogger == NULL) return -1;

	Pipe* loggerPipe = createLoggerPipe();
	if (loggerPipe->err != 0) {
        throwError(PIPE_ERROR);
        return -1;
    }
    
    LOGGER_PID = startProcess(logger, (void*) loggerPipe, -1);
	if (LOGGER_PID < 0) {
        return LOGGER_PID;
    }

	int err = startThread(threadCollector, NULL, -1);
    if (err < 0) return err;

	err = startThread(processCollector, NULL, -1);
    if (err < 0) return err;

	return 0;
}

/* Function: killLogger
*  Kill the logger process.
*/
void killLogger() {
    
    int err = 0;
    char pipe_msg[17] = "TERMINATE_LOGGER";
    
    mutexLock(mcvLogger->mutex);

        while (mcvLogger->full == 1) pthread_cond_wait(mcvLogger->cond1, mcvLogger->mutex);

        err = writePipe(loggerPipe, pipe_msg);
        if (err != 0) {
            throwError(err);
            kill(LOGGER_PID, SIGKILL);
        }
        
        mcvLogger->full = 1;
        pthread_cond_signal(mcvLogger->cond2); 
        
    mutexUnlock(mcvLogger->mutex);

    int status;
    if (err == 0) waitpid(LOGGER_PID, &status, 0);
}

/* Function: sighup_handler
*  Handle SIGHUP.
*/
void sighup_handler(int s) {
    
    printlog("Reloading server...\n", 0, NULL);
    serverStop();
}

/* Function: sigint_handler
*  Handle SIGINT.
*/
void sigint_handler(int s) {

    printlog("Quitting server...\n", 0, NULL);
    SERVER_ALIVE = 0;
    serverStop();
}

/* Function: setSighupEvent
*  Set SIGHUP event.
*/
int setSighupEvent() {
   
    struct sigaction sighupHandler;
    sighupHandler.sa_handler = sighup_handler;
    int err = sigemptyset(&sighupHandler.sa_mask);
    if (err != 0) return SIGEVENT_ERROR;

    sighupHandler.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sighupHandler, NULL);
    if (err != 0) return SIGEVENT_ERROR;
    
    return 0;
}

/* Function: setSigintEvent
*  Set SIGINT event.
*/
int setSigintEvent() {
    
    struct sigaction sigintHandler;
    sigintHandler.sa_handler = sigint_handler;
    int err = sigemptyset(&sigintHandler.sa_mask);
    if (err != 0) return SIGEVENT_ERROR;

    sigintHandler.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigintHandler, NULL);
    if (err != 0) return SIGEVENT_ERROR;

    return 0;
}

/* Function: setup_daemon
*  Setup the daemon.
*/
int setup_daemon() {

    pid_t daem1 = fork();
    if (daem1 < 0) {
        return -1;
    } else if (daem1 > 0) {
        exit(0);
    }

    pid_t sid = setsid();
    if (sid < 0) return -1;

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    pid_t daem2 = fork();
    if (daem2 < 0) {
        return -1;
    } else if (daem2 > 0) {
        exit(0);
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        throwError(DAEMON_ERROR);
        exit(DAEMON_ERROR);
    }

    int err = setSighupEvent();
    if (err != 0) {
        throwError(SIGEVENT_ERROR);
        exit(SIGEVENT_ERROR);
    }

    err = setSigintEvent();
    if (err != 0) {
        throwError(SIGEVENT_ERROR);
        exit(SIGEVENT_ERROR);
    }

    umask(0);

    for (int x = 0; x < sysconf(_SC_OPEN_MAX); x++) {
        close(x);
    }

    for (int fd = 0; fd < 3; fd++) {
        int nfd = open("/dev/null", O_RDWR);
        if (nfd < 0 || nfd == fd) continue;
        dup2(nfd, fd);
        if (nfd > 2) close(nfd);
    }

    //openlog("gopher-server", LOG_PID | LOG_CONS, LOG_USER);
    //syslog(LOG_INFO, "%s", "-----------------------");
    return 0;
}

/* Function: clean_env
*  Clean the environment.
*/
void clean_env() {

    stopThreadCollector();
    stopProcessCollector();
    serverStop();
    destroyServerOpts();
    destroyThreads();
    destroyCache();
    destroyProcess();
    killLogger();
    destroyMutexCV(mcvLogger);
    destroyLoggerPipe();
    //if (daemonize == 1) closelog();
}