#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/error.h"
#include "../include/utils.h"
#include "../include/const.h"

#ifndef __linux__
	#include "../../win32/include/utils_win32.h"
#else
	#include "../../posix/include/utils_posix.h"
#endif

/* Function: freeList
*  Free a list of char pointers.
*/
void freeList(char** list, int count) {

	if (list == NULL) return;
    
    for (int i = 0; i < count; i++) {
        if (list[i] != NULL) free(list[i]);
    }
    free(list);
}

/* Function: cpyalloc
*  Alloc a given string and return the pointer.
*/
char* cpyalloc(char* source) {
	
	char* res = (char*) malloc(strlen(source) + 1);
	if (res == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}
	strcpy(res, source);
	return res;
}

/* Function: slice
*  Return the slice for a given string from lo to hi.
*/
char* slice(char* text, int lo, int hi) {

	char* sliced = (char*) malloc(hi - lo + 1);
	if (sliced == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	for (int i = 0; i < hi - lo; i++) {
		sliced[i] = text[i+lo];
	}
	
	sliced[hi-lo] = '\0';
	return sliced;
}

/* Function: lastOccurrence
*  Return index of c in string s (first occurrence). 
*/
int lastOccurrence(char* s, char c, int end_offset) {
	
	for (int i = strlen(s) - 1 + end_offset; i >= 0; i--) {
		if (s[i] == c) {
			return i;
		}
	}
	return -1;
}

/* Function: countSplitChar
*  Count the occurrence of selimiter in text.
*/
int countSplitChar(char* text, char delimiter) {
	
	int count = 0;
	for (int i = 0; i < strlen(text); i++) {
		if (text[i] == delimiter) {
			if (i == 0 || i == strlen(text) - 1) continue;
			count++;
		}
	}
	return count + 1;
}

/* Function: split
*  String split. 
*/
char** split(char* text, char delimiter, int* c) {

	if (strlen(text) == 0) {
		*c = 0;
		return NULL;
	}

	int count = countSplitChar(text, delimiter);
	*c = count;
	char** list = (char**) malloc(sizeof(char*) * count);
	if (list == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	int tmp_count = 0, x = 0;
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

/* Function: readlines
*  Read a given file.
*/
char** readlines(char* path, int* count) {

	int size;
	char* text = readFile(path, &size);
	if (text == NULL) {
		throwError(READ_FILE_ERROR);
		return NULL;
	}
	
	int n;
	char** lines = split(text, '\n', &n);
	if (lines == NULL) {
		free(text);
		return NULL;
	}

	*count = n;
	free(text);
	return lines;
}

/* Function: concatList
*  Concatenate a list of strings.
*/
char* concatList(char** list, int count, char separator) {

	char* s = (char*) malloc(1);
    if (s == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	s[0] = '\0';
	for (int i = 0; i < count; i++) {
		s = (char *) realloc(s, strlen(s) + strlen(list[i]) + 2);
		if (s == NULL) {
			throwError(ALLOC_ERROR);
			return NULL;
		}	
		strcat(s, list[i]);
		char tmp[2] = {separator, '\0'};
		strcat(s, tmp);
	}
	return s;
}

/* Function: buildDict
*  Build a key:value dictionary from given list of strings. 
*/
Dict buildDict(char** list, int length) {

	Dict dict = {NULL, NULL, 0, 0};
	char** keys = (char**) malloc(sizeof(char*) * length);
	char** values = (char**) malloc(sizeof(char*) * length);
	if (keys == NULL || values == NULL) {
		if (keys != NULL) free(keys);
		if (values != NULL) free(values);
		throwError(ALLOC_ERROR);
		dict.err = 1;
		return dict;
	}
	
	int tmp_len;
	int c = 0;
	for (int i = 0; i < length; i++) {
		if (list[i][0] == '\0') continue;
		char** line = split(list[i], '|', &tmp_len);
		if (tmp_len > 2) {
			for (int i = 0; i < tmp_len; i++) free(line[i]);
			free(line);
			continue;
		}
		if (line == NULL || line[0] == NULL || strlen(line[0]) == 0) {
			freeList(keys, length);
			freeList(values, length);
			dict.err = 1;
			return dict;
		
		} else if (tmp_len == 1) {
			keys[c] = line[0];
			values[c] = (char*) malloc(1);
			if (values[c] == NULL) {
				throwError(ALLOC_ERROR);
				freeList(keys, length);
				freeList(values, length);
				dict.err = 1;
			}
			
			values[c][0] = '\0';
		} else {
			keys[c] = line[0];
			values[c] = line[1];
		}
		free(line);
		c++;
	}

	keys = (char**) realloc(keys, sizeof(char*) * c);
	values = (char**) realloc(values, sizeof(char*) * c);
	if (keys == NULL || values == NULL) {
		if (keys != NULL) freeList(keys, c);
		if (values != NULL) freeList(values, c);
		dict.err = 1;
		return dict;
	}

	dict.keys = keys;
	dict.values = values;
	dict.length = c;
	return dict;
}

/* Function: getAssocValue
*  Return dick[key] value.
*/
char* getAssocValue(char* key, Dict dict) {
	
	for (int i = 0; i < dict.length; i++) {
		if (strcmp(key, dict.keys[i]) == 0) {
			return dict.values[i];
		}
	}
	return NULL;
}

/* Function: freeDict
*  Free a Dict struct
*/
void freeDict(Dict dict) {
	
	freeList(dict.keys, dict.length);
	freeList(dict.values, dict.length);
}

/* Function: fixPath
*  Fix a given path (if wrong).
*/
char* fixPath(char* path) {
	
	char* fixed_path = (char*) malloc(strlen(path) + 1);
	if (fixed_path == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	int c = 0;
	for (int i = 0; i <= strlen(path); i++) {
		if (path[i] == '/' && path[i+1] == '/') continue;
		fixed_path[c++] = path[i];
	}

	fixed_path = (char*) realloc(fixed_path, strlen(fixed_path) + 1);
	if (fixed_path == NULL) {
		throwError(ALLOC_ERROR);
		return NULL;
	}

	free(path);
	return fixed_path;
}

/* Function: searchList
*  Search an item in a list of items.
*/
int searchList(char* item, char** list, int length) {
	
	for (int i = 0; i < length; i++) {
		if (strcmp(item, list[i]) == 0) {
			return i;
		}
	}
	return -1;
}

/* Function: isNumeric
*  Check if a string is numeric.
*/
int isNumeric(char* str) {
	
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= 48 && str[i] < 58)) {
			return -1;
		}
	}
	return 0;
}

/* Function: checkMalloc
*  Check if a list of string is allocated with no errors.
*/
int checkMalloc(char** list, int c) {

	for (int i = 0; i < c; i++) {
		if (list[i] == NULL) {
			return ALLOC_ERROR;
		}
	}
	return 0;
}

/* Function: hash_item
*  Calculate the hash value for given string.
*/
long long hash_item(char* item) {

    int p = 131;
    int m = 1e9 + 9;
    long long hash_value = 0;
    long long p_pow = 1;

    for (int i = 0; i < strlen(item); i++) {
        hash_value = (hash_value + (item[i] + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }

	return hash_value;
}

/* Function: isValidPath
*  Check if a path is valid.
*/
int isValidPath(char* path) {
	
	for (int i = 1; i < strlen(path); i++) {
		if (path[i-1] == '.' && path[i] == '.') {
			return 0;
		}
	}
	return 1;
}

/* Function: writeErr
*  Write on error log (NB: return values are ignored).
*/
void writeErr(char* msg, int err) {

	FILE* fp = fopen(ERROR_LOG_PATH, "a+b");
	if (fp ==  NULL) return;

	if (err == 0) {
		int written = fwrite(msg, 1, strlen(msg), fp);
		if (written < 0) {
			pclose(fp);
			return;
		}
	} else {
		char* msgn = (char*) malloc(strlen(msg) +INTSTR +ERR_BOUND);
		if (msgn == NULL) {
			pclose(fp);
			return;
		}
		
		sprintf(msgn, "%s, error: %d", msg, err);
		int written = fwrite(msgn, 1, strlen(msgn), fp);
		if (written < 0) {
			pclose(fp);
			return;
		}
	}
    pclose(fp);
}

/* Function: checkConfigFile
*  Checkthe config file.
*/
int checkConfigFile() {

	size_t sz = getFileSize(CONFIG_FILE_PATH);
	if (sz < 0 || sz > MAX_CONFIG_FILE_SZ) return sz;

	FILE* fp = fopen(CONFIG_FILE_PATH, "rb");
	if (fp == NULL) return OPEN_FILE_ERROR;

	char* buf = malloc(sz + 1);
	if (buf == NULL) {
		fclose(fp);
		return ALLOC_ERROR;
	}

	size_t read = fread(buf, 1, sz, fp);
	if (read != sz) {
		fclose(fp);
		free(buf);
		return READ_FILE_ERROR;
	}

	fclose(fp);
	buf[sz] = '\0';

	for (int i = 0; i < sz; i++) {
		if (buf[i] < 0 || buf[i] > 127) {
			free(buf);
			return BAD_CONFIG_FILE;
		}
	}

	return 0;
}