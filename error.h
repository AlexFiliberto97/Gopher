#ifndef ERROR_H_
	
	#define ERROR_H_
	
	#include "error.c"
	
	#define END_ERROR -10
	#define WSA_ERROR -11
	#define SOCK_ERROR -12
	#define BIND_ERROR -13
	#define LISTEN_ERROR -14
	#define SELECT_ERROR  -15
	#define ACCEPT_ERROR -16
	#define ALLOC_ERROR -17
	#define CONSOLE_HANDLER -18
	#define BAD_PORT -19
	#define BAD_ROOT -20
	#define BAD_PMODE -21
	#define SERVER_ERROR_H -22
	#define SERVER_ERROR_R -23
	#define SERVER_ERROR_S -24
	#define GRAB_SIZE -25
	#define NOT_FOUND -26
	#define INVALID_HANDLE -27
	#define FILE_SIZE_ERROR -28
	#define FOLDER_NOT_FOUND -29
	#define FILE_NOT_FOUND -30
	#define WRITE_FILE_ERROR -31
	#define THREAD_UNAVAILABLE -32
	#define THREAD_ERROR -33
	#define PIPE_UNAVAILABLE -34
	#define PIPE_INDEX -35
	#define PIPE_ERROR -36
	#define PIPE_NOT_FOUND -37
	#define WRITE_PIPE -38
	#define READ_PIPE -39
	#define CREATE_MAPPING -40 // mapping e shared memory
	#define OPEN_MAPPING -41
	#define READ_MAPPING -42
	#define DELETE_MAPPING -43
	#define EVENT_UNAVAILABLE -44
	#define EVENT_INDEX -45
	#define CREATE_EVENT -46
	#define SET_EVENT -47
	#define PROCESS_UNAVAILABLE -48
	#define PROCESS_ERROR -49
	#define CLIENT_DATA -50
	#define RECV_ERROR -51
	#define SEND_ERROR -52
	#define LOCK_FILE -53
	#define UNLOCK_FILE -54
	#define READ_FILE_ERROR -55
	#define BAD_ADDRESS -56
	#define ENVIRONMENT_ERROR -57
	#define THREAD_JOIN_ERROR -58
	#define FOLDER_ERROR -59
	#define OPEN_FILE_ERROR -60
	#define FORK_ERROR -61
	#define WAITPID_ERROR -62
	#define CLOSE_FD_ERROR -63
	#define SIGEVENT_ERROR -64
	#define DAEMON_ERROR -65
	
	char* errorCode(int);
	void throwError(int, ...);

#endif