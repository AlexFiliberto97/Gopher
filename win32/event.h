#ifndef EVENT_H_
	
	#define EVENT_H_

	#include <windows.h>
	#include "event.c"
	
	void initEvents();
	DWORD newEventIndex();
	HANDLE eventHandler(char*);
	BOOL setEvent(char*);
	int createEvent(char*, BOOL);
	BOOL addEvent(char*, HANDLE);
	void waitEvent(char*);
	void destroyEvents();

#endif