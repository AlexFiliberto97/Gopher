#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../error.h"

int isDirectory(const char* path) {
    
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

int isRegularFile(const char* path) {
    
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int existsDir(char* path) {
	
	if (isDirectory(path) == 0) {
		return -1;
	}
	return 0;
}

int existsFile(char* path) {

	if (isRegularFile(path) == 0) {
		return -1;
	}
	return 0;
}

void freeArray(char **list, int count) {
	
	for (int i = 0; i < count; i++) {
		free(list[i]);
	}
	free(list);
}

int countDirElements(char* path) {

	DIR* dp;
  	struct dirent* ep;     
  	dp = opendir(path);
  	int count = 0;
	if (dp == NULL) return FOLDER_ERROR;

	while ((ep = readdir(dp)) != 0) {
		if (strcmp(ep->d_name, ".") != 0 &&
			strcmp(ep->d_name, "..") != 0 &&
			strcmp(ep->d_name, "_dispnames") != 0) {

			char tmp_path[strlen(path) + strlen(ep->d_name) + 1];
			sprintf(tmp_path, "%s%s", path, ep->d_name);

			if (isDirectory(tmp_path) || isRegularFile(tmp_path)) {
  				count++;
			}
  		}
	}

	closedir(dp);
 	return count;
}

char** listDir(char* path, int* count) {

	int n = countDirElements(path);
	if (n == FOLDER_ERROR) {
		throwError(1, n);
		return NULL;
	}

	*count = n;
	char** list = (char**) malloc(sizeof(char*) * n);
	if(list == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	DIR* dp;
  	struct dirent* ep;     
  	dp = opendir(path);

	if (dp == NULL) {
		free(list);
		throwError(1, FOLDER_ERROR);
    	return NULL;
    }

    int i = 0;

	while ((ep = readdir(dp)) != 0) {
		if (strcmp(ep->d_name, ".") != 0 &&
			strcmp(ep->d_name, "..") != 0 &&
			strcmp(ep->d_name, "_dispnames") != 0) {

			char tmp_path[strlen(path) + strlen(ep->d_name) + 1];
			sprintf(tmp_path, "%s%s", path, ep->d_name);

			if (isDirectory(tmp_path)) {
				char* filename = (char*) malloc(strlen(ep->d_name) + 2);
				if (filename == NULL) {
					freeArray(list, i);
					throwError(1, ALLOC_ERROR);
					return NULL;
				}
  				sprintf(filename, "%s/", ep->d_name);
  				list[i++] = filename;
			} else if (isRegularFile(tmp_path)) {
  				char* filename = (char*) malloc(strlen(ep->d_name) + 1);
				if (filename == NULL) {
					freeArray(list, i);
					throwError(1, ALLOC_ERROR);
					return NULL;
				}
  				strcpy(filename, ep->d_name);
  				list[i++] = filename;
			}
  		}
	}

	closedir(dp);
 	return list;
}

size_t getFileSize(char* file_name) {

    struct stat st;
     
    if (stat(file_name, &st) == 0) {
        return (st.st_size);
    } else {
        return FILE_SIZE_ERROR;
    }
}

char* readFile(char* path, size_t* size) {

	FILE* fp = fopen(path, "rb");
	if (fp == NULL) {
		throwError(1, OPEN_FILE_ERROR);
		return NULL;
	}

	size_t sz = getFileSize(path);
	if (sz == FILE_SIZE_ERROR) {
		throwError(1, FILE_SIZE_ERROR);
		fclose(fp);
		return NULL;
	}

	char* buf = (char *) malloc(sz + 1);
	if (buf == NULL) {
		throwError(1, ALLOC_ERROR);
		fclose(fp);
		return NULL;
	}

	size_t read = fread(buf, 1, sz, fp);
	if (read != sz) {
		throwError(1, READ_FILE_ERROR);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	buf[sz] = '\0';
	char* data = (char*) malloc(sz + 1);
	if (data == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	int idx = 0;
	for (int i = 0; i < strlen(buf); i++) {
	 	if (buf[i] == '\r' && buf[i+1] == '\n' && i + 1 < strlen(buf)) continue;
	 	data[idx++] = buf[i];
	}
	data[idx] = '\0';

	data = (char*) realloc(data, strlen(data) + 1);
	if (data == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	*size = strlen(data) + 1;
	free(buf);
	return data;
}

void* create_shared_memory(size_t size) {
    
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
}

int free_shared_memory(void* map, size_t size) {
	
	int err = munmap(map, size);
	if (err != 0) return DELETE_MAPPING;
	return 0;
}