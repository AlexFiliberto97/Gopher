#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include "../include/utils_posix.h"
#include "../include/network.h"
#include "../include/locking.h"
#include "../../common/include/network.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"

/* Function: sendFile
*  Sends file to client in process mode
*/
int sendFileProc(SendFileData* sfd) {

    int err;
    void* view_ptr;
    long long offset = 0, bytes_left;
    int n_bytes = 0;

    sfd->size = getFileSize(sfd->file);
    if (sfd->size < 0) return sfd->size;


    int fd = open(sfd->file, O_RDWR);
    if (fd == -1) return CREATE_MAPPING;

    struct flock lock = create_lock();

    while (offset < sfd->size) {

        bytes_left = sfd->size - offset;
        n_bytes = bytes_left >= MAP_VIEW_SIZE ? MAP_VIEW_SIZE : bytes_left;

        err = lock_fd(fd, lock, offset, n_bytes);
        if (err != 0) {
            close(fd);
            return LOCK_FILE;
        } 

        view_ptr = mmap(NULL, n_bytes, PROT_READ, MAP_PRIVATE, fd, offset);
        if (view_ptr == MAP_FAILED) {
            close(fd);
            return MAP_VOF_ERROR;
        }

        err = sendAll(sfd->sock, (char*) view_ptr, n_bytes);
        if (err != 0) {
            throwError(err);
            err = munmap(view_ptr, n_bytes);
            if (err != 0) throwError(DELETE_MAPPING);
            close(fd);
            return SEND_FILE_ERROR;
        }

        offset += n_bytes;

        err = munmap(view_ptr, n_bytes);
        if (err != 0) throwError(DELETE_MAPPING);

        err = unlock_fd(fd, lock);
        if (err != 0) {
            close(fd);
            return UNLOCK_FILE;
        } 
    }

    close(fd);
    return 0;
}