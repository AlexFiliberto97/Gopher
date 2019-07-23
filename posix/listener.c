#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../gopher.h"
#include "../utils.h"
#include "../network.h"

void* listener(void* input){

	struct HandlerData* hd = (struct HandlerData*) input;
	int err = handler((void*) hd, 1);
	if (err != 0) throwError(1, err);

	return NULL;
}