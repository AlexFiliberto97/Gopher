// #define _GNU_SOURCE
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
		return 0;
	}
	return 1;
}


int existsFile(char* path) {
	if (isRegularFile(path) == 0) {
		return 0;
	}
	return 1;
}


/* Function: countDirElements
* -------------------------------
*  
*/
int countDirElements(char* path) {

	DIR* dp;
  	struct dirent* ep;     
  	dp = opendir(path);

  	int count = 0;
	
	if (dp == NULL) {
		perror("Couldn't open the directory");
    	return -1;
    }

	while (ep = readdir(dp)) {
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


/* Function: listDir
* -------------------------------
*  
*/
char** listDir(char* path, int* count) {

	int n = countDirElements(path);

	if (n == -1) {
		perror("Errore in countDirElements");
		return NULL;
	}

	*count = n;

	char** list = (char**) malloc(sizeof(char*) * n);
	if(list == NULL) {
		printf("Errore in listDir - malloc\n");
		return NULL;
	}

	DIR* dp;
  	struct dirent* ep;     
  	dp = opendir(path);

	if (dp == NULL) {
		perror("Couldn't open the directory");
    	return NULL;
    }

    int i = 0;

	while (ep = readdir(dp)) {
		if (strcmp(ep->d_name, ".") != 0 &&
			strcmp(ep->d_name, "..") != 0 &&
			strcmp(ep->d_name, "_dispnames") != 0) {

			char tmp_path[strlen(path) + strlen(ep->d_name) + 1];
			sprintf(tmp_path, "%s%s", path, ep->d_name);

			if (isDirectory(tmp_path)) {
				char* filename = (char*) malloc(strlen(ep->d_name) + 2);
  				sprintf(filename, "%s/", ep->d_name);
  				list[i++] = filename;
			} else if (isRegularFile(tmp_path)) {
  				char* filename = (char*) malloc(strlen(ep->d_name) + 1);
  				strcpy(filename, ep->d_name);
  				list[i++] = filename;
			}
  		}
	}

	(void) closedir(dp);

 	return list;

}


size_t getFileSize2(char* file_name) {

    struct stat st;
     
    if (stat(file_name, &st) == 0) {
        return (st.st_size);
    } else
        return -1;

}

/* Function: readFile
* -------------------------------
*  
*/
char* readFile(char* path, size_t* size) {

	FILE* fp;
	fp = fopen(path, "rb");

	if (fp == NULL) {
		printf("file non valido");
		return NULL;
	}

	size_t sz = getFileSize2(path);

	if (sz == -1) {
		printf("Errore in readFile - getFileSize\n");
		fclose(fp);
		return NULL;
	}

	char* buf = (char *) malloc(sz + 1);

	if (buf == NULL) {
		printf("Errore in readFile - malloc\n");
		fclose(fp);
		return NULL;
	}

	size_t read = fread(buf, 1, sz, fp);

	if (read != sz) {
		printf("Errore in readFile - fread\n");
		fclose(fp);
		return NULL;
	}

	fclose(fp);

	buf[sz] = '\0';

	// return buf;

	char* data = (char*) malloc(sz + 1);

	int idx = 0;

	for (int i = 0; i < strlen(buf); i++) {
		if (buf[i+1] == '\n' && i + 1 < strlen(buf)) continue;
		data[idx++] = buf[i];
	}
	data[idx] = '\0';

	data = (char*) realloc(data, strlen(data) + 1);

	*size = strlen(data) + 1;
	free(buf);
	return data;

}


void* create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(NULL, size, protection, visibility, -1, 0);
}


int free_shared_memory(void* map, size_t size) {

	int err = munmap(map, size);

	if (err != 0) return -1;

	return 0;

}