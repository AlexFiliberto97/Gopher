#ifndef _GNU_SOURCE 
    #define _GNU_SOURCE 
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/file.h>
#include "process.h"
#include "thread.h"
#include "mutex.c"


struct flock create_lock() {

    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));

    // lock entire file
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;         
    fl.l_len = 0; 

    return fl;

}


int lock_fd(int fd, struct flock fl, long long offset, long long size) {
    fl.l_start = (off_t) offset;
    fl.l_len = (off_t) size;
    fl.l_type = F_WRLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        return -1;
    }
    return 0;
}


int unlock_fd(int fd, struct flock fl) {
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &fl) == -1) {
        return -1;
    }
    return 0;
}