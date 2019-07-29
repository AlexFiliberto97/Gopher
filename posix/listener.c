#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


void* listener(void* input){

	// int* err = (int*) create_shared_memory(sizeof(int));

	struct HandlerData* hd = (struct HandlerData*) input;

	int err = handler((void*) hd);

	if (err != 0) throwError(1, err);

	return NULL;

}