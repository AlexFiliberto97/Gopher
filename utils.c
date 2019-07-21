#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#ifndef __linux__
	#include "win32/utils_win32.h"
#else
	#include <syslog.h>
	#include "posix/utils_posix.h"
#endif

struct Dict {
	
	char **keys;
	char **values;
	int length;
	int err;
};

void freeList(char **list, int count) {
	
	for (int i = 0; i < count; i++) {
		free(list[i]);
	}
	free(list);
}

int getFileSize(FILE *fp) {
	
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	rewind(fp);
	return sz;
}

char* slice(char *text, int lo, int hi) {

	char* sliced = (char *) malloc(hi - lo + 1);
	if (sliced == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	for (int i = 0; i < hi - lo; i++) {
		sliced[i] = text[i+lo];
	}
	sliced[hi-lo] = '\0';
	return sliced;
}

int lastOccurrence(char* s, char c, int end_offset) {
	
	for (int i = strlen(s) - 1 + end_offset; i >= 0; i--) {
		if (s[i] == c) {
			return i;
		}
	}
	return -1;
}

int countSplitChar(char *text, char delimiter) {
	
	int count = 0;
	for (int i = 0; i < strlen(text); i++) {
		if (text[i] == delimiter) {
			if (i == 0 || i == strlen(text) - 1) continue;
			count++;
		}
	}
	return count + 1;
}

char** split(char* text, char delimiter, int* c) {

	int* pos;
	int count = countSplitChar(text, delimiter);
	*c = count;
	char** list = (char**) malloc(sizeof(char*) * count);
	if (list == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	char* tmp;
	int tmp_count = 0;
	int x = 0;
	for (int i = 0; i < strlen(text); i++) {

		if (i == 0 && text[i] == delimiter) continue;
		if (i == strlen(text) - 1 && text[i] != delimiter) {
			list[x++] = slice(text, i - tmp_count, i+1);
			if (list[x-1] == NULL) {
				freeList(list, i);
				return NULL;
			}
		}

		if (text[i] == delimiter) {
			list[x++] = slice(text, i - tmp_count, i);
			if (list[x-1] == NULL) {
				freeList(list, i);
				return NULL;
			}
			tmp_count = 0;
		} else {
			tmp_count++;
		}	
	}
	return list;
}

char** readlines(char* path, int* count) {

	size_t size;
	char *text = readFile(path, &size);
	if (text == NULL) {
		throwError(1, READ_FILE_ERROR);
		return NULL;
	}
	
	int n;
	char **lines = split(text, '\n', &n);
	if (lines == NULL) {
		free(text);
		return NULL;
	}
	
	*count = n;
	free(text);
	return lines;
}

char* getExtension(char* filename) {
	
	char* ext;
	if (filename[strlen(filename) - 1] == '/') {
		ext = (char*)  malloc(5);
		if (ext == NULL) {
			throwError(1, ALLOC_ERROR);
			return NULL;
		}
		strcpy(ext, "menu");
		return ext;
	}

	int lastDot = lastOccurrence(filename, '.', 0);
	if (lastDot < 0) return NULL;

	ext = slice(filename, lastDot + 1, strlen(filename));
	if(ext == NULL) return NULL;

	return ext;
}

char* concatList(char** list, u_int count, char separator) {
	
	char*s = (char*) malloc(1);
	if (s == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	s[0] = '\0';
	for (int i = 0; i < count; i++) {
		s = (char *) realloc(s, strlen(s) + strlen(list[i]) + 2);
		if (s == NULL) {
			throwError(1, ALLOC_ERROR);
			return NULL;
		}	
		sprintf(s, "%s%s%c", s, list[i], separator);
	}
	return s;
}

struct Dict buildDict(char** list, int length) {

	struct Dict dict = {NULL, NULL, 0, 0};
	char** keys = (char**) malloc(sizeof(char*) * length);
	char** values = (char**) malloc(sizeof(char*) * length);
	if (keys == NULL || values == NULL) {
		if (keys == NULL) free(keys);
		if (keys == NULL) free(values);
		throwError(1, ALLOC_ERROR);
		dict.err = 1;
		return dict;
	}
	
	int tmp_len;
	for (int i = 0; i < length; i++) {
		char** line = split(list[i], '|', &tmp_len);
		if (line == NULL || line[0] == NULL || strlen(line[0]) == 0) {
			freeList(keys, length);
			freeList(values, length);
			dict.err = 1;
			return dict;
		
		} else if (tmp_len == 1) {
			keys[i] = line[0];
			values[i] = (char*) malloc(1);
			if (values[i] == NULL) {
				throwError(1, ALLOC_ERROR);
				freeList(keys, length);
				freeList(values, length);
				dict.err = 1;
			}
			
			values[i][0] = '\0';
		} else {
			keys[i] = line[0];
			values[i] = line[1];
		}
		free(line);
	}
	dict.keys = keys;
	dict.values = values;
	dict.length = length;
	return dict;
}

char* getAssocValue(char* key, struct Dict dict) {
	
	for (int i = 0; i < dict.length; i++) {
		if (strcmp(key, dict.keys[i]) == 0) {
			return dict.values[i];
		}
	}
	return NULL;
}

void freeDict(struct Dict dict) {
	
	freeList(dict.keys, dict.length);
	freeList(dict.values, dict.length);
}

char* fixPath(char* path) {
	
	char* fixed_path = (char*) malloc(strlen(path) + 1);
	if (fixed_path == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	int c = 0;
	for (int i = 0; i <= strlen(path); i++) {
		if (path[i] == '/' && path[i+1] == '/') continue;
		fixed_path[c++] = path[i];
	}
	fixed_path = (char*) realloc(fixed_path, strlen(fixed_path) + 1);
	if (fixed_path == NULL) {
		throwError(1, ALLOC_ERROR);
		return NULL;
	}

	return fixed_path;
}

int searchList(char* item, char** list, int length) {
	
	for (int i = 0; i < length; i++) {
		if (strcmp(item, list[i]) == 0) {
			return i;
		}
	}
	return -1;
}

int isNumeric(char* str) {
	
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= 48 && str[i] < 58)) {
			return 0;
		}
	}
	return -1;
}

void log_output(char* format, int n) {
    
    printf(format, n);
    #ifdef __linux__
        syslog(LOG_INFO, format, n);
    #endif
}

int checkMalloc(char** list, int c) {

	for (int i = 0; i < c; i++) {
		if (list[i] == NULL) {
			return ALLOC_ERROR;
		}
	}
	return 0;
}