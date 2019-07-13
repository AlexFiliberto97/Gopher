#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "server.h"


//#include "network.h"

#ifndef __linux__
	#include "win32/environment.h"
#else
	#include "posix/environment.h"
#endif




//Globals variables

//int sock;

/*


int getOpts(int argc, char** args, struct Opts* opts) {

	int port = 7070;
	int process_mode = 0;

	for(int i = 0; i < argc; i++){
		if(strcmp(args[i], "-p") == 0 && i < argc -1 && isNumeric(args[i+1])){
			port = atoi(args[i+1]);
			if (checkPort(port) != 0) {
				printf("Errore: la root specificata non e' valida\n");
				return 1;
			}

		} else if (strcmp(args[i], "-process") == 0) {
			process_mode = 1;
		}
	}
	opts->port = port;
	opts->process_mode = process_mode;

	return 0;

}


int getConfig(struct Opts* opts) {

	int config_count;
	char **config_assoc_list;
	config_assoc_list = readlines(CONFIG_FILE_PATH, &config_count);
	if (config_assoc_list == NULL) {
		printf("Errore in gopherListDir - readlines\n");
		return 1;
	}

	struct Dict config_dict = buildDict(config_assoc_list, config_count);
	if (config_dict.err != 0) {
		printf("Errore in gopherListDir - buildDict\n");
		return 1;
	}
	freeMDArray(config_assoc_list, config_count);

	char* root_path_v = getAssocValue("root", config_dict);
	char* port_v = getAssocValue("port", config_dict);
	char* process_mode_v = getAssocValue("process_mode", config_dict);

	if (root_path_v == NULL || port_v == NULL || process_mode_v == NULL) {
		printf("Errore nel file di configurazione\n");
		return 1;
	}

	if ((root_path_v[strlen(root_path_v)-1] != '/') || (existsDir(root_path_v) != 0)) {
		freeDict(config_dict);
		return 2;
	}

	int port = atoi(port_v);
	int process_mode = atoi(process_mode_v);

	if (checkPort(port) != 0) {
		printf("Errore: il numero di porta non e' valido");
		freeDict(config_dict);
		return 1;
	}

	if (process_mode != 0 && process_mode != 1) {
		printf("Errore: la selezione di process_mode non e' valida");
		freeDict(config_dict);
		return 1;
	}

	opts->port = port;
	opts->process_mode = process_mode;
	opts->root_path = (char*) malloc(strlen(root_path_v) + 1);
	strcpy(opts->root_path, root_path_v);

	freeDict(config_dict);

	return 0;

}
*/



int main(int argc, char** argv){

	init_env();
	int error = start_env();

	if (error == 0) {
		
		printf("environment started!\n");
		
		error = serverInit(argc, argv);
		if (error ==-1) return -1;
		
		error = serverStart();

		serverService();

		//printf("environment error\n");
	
	}
	
	//printf("porcamadonna: %d\n", error);

	//Sleep(10000);
	
	//printf("cazzodio\n");
	
	//destroyProcess();

	/*
	struct Opts opts = {7070, 1, "root/"};

	int err;
	
	err = getConfig(&opts);

	// Starting the environment
	if (err == 2) {
		printf("Errore: la root specificata non e' valida\n");
		return 1;
	}
	if (err == 1) {
		printf("Errore nell'inserimento delle opzione da file di configurazione\nUtilizzo le opzioni di default\n");
	}
	err = getOpts(argc, argv, &opts);
	if (err != 0) {
		printf("Errore nell'inserimento delle opzione da linea di comando\nUtilizzo le opzioni di default\n");
	}

	if (start_env() == TRUE) {

		serverInit();

		printf("%s\n", opts.root_path);

		int sock = serverStart(opts);

		// BOOL succ = SetConsoleCtrlHandler(consoleHandler, TRUE);
		// if(!succ) return 1; // Error setting the console event

		serverService(sock, opts);
	} else { Error: creating server }*/

	return 0;

}