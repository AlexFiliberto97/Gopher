#include <windows.h>
#include <stdio.h>
#include "../error.h"

#define MAX_EVENTS 10

struct Event {
	
	char* name;
	HANDLE hEvent;
};

struct Event Events[MAX_EVENTS];

void initEvents() {
	
	for (int i = 0; i < MAX_EVENTS; i++) {
		Events[i].name = NULL;
		Events[i].hEvent = NULL;
	}
}

DWORD newEventIndex() {
	
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (Events[i].hEvent == NULL) {
			return i;
		}
	}
	return EVENT_INDEX;
}	

HANDLE eventHandler(char* name) {
	
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (strcmp(Events[i].name, name) == 0) {
			return Events[i].hEvent;
		}
	}
	return NULL;
}

BOOL setEvent(char* name) {

	HANDLE event = eventHandler(name);
	if(event == NULL) return EVENT_UNAVAILABLE;

	BOOL succ = SetEvent(event);
	if(!succ) return SET_EVENT;
	return 0;
}

int createEvent(char* name, BOOL pulse) {

	DWORD index = newEventIndex();
	if (index < 0) return index;
	
	SECURITY_ATTRIBUTES eventSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    HANDLE event = CreateEvent(&eventSA, FALSE, pulse, NULL); 
    if (event == NULL) return CREATE_EVENT;

    Events[index].name = (char*) malloc(strlen(name));
    if (Events[index].name == NULL) return ALLOC_ERROR;

    strcpy(Events[index].name, name);
   	Events[index].hEvent = event;
   	return 0;
}

int addEvent(char* name, HANDLE hEvent) {

	DWORD index = newEventIndex();
	if (index < 0) return index;

	Events[index].name = (char*) malloc(strlen(name));
	if (Events[index].name == NULL) return ALLOC_ERROR;
    
    strcpy(Events[index].name, name);
	Events[index].hEvent = hEvent;
	return 0;
}

void waitEvent(char* name) {
	
	HANDLE h = eventHandler(name);
	if (h == NULL) {
		return;
	} else {
		WaitForSingleObject(h, INFINITE);
	}
}

void destroyEvents() { //Check this
	
	for(int i = 0; i < MAX_EVENTS; i++) {
		if(Events[i].name != NULL) {
			free(Events[i].name);
			CloseHandle(Events[i].hEvent);
		}
	}
}