#include <stdio.h>
#include "../include/listener.h"
#include "../../common/include/error.h"
#include "../../common/include/gopher.h"

void* listener(void* input) {

	HandlerData* hd = (HandlerData*) input;
	int err = handler((void*) hd);
	if (err != 0) throwError(err);
	return NULL;
}