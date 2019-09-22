#ifndef CONST_H

	#define CONST_H
	
	#define FILE_NOT_FOUND_MSG "\n>>> Error: file not found\n"
	#define EMPTY_FOLDER_MSG "\n>>> The requested folder is empty\n"
	#define ACCESS_DENIED_MSG "\n>>> Error: access denied\n"
	#define SERVER_ERROR_MSG "\n>>> Error: internal server error\n"
	
	#define GOPHER_EXTENSIONS_FILE "extensions.txt"
	#define CONFIG_FILE_PATH "config.txt"
	#define LOG_FILE_PATH "logs/log.txt"
	#define SYSLOG_PATH "logs/syslog.txt"
	#define ERROR_LOG_PATH "logs/error.txt"

	#define INTSTR 11				//Int to string max length (to string)
	#define LLISTR 20				//64-bit integer max length (to string)
	#define MAX_THREADS 1024
	#define MAX_PROCESS 1024
	#define MUTEX_TIMEOUT 10
	#define MAP_VIEW_SIZE 2097152	//Cache page size (2 MB) 
	#define CACHE_SIZE 128			//Number of pages for cache
	#define MAX_ERROR_LENGTH 73		//Max length for an error message
	#define PORTSTR 6				//Port to string max length (max value: 65536)
	#define TAB 5					//Tab length (\t char)
	#define PIPE_MSG_PADDING 8 		//longht of: 3 spaces + the string 'byte' +\n char
	#define LISTNR_ARGC 5 			//Number of cmd args for listener process
	#define LOGGER_ARGC 1 			//Number of cmd args for logger process
	#define N_SOCKETS 128        	//Max number of possible sockets
	#define SENDBUF_SIZE 1024		//Send buffer size
	#define RECVBUF_SIZE 64			//Receive buffer size
	#define MIN_IP_LEN 7			
	#define MAX_IP_LEN 15			
	#define MAX_CONFIG_FILE_SZ 256
	#define TIMEOUT 3000
	#define ERR_BOUND 10

#endif