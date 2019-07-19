// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


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