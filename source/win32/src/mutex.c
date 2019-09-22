#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../include/mutex.h"
#include "../include/event.h"
#include "../../common/include/utils.h"
#include "../../common/include/const.h"
#include "../../common/include/error.h"

/* Function: createMutexCV
*  Create new mutex.
*/
MutexCV* createMutexCV() {

    SECURITY_ATTRIBUTES mutexSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    SECURITY_ATTRIBUTES eventSA = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

    MutexCV* mcv = (MutexCV*) malloc(sizeof(MutexCV));
    if (mcv == NULL) {
        throwError(ALLOC_ERROR);
        return NULL;
    }
    
    mcv->mutex = CreateMutexA(&mutexSA, FALSE, NULL);
    if (mcv->mutex == NULL) {
        throwError(CREATE_MUTEX_ERR);
        return NULL;
    }

    mcv->cv = CreateEventA(&eventSA, FALSE, FALSE, NULL);
    if (mcv->cv == NULL) {
        throwError(CREATE_EVENT);
        return NULL;
    } 

    return mcv;
}

/* Function: mutexLock
*  Lock the given mutex.
*/
void mutexLock(HANDLE mutex) {
    
    WaitForSingleObject(mutex, INFINITE);
}

/* Function: mutexUnlock
*  Unlock the given mutex.
*/
void mutexUnlock(HANDLE mutex) {
    
    ReleaseMutex(mutex);
}

/* Function: mutexUnlock
*  Wait for a given mutex.
*/
void mcvWait(MutexCV* mcv) {

    mutexUnlock(mcv->mutex);
    WaitForSingleObject(mcv->cv, MUTEX_TIMEOUT);
    mutexLock(mcv->mutex);
}

/* Function: desroyMutextCV
*  Destroy a MutexCv struct.
*/
void destroyMutexCV(MutexCV* mcv) {
    
    CloseHandle(mcv->mutex);
    CloseHandle(mcv->cv);
    free(mcv);
}