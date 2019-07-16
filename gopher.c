#define _GNU_SOURCE
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
	// #include "posix/locking.h"
	#include "posix/logger.h"
	#include "posix/mapping.h"
	#include "posix/thread.h"
	#include "posix/process.h"
#endif


// #define gopherOptions.root_path "root/"
#define GOPHER_EXTENSIONS_FILE "config/gopher_ext_to_type.txt"
#define FILE_NOT_FOUND_MSG "\n>>> Errore: percorso non valido\n"
#define EMPTY_FOLDER_MSG "\n>>> La cartella richiesta è vuota\n"
#define SERVER_ERROR_MSG "\n>>> Errore sul server\n"


//char* gopherOptions.root_path;

struct Gopts {
	char* address;
	int *port;
	char* root_path;
};

struct GopherElementData {
	char type;
	char* dispName;
	char* path;
	char* host;
	int port;
};

static struct Gopts gopherOptions;

int setDefaultGopherOptions() {

	gopherOptions.address = (char*)malloc(10);
	gopherOptions.port = (int*)malloc(sizeof(int));
	gopherOptions.root_path = (char*)malloc(3);
	if (gopherOptions.address == NULL || gopherOptions.port == NULL || gopherOptions.address == NULL) return -1;

	strcpy(gopherOptions.address, "127.0.0.1");
	*gopherOptions.port = 7070;
	strcpy(gopherOptions.root_path, "./");
	return 0;

}

void setGopherOptions(char* address, int* port, char* root_path) {

	free(gopherOptions.address);
	free(gopherOptions.port);
	free(gopherOptions.root_path);

	gopherOptions.address = address;
	gopherOptions.port = port;
	gopherOptions.root_path = root_path;

}


/* Function: getGopherType
* -------------------------------
*  determina il carattere che rappresenta il tipo da associare 
*  all'estensione secondo lo standard Gopher
*  
*  path: percorso del file/cartella
*  dict: struttura Dict contenente l'associazione tra estensioni e tipi
*
*  return: il carattere che rappresenta il tipo
*/
char getGopherType(char* path, struct Dict ext_dict) {
	char* ext = getExtension(path);
	char* type = getAssocValue(ext, ext_dict);
	char t;
	if (type == NULL) {
		t = '3';
	} else {
		t = type[0];
	}
	free(ext);
	return t;
}


/* Function: getDispNamesAssoc
* -------------------------------
*  legge il file _dispnames nella cartella path, se esiste, e ne
*  ritorna il contenuto come lista di stringhe
*
*  path: percorso del file/cartella
*  *count: puntatore in cui salvare il numero di elementi di _dispnames
*  
*  return: una lista con il contenuto del file path/_dispnames diviso per righe
*/
char** getDispNamesAssoc(char* path, int *count) {
	char* fname = "_dispnames";
	char dispNamesPath[strlen(path)+strlen(fname)+1];
	sprintf(dispNamesPath, "%s%s", path, fname);
	if (existsFile(dispNamesPath) != 0) return NULL;
	int n;
	char** lista = readlines(dispNamesPath, &n);
	if (lista == NULL) {
		printf("Errore in getDispNamesAssoc - readlines\n");
		return NULL;
	}
	*count = n;
	return lista;
}


/* Function: getItem
* -------------------------------
*  
*/
char* getItem(char* path, int *type) {

	char* item = NULL;

	int last_slash;
	for (int i = strlen(path)-1; i >= 0; i--) {
		if (path[i] == '/' && i != strlen(path)-1) {
			last_slash = i;
			break;
		}
	}

	char* itemPath = slice(path, 0, last_slash + 1);
	char* itemName;
	if (path[strlen(path)-1] == '/') {
		itemName = slice(path, last_slash + 1, strlen(path)-1);
	} else {
		itemName = slice(path, last_slash + 1, strlen(path));
	}
	char* itemNameSlash = (char*) malloc(sizeof(char) * (strlen(itemName) + 2));
	sprintf(itemNameSlash, "%s/", itemName);

	int files_count;
	char** files_list = listDir(itemPath, &files_count);

	if (files_list == NULL) {
		printf("Errore in getItem - listDir\n");
		return NULL;
	}

	if (searchList(itemNameSlash, files_list, files_count) != -1) {
		item = (char*) malloc(strlen(itemPath) + strlen(itemNameSlash) + 1);
		sprintf(item, "%s%s", itemPath, itemNameSlash);
		*type = 1;
	} else if (searchList(itemName, files_list, files_count) != -1) {
		printf("%s\n", itemName);
		item = (char*) malloc(strlen(itemPath) + strlen(itemName) + 1);
		sprintf(item, "%s%s", itemPath, itemName);
		*type = 0;
	} else {
		*type = -1;
	}

	freeList(files_list, files_count);
	free(itemPath);
	free(itemName);
	free(itemNameSlash);

	return item;

}

/* Function: gopherListDir
* -------------------------------
*  
*/
char** gopherListDir(char* path, int *n) {

	int err;

	//////////////////////////////////////////////////////////////////
	// Genero la lista dei file contenuti nel path dato
	int files_count = 0;
	char** files_list = listDir(path, &files_count);

	if (files_list == NULL) {
		printf("Errore in gopherListDir - listDir\n");
		return NULL;
	} else if (files_count == 0) {
		return NULL;
	}

	//////////////////////////////////////////////////////////////////
	// Carico il file delle estensioni riga per riga
	int ext_count;
	char** ext_assoc_list;
	ext_assoc_list = readlines(GOPHER_EXTENSIONS_FILE, &ext_count);
	if (ext_assoc_list == NULL) {
		printf("Errore in gopherListDir - readlines\n");
		return NULL;
	}

	// Separo i valori di ext_assoc_list in coppie chiave-valore usando un Dict
	struct Dict ext_dict = buildDict(ext_assoc_list, ext_count);
	if (ext_dict.err != 0) {
		printf("Errore in gopherListDir - buildDict\n");
		return NULL;
	}
	freeList(ext_assoc_list, ext_count);

	//////////////////////////////////////////////////////////////////
	// Carico il file _dispnames relativo al path dato riga per riga
	int use_assoc;
	int assoc_count;
	struct Dict assoc_dict;
	char** assoc = getDispNamesAssoc(path, &assoc_count);
	if (assoc == NULL) {
		printf("File _dispnames non disponibile\n");
		use_assoc = 0;
	} else {
		use_assoc = 1;
		// Separo i valori di assoc in coppie chiave-valore usando un Dict
		assoc_dict = buildDict(assoc, assoc_count);
		if (assoc_dict.err != 0) {
			printf("Errore in gopherListDir - buildDict\n");
			return NULL;
		}
		freeList(assoc, assoc_count);
	}

	//////////////////////////////////////////////////////////////////
	// Inizializzo la lista di stringhe di output
	char** gophList = (char** ) malloc(sizeof(char*) * files_count);

	if (gophList == NULL) {
		printf("Errore in gopherListDir - gophList - malloc\n");
		return NULL;
	}

	//////////////////////////////////////////////////////////////////

	struct FilePath fp;
	struct GopherElementData ged;
	fp.path = path;

	for (int i = 0; i < files_count; i++) {
		fp.name = files_list[i];
		ged.type = getGopherType(getFullPath(fp), ext_dict);
		if (use_assoc == 1) {
			ged.dispName = getAssocValue(files_list[i], assoc_dict);
			if (ged.dispName == NULL) {
				ged.dispName = files_list[i];
			}
		} else {
			ged.dispName = files_list[i];
		}
		ged.path = getFullPathNoRoot(fp);
		ged.host = gopherOptions.address;
		ged.port = *gopherOptions.port;

		char port[6];
		// itoa(ged.port, port, 10);
		sprintf(port, "%d", ged.port);

		char* gophString = (char*) malloc(1 + strlen(ged.dispName) + strlen(ged.path) + strlen(ged.host) + strlen(port) + 5);
		if (gophString == NULL) {
			printf("Errore in gopherListDir - malloc\n");
			return NULL;
		}

		sprintf(gophString, "%c%s\t%s\t%s\t%s", ged.type, ged.dispName, ged.path, ged.host, port);


		gophList[i] = gophString;

	}

	*n = files_count;

	freeList(files_list, files_count);
	freeDict(ext_dict);
	if (use_assoc == 1) freeDict(assoc_dict);
	return gophList;

}


/* Function: checkEmptyRequest
* -------------------------------
*  determina se la stringa data in input e' da considerare come vuota,
*  controllandone la lunghezza e ignorando i simboli '/'
*  
*  return: 1 se e' vuota, 0 se non lo e'
*/
int checkEmptyRequest(char* req) {
	if (strlen(req) == 0) return 1;
	for (int i = 0; i < strlen(req); i++) {
		if (req[i] != '/') return 0;
	}
	return 1;
}


/* Function: handleRequest
* -------------------------------
*  
*/
char* handleRequest(char* request, size_t* response_sz, int* mapping, char* cli_data, int process_mode) {

	char* response;

	if (checkEmptyRequest(request) != 0) {
		printf("Richiesta: \n  %s\n", gopherOptions.root_path);

		int count;
		char** list = gopherListDir(gopherOptions.root_path, &count);
		if (list == NULL) {
			printf("Errore in handleRequest - gopherListDir\n");
			return NULL;
		}
		printf("Genero la lista dei contenuti di %s\n", gopherOptions.root_path);
		response = concatList(list, count, '\n');
		if (response == NULL) {
			printf("Errore in handleRequest - concatList\n");
			response = cpyalloc(SERVER_ERROR_MSG);
		}
		*response_sz = strlen(response);
		freeList(list, count);

	} else {
		printf("Richiesta:\n  %s\n", request);

		int type = -1;
		char* input_path = (char*) malloc(strlen(gopherOptions.root_path) + strlen(request) + 1);
		if (input_path == NULL) {
			printf("Errore in handleRequest - malloc\n");
			response = cpyalloc(SERVER_ERROR_MSG);
			return response;
		}
		sprintf(input_path, "%s%s", gopherOptions.root_path, request);

		char* req_path = fixPath(input_path);
		free(input_path);
		if (req_path == NULL) {
			printf("Errore in handleRequest - fixPath\n");
			response = cpyalloc(SERVER_ERROR_MSG);
			return response;
		}

		printf("Percorso: \n  %s\n", req_path);

		char* item = getItem(req_path, &type);

		if (type == 0) { // FILE

			printf("  %s -> file\n", item);
			size_t file_sz;
			#ifndef __linux__
				HANDLE hMap = createMapping(item, cli_data, &file_sz, process_mode);
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
			char** list = gopherListDir(item, &count);
			if (list == NULL) {
				printf("Errore in handleRequest - gopherListDir\n");
				response = cpyalloc(EMPTY_FOLDER_MSG);
			} else {
				response = concatList(list, count, '\n');
				freeList(list, count);
			}
			*response_sz = strlen(response);

		} else if (type == -1) { // NOT FOUND

			printf("Il percorso %s non e' valido\n", req_path);
			response = cpyalloc(FILE_NOT_FOUND_MSG);
			*response_sz = strlen(response);

		}

		free(req_path);
		free(item);
	}

	return response;

}


void* sendFile(void* input) {
	struct SendFileData* sfd = (struct SendFileData*) input;
	int err = sendAll(sfd->sock, sfd->response, sfd->response_sz);
	// return (void*) err;
}


int handler(void* input, int process_mode) {

	int err;

	struct ClientData* cd = (struct ClientData*) input;
	char* cli_data= cd->data;

	int sock = cd->sock;

    printf("-----------------------------------------\n");
    printf("Attendo la richiesta di: %d\n\n", sock);

    size_t msg_len;
    char* msg = recvAll(sock, &msg_len);
    msg[msg_len-2] = '\0';

    printf("Bytes received: %ld\n", msg_len);
    printf("Data received: %s\n\n", msg);

    char* request = (char*) malloc(strlen(msg) + 1);
    strcpy(request, msg);

    free(msg);

    size_t response_sz;
    int file_request = 0;

    char* response = handleRequest(request, &response_sz, &file_request, cli_data, process_mode);
    if (response == NULL) {
    	printf("Errore in handler - handleRequest\n");
    }

	printf("Invio la risposta al client\n\n");

	if (file_request != 0) {
		struct SendFileData sfd = {sock, response, response_sz};

		#ifndef __linux__
			HANDLE th = (HANDLE) startThread(sendFile, (void*) &sfd);
			WaitForSingleObject(th, INFINITE);
		#else
			int th = startThread(sendFile, (void*) &sfd);
			if (th < 0) return -1;
			if (pthread_join(th, NULL) != 0) return -1;
		#endif

		char file_sz[11];
		sprintf(file_sz, "%lu", (unsigned long int) response_sz);

		char* pipe_msg = (char*) malloc(strlen(cli_data) + strlen(request) + strlen(file_sz) + 9);
		sprintf(pipe_msg, "%s %s %s byte\n", cli_data, request, file_sz);

		#ifndef __linux__
		 	waitEvent("WRITE_LOG_EVENT");
		 	err = writePipe("LOGGER_PIPE", pipe_msg);
		 	if (err < 0) {/* on va bene*/ }
		 	setEvent("READ_LOG_EVENT");
		#else
		    pthread_mutex_lock(shared_lock->mutex);
	        while (*(shared_lock->full) == 1) pthread_cond_wait(shared_lock->cond1, shared_lock->mutex);

	        writePipe("LOGGER_PIPE", pipe_msg);

	        *(shared_lock->full) = 1;
	        pthread_cond_signal(shared_lock->cond2); 
		    pthread_mutex_unlock(shared_lock->mutex);

		#endif

		free(pipe_msg);

	} else {
		err = sendAll(sock, response, response_sz);
	}

	#ifndef __linux__
		shutdown(sock, SD_BOTH);
		closesocket(sock);
	#else
		shutdown(sock, SHUT_RDWR);
		close(sock);
	#endif

	free(request);
	if (file_request == 0) {
		free(response);
	} else {
		#ifndef __linux__
			if (deleteMapping(response) != 0) {
				printf("Errore in fun - deleteMapping\n");
				free(cd->data);
				return -1;
			}
		#else
			if (deleteMapping(response, response_sz) != 0) {
				printf("Errore in fun - deleteMapping\n");
				free(cd->data);
				return -1;
			}
		#endif
	}

	free(cd->data);
	printf("I handled the request: %d\n\n\n", sock);
	return 0;

}