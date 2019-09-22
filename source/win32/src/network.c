#include <stdio.h>
#include <stdlib.h>
#include "../include/network.h"
#include "../include/utils_win32.h"
#include "../include/locking.h"
#include "../../common/include/error.h"
#include "../../common/include/const.h"
#include "../../common/include/network.h"

/* Function: sendFile
*  Sends file to client in process mode.
*/
int sendFileProc(SendFileData* sfd) {

    int err;
    BOOL succ;
    void* view_ptr;
    long long offset = 0, bytes_left = 0;
    int n_bytes = 0;

    sfd->size = getFileSize(sfd->file);
    if (sfd->size < 0) return sfd->size;
    

    HANDLE hFile = NULL, hMap = NULL;

    hFile = CreateFile(sfd->file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE || hFile == NULL) {
        throwError(INVALID_HANDLE);
        return -1;
    }

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == INVALID_HANDLE_VALUE || hMap == NULL) {
        CloseHandle(hFile);
        return -1;
    }

    while (offset < sfd->size) {
        
        DWORD offsetHigh = (DWORD) (offset >> 32);
        DWORD offsetLow = (DWORD) ((offset << 32) >> 32);
        OVERLAPPED overlapped = {0};
        overlapped.Offset = offsetLow;
        overlapped.OffsetHigh = offsetHigh;

        bytes_left = sfd->size - offset;
        n_bytes = bytes_left >= MAP_VIEW_SIZE ? MAP_VIEW_SIZE : bytes_left;

        succ = lockFile(hFile, n_bytes, &overlapped);
        if (!succ) {
            CloseHandle(hMap);
            CloseHandle(hFile);
            return LOCK_FILE;
        } 

        view_ptr = MapViewOfFile(hMap, FILE_MAP_READ, offsetHigh, offsetLow, (SIZE_T) n_bytes);
        if (view_ptr == NULL) {
            unlockFile(hFile, n_bytes, &overlapped);
            CloseHandle(hMap);
            CloseHandle(hFile);
            return SEND_FILE_ERROR;
        }

        err = sendAll(sfd->sock, (char*) view_ptr, n_bytes);
        if (err != 0) {
            throwError(err);
            unlockFile(hFile, n_bytes, &overlapped);
            CloseHandle(hMap);
            CloseHandle(hFile);
            return SEND_FILE_ERROR;
        }

        offset += n_bytes;

        BOOL succ = UnmapViewOfFile(view_ptr);
        if (!succ) throwError(DELETE_MAPPING);

        succ = unlockFile(hFile, n_bytes, &overlapped);
        if (!succ) {
            CloseHandle(hMap);
            CloseHandle(hFile);
            return UNLOCK_FILE;
        }
    }
    
    CloseHandle(hMap);
    CloseHandle(hFile);
    return 0;
}