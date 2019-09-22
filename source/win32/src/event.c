#include <stdio.h>
#include <windows.h>
#include "../include/event.h"
#include "../../common/include/error.h"

/* Global variables */
HANDLE hLoggerEventW = NULL;
HANDLE hLoggerEventR = NULL;
HANDLE hListenerEventW = NULL;
HANDLE hListenerEventR = NULL;

/* Function: initEvents
*  Initialize events.
*/
int initEvents() {
	
	hLoggerEventW = createEvent("LOGGER_EVENT_W", TRUE);
	hLoggerEventR = createEvent("LOGGER_EVENT_R", FALSE);
	hListenerEventW = createEvent("LISTENER_EVENT_W", FALSE);
	hListenerEventR = createEvent("LISTENER_EVENT_R", FALSE);
	if (hLoggerEventW == NULL || hLoggerEventW == NULL || hLoggerEventW == NULL || hLoggerEventW == NULL) return CREATE_EVENT;
	return 0;
}

/* Function: setEvent
*  Set the given event (by name).
*/
int setEvent(HANDLE hEvent) {
	
	BOOL succ = SetEvent(hEvent);
	if(!succ) return SET_EVENT;
	return 0;
}

/* Function: waitEvent
*  Wait for a given event (by name).
*/
void waitEvent(HANDLE hEvent, DWORD timeout) {

	DWORD tout = (timeout != -1) ? timeout : INFINITE;
	WaitForSingleObject(hEvent, tout);
}

/* Function: createEvent
*  Create a new event.
*/
HANDLE createEvent(char* name, BOOL pulse) {
	
	SECURITY_ATTRIBUTES eventSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    HANDLE hEvent = CreateEvent(&eventSA, FALSE, pulse, name); 
	return hEvent;
}

/* Function: openEvent
*  Opena an event by name.
*/
HANDLE openEvent(char* name) {
	
	return OpenEvent(EVENT_MODIFY_STATE, FALSE, name);
}

/* Function: destroyEvents
*  Destroy all events.
*/
void destroyEvents() {
	
	CloseHandle(hListenerEventW);
	CloseHandle(hListenerEventR);
	CloseHandle(hLoggerEventW);
	CloseHandle(hLoggerEventR);
}