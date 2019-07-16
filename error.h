#ifndef ERROR_H
	
	#define ERROR_H
	
	#include "error.c"
	
	//Error codes
	#define ALLOC_ERROR -10    //GENERIC ERROR could be malloc/calloc failure  or BAD_HANDLE
	#define SERVER_ERROR_S -11 //start
	#define SERVER_ERROR_H -12	//handle request
	#define SERVER_ERROR_R -13	//reload
	#define WSA_ERROR -14
	#define SOCK_ERROR -15
	#define BIND_ERROR -16
	#define LISTEN_ERROR -17
	#define SELECT_ERROR -18
	#define ACCEPT_ERROR -19
	#define BAD_PORT -20
	#define BAD_PMODE -21
	#define BAD_ROOT -22
	#define GRAB_SIZE -23  //error grabbing the file size (fseek or ftell)
	#define NOT_FOUND -24   //Element not found (in list of strings)
	#define BAD_HANDLE -25
	#define THREAD_ERROR -26
	#define THREAD_UNAVAILABLE -27
	#define PROCESS_ERROR -28
	#define PROCESS_UNAVAILABLE -29
	#define PIPE_ERROR -30
	#define PIPE_UNAVAILABLE -31
	#define PIPEW_ERROR -32
	#define EVENT_ERROR -33
	#define EVENT_UNAVAILABLE -34
	#define DELETE_MAPPING -35
	#define CONSOLE_HANDLER -36



	char* getMessage(int);
	void raiseError(int, ...);

#endif