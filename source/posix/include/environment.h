#ifndef ENVIRONMENT_H_

	#define ENVIRONMENT_H_
	#include "mutex.h"
	
	/* Global variables */
	extern int SERVER_ALIVE, LOGGER_PID, daemonize;

	void no_daemon();
	int init_env();
	int start_env();
	void killLogger();
	void sighup_handler(int);
	void sigint_handler(int);
	int setSighupEvent();
	int setSigintEvent();
	int setup_daemon();
	void clean_env();

#endif