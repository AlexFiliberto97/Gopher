#ifndef ENVIRONMENT_H_

	#define ENVIRONMENT_H_

	#include "environment.c"
	
	BOOL WINAPI CtrlHandler(DWORD);
	void consoleDecoration();
	void init_env();
	int start_env();
	int clean_env();

#endif