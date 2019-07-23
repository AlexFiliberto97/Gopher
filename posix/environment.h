#ifndef ENVIRONMENT_H_

	#define ENVIRONMENT_H_
	
	#include "environment.c"

	void no_daemon();
	void init_env();
	int start_env();
	int clean_env();
	int killLogger();
	void sighup_handler(int);
	void sigint_handler(int);
	int setSighupEvent();
	int setup_daemon();

#endif