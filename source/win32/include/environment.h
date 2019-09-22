#ifndef ENVIRONMENT_H_

	#define ENVIRONMENT_H_
	#include <windows.h>
	
	BOOL WINAPI CtrlHandler(DWORD);
	void consoleDecoration();
	int init_env();
	int start_env();
	void clean_env();

#endif