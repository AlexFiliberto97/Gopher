// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


// int processHandler(void* input) {
// 	return handler(input, 1);
// }


// void* listener(int argc, char** argv){

// 	printf("PROONDNJNJ\n");

// 	// serverInit();
// 	struct ClientData cd;
// 	cd.sock = atoi(argv[1]);
// 	cd.data = argv[1];


// 	printf("%s\n", argv[0]);

// 	// char* address = (char*) malloc(strlen(argv[5]) + 1);
// 	// int* port = (int*) malloc(sizeof(int));
// 	// char* root_path = (char*) malloc(strlen(argv[7]) + 1);

// 	// strcpy(address, argv[5]);
// 	// *port = atoi(argv[6]);
// 	// strcpy(root_path, argv[7]);

// 	// setGopherOptions(address, port, root_path);

// 	// handler((void*) &cd, 1);

// 	return 0;

// }

// struct ListenerData {
// 	int sock;
// 	int port;
// 	char* cli_data;
// 	char* address;
// 	char* root_path;
// };




void* listener(void* input){

	int* err = (int*) create_shared_memory(sizeof(int));

	struct HandlerData* hd = (struct HandlerData*) input;

	printf("%d\n", hd->sock);
	printf("%s\n", hd->cli_data);
	printf("%s\n", hd->address);
	printf("%d\n", hd->port);
	printf("%s\n", hd->root_path);

	handler((void*) hd, 1);

	*err = 0;
	*err |= free_shared_memory(hd->cli_data, strlen(hd->cli_data) + 1);
	*err |= free_shared_memory(hd->address, strlen(hd->address) + 1);
	*err |= free_shared_memory(hd->root_path, strlen(hd->root_path) + 1);
	*err |= free_shared_memory(hd->cli_data, strlen(hd->cli_data) + 1);
	*err |= free_shared_memory(hd, sizeof(hd));

	if (*err != 0) {
		printf("ERRORE: free_shared_memory\n");
	}

	return err;

}