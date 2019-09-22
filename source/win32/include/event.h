#ifndef EVENT_H_
	
	#define EVENT_H_
	#include <windows.h>
	
	/* Global variables */
	extern HANDLE hLoggerEventW;
	extern HANDLE hLoggerEventR;
	extern HANDLE hListenerEventW;
	extern HANDLE hListenerEventR;

	int initEvents();
	int setEvent(HANDLE);
	void waitEvent(HANDLE, DWORD);
	HANDLE createEvent(char*, BOOL);
	HANDLE openEvent(char* name);
	void destroyEvents();

#endif