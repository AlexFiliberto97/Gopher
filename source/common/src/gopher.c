#include <stdio.h>
#include <stdlib.h>
#include "../include/gopher.h"
#include "../include/utils.h"
#include "../include/network.h"
#include "../include/error.h"
#include "../include/const.h"

#ifndef __linux__
	#include <windows.h>
	#include "../../win32/include/pipe.h"
	#include "../../win32/include/event.h"
	#include "../../win32/include/thread.h"
	#include "../../win32/include/utils_win32.h"
#else
	#include <unistd.h>
	#include <string.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <pthread.h>
	#include "../../posix/include/utils_posix.h"
	#include "../../posix/include/pipe.h"
	#include "../../posix/include/logger.h"
	#include "../../posix/include/thread.h"
	#include "../../posix/include/mutex.h"
	#include "../../posix/include/environment.h"
#endif

/* Function: freeHandlerDataStruct
*  Free HandlerData struct instance.
*/
void freeHandlerDataStruct(HandlerData* hd) {
	
	if (hd->process_mode == 0) {
		if(hd->cliData != NULL) free(hd->cliData);
		if(hd->address != NULL) free(hd->address);
		if(hd->abs_root_path != NULL) free(hd->abs_root_path);
		if(hd != NULL) free(hd);
	} else {
		#ifdef __linux__
			free_shared_memory(hd->cliData, strlen(hd->cliData) + 1);
			free_shared_memory(hd->address, strlen(hd->address) + 1);
			free_shared_memory(hd->abs_root_path, strlen(hd->abs_root_path) + 1);
			free_shared_memory(hd, sizeof(hd));
		#else
			if(hd->cliData != NULL) free(hd->cliData);
			if(hd->address != NULL) free(hd->address);
			if(hd->abs_root_path != NULL) free(hd->abs_root_path);
			if(hd != NULL) free(hd);
		#endif
	} 
}

/* Function: getGopherType
*  Return the gopher type for a given path.
*/
char getGopherType(char* path, char* rootPath, Dict extDict) {
	
	char* ext = getExtension(path, rootPath);
	if (ext == NULL) return '3';

	char* type = getAssocValue(ext, extDict);
	if (type == NULL) {
		free(ext);
		return '3';
	}

	free(ext);
	return type[0];
}

/* Function: getGopherType
*  Return _dispnames assoc for given path.
*/
char** getDispNamesAssoc(char* path, int* count) {
    
    char* fname = "_dispnames";
    char* dispNamesPath = (char*) malloc(strlen(path) +strlen(fname) +1);
    if (dispNamesPath == NULL) {
        throwError(ALLOC_ERROR);
        return NULL;
    }
    
    sprintf(dispNamesPath, "%s%s", path, fname);
    if (existsFile(dispNamesPath) != 0) {
        free(dispNamesPath);
        return NULL;
    }

    int n;
    char** list = readlines(dispNamesPath, &n);
    if (list == NULL) {
        free(dispNamesPath);
        return NULL;
    }

    free(dispNamesPath);
    *count = n;
    return list;
}

/* Function: getItem
*  Return the gopher item for a given path.
*/
char* getItem(char* item, int* type) {

	if (existsDir(item) == 0) {
		if (item[strlen(item)-1] != '/') {
			char* tmpItem = (char*) malloc(strlen(item) + 2);
			if (tmpItem == NULL) {
				*type = -1;
				throwError(ALLOC_ERROR);
				return NULL;
			}
			sprintf(tmpItem, "%s/", item);
			item = (char*) realloc(item, strlen(tmpItem) + 1);
			if (item == NULL) {
				free(tmpItem);
				*type = -1;
				throwError(ALLOC_ERROR);
				return NULL;
			}

			strcpy(item, tmpItem);
			free(tmpItem);
		} 
		*type = 1;
	} else if (existsFile(item) == 0) {
		*type = 0;
	} else {
		*type = -1;
	}
	return item;
}

/* Function: gopherListDir
*  List the content of a given directory (gopher standard).
*/
char** gopherListDir(char* path, char* req, int* n, HandlerData* hd) {

	char* request = (char*) malloc(strlen(req) + 3);	
	if (request == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	sprintf(request, "/%s/", req);
	request = fixPath(request);
	if (request == NULL) return NULL;

	int filesCount = 0;
	char** filesList = listDir(path, &filesCount);
	if (filesList == NULL || filesCount == 0) {
		free(request);
		return NULL;
	}

	int extCount;
	char** extAssocList = readlines(GOPHER_EXTENSIONS_FILE, &extCount);
	if (extAssocList == NULL) {
		free(request);
		freeList(filesList, filesCount);
		return NULL;
	}

	Dict extDict = buildDict(extAssocList, extCount);
	if (extDict.err != 0) {
		free(request);
		freeList(filesList, filesCount);
		freeList(extAssocList, extCount);
		return NULL;
	}

	freeList(extAssocList, extCount);
	int useAssoc = 0, assocCount = 0;
	Dict assocDict;
	char** assoc = getDispNamesAssoc(path, &assocCount);
	if (assoc != NULL) {
		assocDict = buildDict(assoc, assocCount);
		if (assocDict.err == 0) useAssoc = 1;
		freeList(assoc, assocCount);
	}

	char** gophList = (char**) malloc(sizeof(char*) * filesCount);
	if (gophList == NULL) {
		free(request);
		freeList(filesList, filesCount);
		freeDict(extDict);
		freeDict(assocDict);
		throwError(ALLOC_ERROR);
		return NULL;
	}

	char type;
	char* dispName;
	char* gopherPath;

	for (int i = 0; i < filesCount; i++) {

		gopherPath = (char*) malloc(strlen(request) + strlen(filesList[i]) + 1);
		if (gopherPath == NULL) {
			free(request);
			freeList(filesList, filesCount);
			freeDict(extDict);
			freeDict(assocDict);
			throwError(ALLOC_ERROR);
			return NULL;
		}
		
		sprintf(gopherPath, "%s%s", request, filesList[i]);
		type = getGopherType(gopherPath, hd->abs_root_path, extDict);
		if (type == -1){
			free(gopherPath);
			continue;
		}

		if (useAssoc == 1) {
			dispName = getAssocValue(filesList[i], assocDict);
			if (dispName == NULL) {
				dispName = filesList[i];
			}
		} else {
			dispName = filesList[i];
		}
		
		char* gophString = (char*) malloc(strlen(dispName) + strlen(gopherPath) + strlen(hd->address) + PORTSTR + TAB + 1);
		if (gophString == NULL) {
			free(request);
			freeList(filesList, filesCount);
			freeDict(extDict);
			freeDict(assocDict);
			free(gopherPath);
			free(dispName);
			throwError(ALLOC_ERROR);
			return NULL;
		}

		sprintf(gophString, "%c%s\t%s\t%s\t%d", type, dispName, gopherPath, hd->address, hd->port);
		gophList[i] = gophString;
		free(gopherPath);
	}

	*n = filesCount;
	free(request);
	freeList(filesList, filesCount);
	freeDict(extDict);
	if (useAssoc == 1) freeDict(assocDict);
	return gophList;
}

/* Function: handleRequest
*  Handle the request.
*/
int handleRequest(char* request, SendFileData* sfd, HandlerData* hd) {

	if (isValidPath(request) == 0) {
		sfd->response = cpyalloc(ACCESS_DENIED_MSG);
		if (sfd->response == NULL) return -1;
		sfd->size = strlen(sfd->response);
		return -1;
	}
	
	char* item = (char*) malloc(strlen(hd->abs_root_path) + strlen(request) + 1);
	if (item == NULL) {
		sfd->response = cpyalloc(SERVER_ERROR_MSG);
		if (sfd->response == NULL) return -1;
		sfd->size = strlen(sfd->response) + 1;
		return -1;
	}

	sprintf(item, "%s%s", hd->abs_root_path, request);

	item = fixPath(item);
	if (item == NULL) {
		sfd->response = cpyalloc(SERVER_ERROR_MSG);
		if (sfd->response == NULL) return -1;
		sfd->size = strlen(sfd->response) + 1;
		return -1;
	}

	int type = -1;
	
	item = getItem(item, &type);
	printlog("Request: %s\n", 0, item);

	if (type == 0) {
		sfd->file = cpyalloc(item);
		if (sfd->file == NULL) type = -1;
	
	} else if (type == 1) {

		int count;
		char** list = gopherListDir(item, request, &count, hd);
		if (list == NULL) {
			sfd->response = cpyalloc(EMPTY_FOLDER_MSG);
			if (sfd->response == NULL) type = -1;
		} else {
			sfd->response = concatList(list, count, '\n');
			if (sfd->response == NULL) {
				sfd->response = cpyalloc(SERVER_ERROR_MSG);
				if (sfd->response == NULL) type = -1;
			}
			freeList(list, count);
		}
		sfd->size = strlen(sfd->response);

	} else if (type == -1) {
		sfd->response = cpyalloc(FILE_NOT_FOUND_MSG);
		if (sfd->response == NULL) {
			free(item);
			return -1;
		}
		sfd->size = strlen(sfd->response);
	}
	
	free(item);
	return type;
}

/* Function: sendResponse
*  Start a thread to send the response.
*/
int responseThread(SendFileData* sfd) {
	
	#ifndef __linux__
		
		HANDLE id = startResponseThread(sendResponse, (void*) sfd);
		if (((int) id) == 0) {
			sfd->err = -1;
			return SEND_FILE_ERROR;
		}
		joinThread(id);
	
	#else
		
		pthread_t id = startResponseThread(sendResponse, (void*) sfd);
		if (((int) id) == THREAD_ERROR) {
			sfd->err = -1;
			return SEND_FILE_ERROR;
		}
		joinThread(id);
	
	#endif
	return 0;
}

/* Function: sendResponse
*  Send response to client.
*/
void* sendResponse(void* input) {
	
	SendFileData* sfd = (SendFileData*) input;
	if (sfd->file != NULL) {
		sfd->err = sendFile(sfd);
	} else {
		sfd->err = sendAll(sfd->sock, sfd->response, sfd->size);
	}
	return NULL;
}

/* Function: killSocket
*  Kill a given socket and close connection.
*/
void killSocket(int sock) {
	
	#ifndef __linux__
		
		shutdown(sock, SD_BOTH);
		closesocket(sock);
	
	#else
		
		shutdown(sock, SHUT_RDWR);
		close(sock);
	
	#endif
}

/* Function: handler
*  Handle the request.
*/
int handler(void* input) {

	int err = 0;
	HandlerData* hd = (HandlerData*) input;
    printlog("\n\nI'm handling the request on sock: %d\n", hd->sock, NULL);
    size_t msgLen;
    char* msg = recvAll(hd->sock, &msgLen);
	if (msg == NULL) {
		killSocket(hd->sock);
		freeHandlerDataStruct(hd);
		return RECV_ERROR;
	} 

    msg[msgLen -2] = '\0'; //stops with \r\n

	char* request = cpyalloc(msg);
	if (request == NULL) {
		free(msg);
		killSocket(hd->sock);
		freeHandlerDataStruct(hd);
		return ALLOC_ERROR;
	}

	SendFileData sfd = {hd->sock, NULL, NULL, 0, hd->process_mode, 0};
    int responseType = handleRequest(request, &sfd, hd);

	if (responseType == 0 && sfd.file != NULL) {
		err = responseThread(&sfd);
		if (err != 0) {
			throwError(err);
			free(msg);
			free(request);
			killSocket(hd->sock);
			freeHandlerDataStruct(hd);
			if (sfd.file != NULL) free(sfd.file);
			if (sfd.response != NULL) free(sfd.response);
			return SERVER_ERROR_H;
		}

		if (sfd.err == 0) {
			char fileSz[LLISTR];
			sprintf(fileSz, "%lld", sfd.size);
			char* pipeMsg = (char*) malloc(strlen(hd->cliData) + strlen(hd->abs_root_path) + strlen(msg) + strlen(fileSz) + PIPE_MSG_PADDING + 1);
			if (pipeMsg == NULL) {
				throwError(ALLOC_ERROR);
				free(msg);
				free(request);
				killSocket(hd->sock);
				freeHandlerDataStruct(hd);
				if (sfd.response != NULL) free(sfd.response);
				return PIPE_ERROR;
			}

			sprintf(pipeMsg, "%s %s%s %s byte\n", hd->cliData, hd->abs_root_path, msg, fileSz);

			#ifndef __linux__
				
				HANDLE hEventW = openEvent("LOGGER_EVENT_W");
				HANDLE hEventR = openEvent("LOGGER_EVENT_R");

				waitEvent(hEventW, -1);

				err = writePipe(pipeMsg);
				if (err != 0) throwError(err);

				if (setEvent(hEventR) != 0) throwError(err);

				CloseHandle(hEventW);
				CloseHandle(hEventR);
			
			#else
				
				mutexLock(mcvLogger->mutex);
					while (mcvLogger->full == 1) pthread_cond_wait(mcvLogger->cond1, mcvLogger->mutex);
					
					err = writePipe(loggerPipe, pipeMsg);
					if (err != 0) throwError(err);
					
					mcvLogger->full = 1;
					pthread_cond_signal(mcvLogger->cond2); 	
				mutexUnlock(mcvLogger->mutex);
			
			#endif
			free(pipeMsg);
		}
	} else {
		sendResponse((void*) &sfd);
	}

	if (sfd.err != 0) throwError(sfd.err);
	
	killSocket(hd->sock);
	free(msg);
	free(request);
	if (sfd.file != NULL) free(sfd.file);
	if (sfd.response != NULL) free(sfd.response);
	printlog("I handled the request: %d\n", hd->sock, NULL);
	freeHandlerDataStruct(hd);
	return 0;
}