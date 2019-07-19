#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"


void* listener(void* input){

	int* err = (int*) create_shared_memory(sizeof(int));

	struct HandlerData* hd = (struct HandlerData*) input;

	handler((void*) hd, 1);

}