#ifndef LOCKING_H

    #define LOCKING_H_

    #include "locking.c"

    struct flock create_lock();
    int lock_fd(int, struct flock, long long, long long);
    int unlock_fd(int, struct flock);


#endif