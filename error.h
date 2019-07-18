#ifndef ERROR_H_
	
	#define ERROR_H_
	
	#include "error.c"

	#define GENERIC_ERROR -10
	#define ALLOC_ERROR -11

	#define CONSOLE_HANDLER -88

	#define BAD_PORT -227
	#define BAD_ROOT -177
	#define BAD_PMODE -828
	
	#define WSA_ERROR -827
	#define SOCK_ERROR -22
	#define BIND_ERROR -72
	#define LISTEN_ERROR -828
	
	#define SERVER_ERROR_H -828
	#define SELECT_ERROR  -28
	#define ACCEPT_ERROR -832
	#define SERVER_ERROR_R -22   //SERVER reload
	#define SERVER_ERROR_S -222 //SERVER can't start
	
	#define GRAB_SIZE -282
	#define NOT_FOUND -555
	#define INVALID_HANDLE -83
	#define FILE_SIZE -77  //Cant grab file size
	#define FOLDER_NOT_FOUND -111
	#define FILE_NOT_FOUND -3333
	#define WRITE_FILE_ERROR -2782
	#define SEND_ERROR -999

	#define THREAD_UNAVAILABLE -111
	#define THREAD_ERROR -112

	#define PIPE_UNAVAILABLE -555
	#define PIPE_ERROR -112
	#define PIPE_NOT_FOUND -2889
	#define WRITE_PIPE -88 //Error writing pipe
	#define READ_PIPE -12

	#define CREATE_MAPPING -2928
	#define OPEN_MAPPING -22
	#define READ_MAPPING -2266
	#define DELETE_MAPPING -979
	
	#define EVENT_UNAVAILABLE -288
	#define CREATE_EVENT -30
	#define SET_EVENT -4892
	
	#define PROCESS_UNAVAILABLE -9090
	#define PROCESS_ERROR -38873

#endif