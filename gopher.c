#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "network.h"
#include "error.h"

#ifndef __linux__
	#include <windows.h>
	#include "win32/utils_win32.h"
	#include "win32/pipe.h"
	#include "win32/mapping.h"
	#include "win32/event.h"
	#include "win32/thread.h"
	#include "win32/process.h"
#else
	#include <unistd.h>
	#include <string.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <pthread.h>
	#include "posix/utils_posix.h"
	#include "posix/pipe.h"
	#include "posix/logger.h"
	#include "posix/mapping.h"
	#include "posix/thread.h"
	#include "posix/process.h"
#endif


#define GOPHER_EXTENSIONS_FILE "config/gopher_ext_to_type.txt"
#define FILE_NOT_FOUND_MSG "\n>>> Errore: percorso non valido\n"
#define EMPTY_FOLDER_MSG "\n>>> La cartella richiesta è vuota\n"
#define SERVER_ERROR_MSG "\n>>> Errore sul server\n"


struct GopherElementData {
	char type;
	char* dispName;
	char* path;
	char* host;
	char port[6];
};


struct HandlerData {
	int sock;
	int port;
	char* cli_data;
	char* address;
	char* root_path;
	char* abs_root_path;
};


void freeHandlerDataStruct(struct HandlerData* hd, int process_mode) {
	if (process_mode == 0) {
		free(hd->cli_data);
		free(hd->address);
		free(hd->root_path);
		free(hd->abs_root_path);
		free(hd);
	} else {
		#ifdef __linux__
			int err = 0;
			err |= free_shared_memory(hd->cli_data, strlen(hd->cli_data) + 1);
			err |= free_shared_memory(hd->address, strlen(hd->address) + 1);
			err |= free_shared_memory(hd->root_path, strlen(hd->root_path) + 1);
			err |= free_shared_memory(hd->abs_root_path, strlen(hd->abs_root_path) + 1);
			err |= free_shared_memory(hd, sizeof(hd));
			if (err != 0) throwError(1, err);
		#endif
	} 
}


char getGopherType(char* path, struct Dict ext_dict) {
	char* ext = getExtension(path);
	if (ext == NULL) return '3';

	char* type = getAssocValue(ext, ext_dict);
	if (type == NULL) {
		free(ext);
		return '3';
	}

	free(ext);
	return type[0];
}


char** getDispNamesAssoc(char* path, int *count) {
	char* fname = "_dispnames";
	char dispNamesPath[strlen(path)+strlen(fname)+1];
	sprintf(dispNamesPath, "%s%s", path, fname);
	if (existsFile(dispNamesPath) != 0) return NULL;

	int n;
	char** lista = readlines(dispNamesPath, &n);
	if (lista == NULL) return NULL;

	*count = n;
	return lista;
}


char* getItem(char* path, int *type) {

	char* item = NULL;

	if (existsDir(path) == 0) {
		item = (char*) malloc(strlen(path) + 2);
		if (item == NULL) {
			*type = -1;
			return NULL;
		}
		sprintf(item, "%s/", path);
		*type = 1;
	} else if (existsFile(path) == 0) {
		item = cpyalloc(path);
		if (item == NULL) {
			*type = -1;
			return NULL;
		}
		*type = 0;
	} else {
		*type = -1;
	}

	return item;	

}


char** gopherListDir(char* path, char* req, int *n, struct HandlerData* hd) {

	char* request = (char*) malloc(strlen(req) + 3);
	
	if (request == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	sprintf(request, "/%s/", req);

	request = fixPath(request);
	if (request == NULL) return NULL;

	//////////////////////////////////////////////////////////////////
	// Genero la lista dei file contenuti nel path dato
	int files_count = 0;
	char** files_list = listDir(path, &files_count);

	if (files_list == NULL || files_count == 0) {
		free(request);
		return NULL;
	}

	//////////////////////////////////////////////////////////////////
	// Carico il file delle estensioni riga per riga
	int ext_count;
	char** ext_assoc_list;
	ext_assoc_list = readlines(GOPHER_EXTENSIONS_FILE, &ext_count);
	if (ext_assoc_list == NULL) {
		free(request);
		freeList(files_list, files_count);
		return NULL;
	}

	// Separo i valori di ext_assoc_list in coppie chiave-valore usando un Dict
	struct Dict ext_dict = buildDict(ext_assoc_list, ext_count);
	if (ext_dict.err != 0) {
		free(request);
		freeList(files_list, files_count);
		freeList(ext_assoc_list, ext_count);
		return NULL;
	}
	freeList(ext_assoc_list, ext_count);

	//////////////////////////////////////////////////////////////////
	// Carico il file _dispnames relativo al path dato riga per riga
	int use_assoc = 0;
	int assoc_count;
	struct Dict assoc_dict;
	char** assoc = getDispNamesAssoc(path, &assoc_count);

	if (assoc != NULL) {
		// Separo i valori di assoc in coppie chiave-valore usando un Dict
		assoc_dict = buildDict(assoc, assoc_count);
		if (assoc_dict.err == 0) use_assoc = 1;
	}
	freeList(assoc, assoc_count);

	//////////////////////////////////////////////////////////////////
	// Inizializzo la lista di stringhe di output
	char** gophList = (char** ) malloc(sizeof(char*) * files_count);

	if (gophList == NULL) {
		throwError(1, ALLOC_ERROR);
		free(request);
		freeList(files_list, files_count);
		freeDict(ext_dict);
		freeDict(assoc_dict);
		return NULL;
	}
	//////////////////////////////////////////////////////////////////


	struct GopherElementData ged;

	for (int i = 0; i < files_count; i++) {
		
		ged.path = (char*) malloc(strlen(request) + strlen(files_list[i]) + 1);
		
		if (ged.path == NULL) {
			throwError(1, ALLOC_ERROR);
			free(request);
			freeList(files_list, files_count);
			freeDict(ext_dict);
			freeDict(assoc_dict);
			return NULL;
		}
		
		sprintf(ged.path, "%s%s", request, files_list[i]);

		ged.type = getGopherType(ged.path, ext_dict);

		if (use_assoc == 1) {
			ged.dispName = getAssocValue(files_list[i], assoc_dict);
			if (ged.dispName == NULL) {
				ged.dispName = files_list[i];
			}
		} else {
			ged.dispName = files_list[i];
		}

		ged.host = (char*) malloc(strlen(hd->address) + 1);
		
		if (ged.path == NULL) {
			throwError(1, ALLOC_ERROR);
			free(request);
			freeList(files_list, files_count);
			freeDict(ext_dict);
			freeDict(assoc_dict);
			return NULL;
		}

		strcpy(ged.host, hd->address);

		sprintf(ged.port, "%d", hd->port);

		char* gophString = (char*) malloc(1 + strlen(ged.dispName) + strlen(ged.path) + strlen(ged.host) + strlen(ged.port) + 5);
		
		if (gophString == NULL) {
			throwError(1, ALLOC_ERROR);
			free(request);
			freeList(files_list, files_count);
			freeDict(ext_dict);
			freeDict(assoc_dict);
			free(ged.host);
			return NULL;
		}

		sprintf(gophString, "%c%s\t%s\t%s\t%s", ged.type, ged.dispName, ged.path, ged.host, ged.port);

		gophList[i] = gophString;

		free(ged.path);
		free(ged.host);

	}

	*n = files_count;

	free(request);
	freeList(files_list, files_count);
	freeDict(ext_dict);
	if (use_assoc == 1) freeDict(assoc_dict);
	return gophList;

}


/* Function: handleRequest
* -------------------------------
*  
*/
// char* handleRequest(char* request, size_t* response_sz, int* mapping, struct HandlerData* hd, int process_mode, void*** map) {
char* handleRequest(char* request, size_t* response_sz, int* mapping, struct HandlerData* hd, int process_mode) {

	char* response;

	printf("Richiesta:\n  %s\n", request);

	int type = -1;
	char* req_path = (char*) malloc(strlen(hd->abs_root_path) + strlen(request) + 1);
	if (req_path == NULL) {
		response = cpyalloc(SERVER_ERROR_MSG);
		if (response == NULL) return NULL;
		*response_sz = strlen(response) + 1;
		return response;
	}
	sprintf(req_path, "%s%s", hd->abs_root_path, request);

	req_path = fixPath(req_path);
	if (req_path == NULL) {
		response = cpyalloc(SERVER_ERROR_MSG);
		if (response == NULL) return NULL;
		*response_sz = strlen(response) + 1;
		return response;
	}

	printf("Percorso:\n  %s\n", req_path);

	char* item = getItem(req_path, &type);

	if (type == 0) { // FILE

		printf("  %s -> file\n", item);
		long long file_sz;
		#ifndef __linux__
			HANDLE hMap = createMapping(item, hd->cli_data, &file_sz, process_mode);
			response = readMapping(hMap);
			*response_sz = file_sz;
			*mapping = 1;
		#else
			response = createAndOpenMapping(item, &file_sz, process_mode);
			*response_sz = file_sz;
			*mapping = 1;
		#endif

	} else if (type == 1) { // FOLDER

		printf("  %s -> folder\n", item);
		printf("Genero la lista dei contenuti di %s\n", item);
		int count;
		char** list = gopherListDir(item, request, &count, hd);
		if (list == NULL) {
			response = cpyalloc(EMPTY_FOLDER_MSG);
			if (response == NULL) return NULL;
		} else {
			response = concatList(list, count, '\n');
			if (response == NULL) {
				response = cpyalloc(SERVER_ERROR_MSG);
				if (response == NULL) return NULL;
			}
			freeList(list, count);
		}
		*response_sz = strlen(response);

	} else if (type == -1) { // NOT FOUND
		response = cpyalloc(FILE_NOT_FOUND_MSG);
		if (response == NULL) return NULL;
		*response_sz = strlen(response);
	}

	free(req_path);
	free(item);

	return response;

}


// void* sendResponseFile(void* input) {
// 	struct SendFileData* sfd = (struct SendFileData*) input;
// 	sfd->err = sendFile(sfd->sock, sfd->maps, sfd->response_sz);
// 	return NULL;
// }


void* sendResponse(void* input) {
	struct SendFileData* sfd = (struct SendFileData*) input;
	sfd->err = sendAll(sfd->sock, sfd->response, sfd->response_sz);
	return NULL;
}


int handler(void* input, int process_mode) {

	int err;

	struct HandlerData* hd = (struct HandlerData*) input;

    printf("-----------------------------------------\n");
    printf("Attendo la richiesta di: %d\n\n", hd->sock);

    size_t msg_len;
    char* msg = recvAll(hd->sock, &msg_len);
	if (msg == NULL) return RECV_ERROR;
    msg[msg_len-2] = '\0';

    printf("Bytes received: %ld\n", msg_len);
    printf("Data received: %s\n\n", msg);

	char* request = cpyalloc(msg);
	if (request == NULL) {
		free(msg);
		return ALLOC_ERROR;
	}
	free(msg);

    size_t response_sz;
    int file_request = 0;
	// void** maps;

    char* response = handleRequest(request, &response_sz, &file_request, hd, process_mode);
    
	if (response == NULL) {
		free(request);
		return SERVER_ERROR_H;
	}

	printf("Invio la risposta al client\n\n");
    
	struct SendFileData sfd;

	if (response == NULL) {
		sfd.sock = hd->sock;
		sfd.response = NULL;
		// sfd.maps = maps;
		sfd.response_sz = response_sz;
		sfd.err = 0;
	} else {
		sfd.sock = hd->sock;
		sfd.response = response;
		// sfd.maps = NULL;
		sfd.response_sz = response_sz;
		sfd.err = 0;
	}

	if (file_request == 1) {

		int th_id = startThread(sendResponse, (void*) &sfd, 0);

		if (th_id < 0 || joinCollect(th_id) != 0) {
			free(request);
			freeHandlerDataStruct(hd, process_mode);
			throwError(1, th_id);
			return SERVER_ERROR_H;
		}

		if (sfd.err == 0) {

			char file_sz[20];
			sprintf(file_sz, "%lu", (unsigned long int) response_sz);

			char* pipe_msg = (char*) malloc(strlen(hd->cli_data) + strlen(hd->abs_root_path) + strlen(msg) + strlen(file_sz) + 9);

			if (pipe_msg == NULL) {
				free(msg);
				free(request);
				freeHandlerDataStruct(hd, process_mode);
				throwError(1, ALLOC_ERROR);
				return PIPE_ERROR;
			}

			sprintf(pipe_msg, "%s %s%s %s byte\n", hd->cli_data, hd->abs_root_path, msg, file_sz);

			#ifndef __linux__
				waitEvent("WRITE_LOG_EVENT");

				err = writePipe("LOGGER_PIPE", pipe_msg);
				if (err != 0) printf("ERROR: writing pipe\n");
				
				setEvent("READ_LOG_EVENT");
			#else
				pthread_mutex_lock(shared_lock->mutex);

					while (*(shared_lock->full) == 1) pthread_cond_wait(shared_lock->cond1, shared_lock->mutex);

					err = writePipe(loggerPipe, pipe_msg);
					if (err != 0) {
						throwError(1, err);
					}
					
					*(shared_lock->full) = 1;
					pthread_cond_signal(shared_lock->cond2); 
					
				pthread_mutex_unlock(shared_lock->mutex);
			#endif

			free(pipe_msg);

		}

	} else {
		sendResponse((void*) &sfd);
	}

	if (sfd.err != 0) {
		throwError(1, sfd.err);
	}

	#ifndef __linux__
		shutdown(hd->sock, SD_BOTH);
		closesocket(hd->sock);
	#else
		shutdown(hd->sock, SHUT_RDWR);
		close(hd->sock);
	#endif

	free(request);

	if (file_request == 0) {
		free(response);
	} else {
		err = deleteMapping(response, response_sz);
		if (err != 0) {
			printf("Errore in fun - deleteMapping\n");
			throwError(1, err);
		}
	}

	printf("I handled the request: %d\n\n\n", hd->sock);

	freeHandlerDataStruct(hd, process_mode);

	return 0;

}