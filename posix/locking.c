// #ifndef _GNU_SOURCE 
//     #define _GNU_SOURCE 
// #endif
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/mman.h>
// #include <pthread.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <sys/stat.h> 
// #include <sys/types.h> 
// #include <sys/file.h>
// #include "process.h"
// #include "thread.h"
// #include "mutex.c"


// struct LockTest {
//     int fd;
//     char* msg;
// };


// struct flock create_lock() {

//     struct flock fl = {0};
//     // memset(&fl, 0, sizeof(struct flock));

//     // lock entire file
//     fl.l_whence = SEEK_SET;
//     fl.l_start = 0;         
//     fl.l_len = 0; 

//     return fl;

// }



// int lock_fd(int fd, struct flock fl) {
//     fl.l_type = F_WRLCK;
//     if (fcntl(fd, F_SETLKW, &fl) == -1) {
//         return -1;
//     }
//     return 0;
// }

// int unlock_fd(int fd, struct flock fl) {
//     fl.l_type = F_UNLCK;
//     if (fcntl(fd, F_SETLKW, &fl) == -1) {
//         return -1;
//     }
//     return 0;
// }

// int lock_fd2(int fd) {
//     if (flock(fd, LOCK_EX) == -1) {
//         printf("LOCK ERROR %d\n", errno);
//         return -1;
//     }
//     return 0;
// }

// int unlock_fd2(int fd) {
//     if (flock(fd, LOCK_UN) == -1) {
//         printf("UNLOCK ERROR %d\n", errno);
//         return -1;
//     }
//     return 0;
// }


// void* test(void* input) {

//     int err;

//     struct LockTest* dc = (struct LockTest*) input;

//     // struct flock fl = create_lock();

//     // err = lock_fd(dc->fd, fl);
//     err = lock_fd2(dc->fd);

//     if (err != 0) {
//         printf("LOCK ERROR %d\n", errno);
//         return NULL;
//     }

//     for (int i = 0; i < 50; i++) {
//         write(dc->fd, dc->msg, (size_t) strlen(dc->msg));
//     }

//     // err = unlock_fd(dc->fd, fl);
//     err = unlock_fd2(dc->fd);

//     if (err != 0) {
//         printf("UNLOCK ERROR %d\n", errno);
//         return NULL;
//     }

// }


// int main() {

//     system("> TEST.txt");

//     int fd = open("TEST.txt", O_RDWR | O_CREAT | O_APPEND, 0666);

//     if (fd == -1) {
//         printf("NONE\n");
//         return 1;
//     }

//     pthread_mutex_t* mutex = create_mutex();

//     struct LockTest* dc1 = (struct LockTest*) malloc(sizeof(struct LockTest));
//     dc1->fd = fd;
//     dc1->msg = (char*) malloc(20);
//     strcpy(dc1->msg, "porcane\n");

//     struct LockTest* dc2 = (struct LockTest*) malloc(sizeof(struct LockTest));
//     dc2->fd = fd;
//     dc2->msg = (char*) malloc(20);
//     strcpy(dc2->msg, "madonnona\n");

//     // pthread_t th1 = startThread(test, dc1, 0);
//     // pthread_t th2 = startThread(test, dc2, 0);
//     // joinCollect(th1);
//     // joinCollect(th2);

//     int pid1 = startProcess(test, dc1);
//     int pid2 = startProcess(test, dc2);
//     int status;
//     waitpid(pid1, &status, 0);
//     waitpid(pid2, &status, 0);


//     close(fd);

// }

