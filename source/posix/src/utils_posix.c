#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../include/utils_posix.h"
#include "../../common/include/utils.h"
#include "../../common/include/error.h"

/* Function: isDirectory
*  Check if a path is a directory.
*/
int isDirectory(char* path) {
   
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/* Function: isRegularFile
*  Check if a path is a regular file.
*/
int isRegularFile(char* path) {
    
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

/* Function: existsDir
*  Check if a directory exists.
*/
int existsDir(char* path) {
	
	return (isDirectory(path) == 0) ? -1 : 0;
}

/* Function: existsFile
*  Check if a file exists.
*/
int existsFile(char* path) {

	return (isRegularFile(path) == 0) ? -1 : 0;
}

/* Function: countDirElements
*  Count elements in a given path. 
*/
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

			char* tmp_path = (char*) malloc(strlen(path) + strlen(ep->d_name) + 1);
			if (tmp_path == NULL) {
				closedir(dp);
				return ALLOC_ERROR;
			}
			sprintf(tmp_path, "%s%s", path, ep->d_name);
			if (isDirectory(tmp_path) || isRegularFile(tmp_path)) count++;
			free(tmp_path);
  		}
	}
	closedir(dp);
 	return count;
}

/* Function: listDir
*  List the content for a given path.
*/
char** listDir(char* path, int* count) {

	int n = countDirElements(path);
	if (n < 0) {
		throwError(n);
		return NULL;
	}

	*count = n;
	char** list = (char**) malloc(sizeof(char*) * n);
	if(list == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	DIR* dp;
  	struct dirent* ep;     
  	dp = opendir(path);
	if (dp == NULL) {
		free(list);
		throwError(FOLDER_ERROR);
    	return NULL;
    }

    int i = 0;
	while ((ep = readdir(dp)) != 0) {
		if (strcmp(ep->d_name, ".") != 0 &&
			strcmp(ep->d_name, "..") != 0 &&
			strcmp(ep->d_name, "_dispnames") != 0) {

			char* tmp_path = (char*) malloc(strlen(path) + strlen(ep->d_name) + 1);
			if (tmp_path == NULL) {
				closedir(dp);
				throwError(ALLOC_ERROR);
				return NULL;
			}

			sprintf(tmp_path, "%s%s", path, ep->d_name);
			if (isDirectory(tmp_path)) {
				char* filename = (char*) malloc(strlen(ep->d_name) + 2);
				if (filename == NULL) {
					freeList(list, i);
					free(tmp_path);
					closedir(dp);
					throwError(ALLOC_ERROR);
					return NULL;
				}

  				sprintf(filename, "%s/", ep->d_name);
  				list[i++] = filename;
			
			} else if (isRegularFile(tmp_path)) {
  				
  				char* filename = (char*) malloc(strlen(ep->d_name) + 1);
				if (filename == NULL) {
					freeList(list, i);
					free(tmp_path);
					closedir(dp);
					throwError(ALLOC_ERROR);
					return NULL;
				}
  				strcpy(filename, ep->d_name);
  				list[i++] = filename;
			}
			free(tmp_path);
  		}
	}
	closedir(dp);
 	return list;
}

/* Function: getFileSize
*  Return the size for a given file.
*/
size_t getFileSize(char* file_name) {

    struct stat st;
    return (stat(file_name, &st) == 0) ? st.st_size : FILE_SIZE_ERROR;
}

/* Function: readFile
*  Return the content for a given file.
*/
char* readFile(char* path, int* size) {

	FILE* fp = fopen(path, "rb");

	if (fp == NULL) {
		throwError(OPEN_FILE_ERROR);
		return NULL;
	}

	size_t sz = getFileSize(path);
	if (sz < 0) {
		throwError(sz);
		fclose(fp);
		return NULL;
	}

	char* buf = (char *) malloc(sz + 1);
	if (buf == NULL) {
		throwError(ALLOC_ERROR);
		fclose(fp);
		return NULL;
	}

	size_t read = fread(buf, 1, sz, fp);
	if (read != sz) {
		throwError(READ_FILE_ERROR);
		fclose(fp);
		free(buf);
		return NULL;
	}

	fclose(fp);
	buf[sz] = '\0';
	char* data = (char*) malloc(sz + 1);
	if (data == NULL) {
		throwError(ALLOC_ERROR);
		free(buf);
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
		throwError(ALLOC_ERROR);
		free(buf);
		return NULL;
	}

	*size = strlen(data) + 1;
	free(buf);
	return data;
}

/* Function: create_shared_memory
*  Create shared memory.
*/
void* create_shared_memory(size_t size) {
    
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
}

/* Function: free_shared_memory
*  Free a given shared memory.
*/
void free_shared_memory(void* map, size_t size) {

	if (map != MAP_FAILED) munmap(map, size);	
}

/* Function: lowerCase
*  Convert a string to lower case.
*/
void lowerCase(char* s) {
	
	for (int i = 0; i < strlen(s); i++) {
		if (s[i] >= 65 && s[i] <= 90) s[i] += 32;
	}
}

/* Function: getExtension
*  Returns the extension of a given file (file command + popen).
*/
char* getExtension(char* file, char* root_path) {
	
	char* sys_cmd = "file -b ";

	char* cmd = (char*) malloc(strlen(sys_cmd) + strlen(root_path) + strlen(file) + 1);
	if (cmd == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	sprintf(cmd, "%s%s%s", sys_cmd, root_path, file);

	FILE* fp = popen(cmd, "r");
	free(cmd);
	if (fp == NULL) return NULL;

	int sz = 8;

	char* buf = (char*) malloc(1);
	if (buf == NULL) {
		throwError(ALLOC_ERROR);
		pclose(fp);
		return NULL;
	}

	char c = '\0';
	int i = 0;

	while ((c = fgetc(fp)) != ' ' && c != '\n' &&  c != EOF) {
		if (i >= 64) {
			free(buf);
			pclose(fp);
			return NULL;
		} else if (i % sz == 0) {
			buf = (char*) realloc(buf, i + sz);
			if (buf == NULL) {
				throwError(ALLOC_ERROR);
		        pclose(fp);
		 		return NULL;
			}
		}
		buf[i++] = c;
	}

	pclose(fp);

	buf[i] = '\0';

	buf = (char*) realloc(buf, i + 1);
	if (buf == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	lowerCase(buf);
	return buf;
}