#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* Function: create_lock
*  Create a file locking.
*/
struct flock create_lock() {

    struct flock fl;
    memset(&fl, 0, sizeof(struct flock));

    fl.l_whence = SEEK_SET;
    fl.l_start = 0;         
    fl.l_len = 0; 
    return fl;
}

/* Function: reset_lock
*  Reset a lock.
*/
int reset_lock(struct flock fl) {
    
    memset(&fl, 0, sizeof(struct flock));
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;         
    fl.l_len = 0; 
    return 0;
}

/* Function: lock_fd
*  Lock a given file descriptor.
*/
int lock_fd(int fd, struct flock fl, long long offset, long long size) {
    
    fl.l_start = (off_t) offset;
    fl.l_len = (off_t) size;
    fl.l_type = F_WRLCK;
    return (fcntl(fd, F_SETLKW, &fl) == -1) ? -1 : 0;
}

/* Function: unlock_fd
*  Unlock a given file descriptor.
*/
int unlock_fd(int fd, struct flock fl) {
    
    fl.l_type = F_UNLCK;
    return (fcntl(fd, F_SETLKW, &fl) == -1) ? -1 : 0;
}