#include <stdio.h>
#include <stdlib.h>
#include "error.h"

#ifndef __linux__
	#include "win32/utils_win32.h"
#else
	#include "posix/utils_posix.h"
#endif


//Global structures
struct FilePath {

	char* path;
	char* name;
};

struct Dict {

	char** keys;
	char** values;
	int length;
	int err;
};


//Return the size of a file pointer
size_t getFileSize(FILE *fp) {
	
	int err = fseek(fp, 0L, SEEK_END);
	if (err == -1) return GRAB_SIZE;
	
	int sz = ftell(fp);
	if (sz == -1) {
		rewind(fp);
		return GRAB_SIZE;
	}
	rewind(fp);
	return sz;
}

//Index slicing on a text
char* slice(char *text, int lo, int hi) {

	char *sliced = (char *) malloc(hi - lo + 1);
	if (sliced == NULL) return NULL;

	for (int i = 0; i < hi - lo; i++) {
		sliced[i] = text[i+lo];
	}
	sliced[hi-lo] = '\0';
	return sliced;
}

//Count the number of delimiter in text
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

//String split function
char** split(char* text, char delimiter, int* c) {

	int* pos;
	int count = countSplitChar(text, delimiter);
	*c = count;
	char** list = (char**) malloc(sizeof(char*) * count);
	if (list == NULL) return NULL;
	
	char* tmp;
	int tmp_count = 0, x = 0;
	for (int i = 0; i < strlen(text); i++) {

		if (i == 0 && text[i] == delimiter) continue;
		if (i == strlen(text) - 1 && text[i] != delimiter) {
			list[x++] = slice(text, i - tmp_count, i+1);
			if (list[x-1] == NULL) return NULL;
		}

		if (text[i] == delimiter) {
			list[x++] = slice(text, i - tmp_count, i);
			if (list[x-1] == NULL) return NULL;
			tmp_count = 0;
		} else {
			tmp_count++;
		}
	}
	return list;
}

//Get the full path from a file path structure
char* getFullPath(struct FilePath fp) {

	char *fullPath = (char *) malloc(strlen(fp.path) + strlen(fp.name) + 1);
	if (fullPath == NULL) return NULL;
	
	sprintf(fullPath, "%s%s", fp.path, fp.name);
	return fullPath;
}

//Return the full path without the root
char* getFullPathNoRoot(struct FilePath fp) {

	char* fullPath = getFullPath(fp);
	if (fullPath == NULL) return NULL;
	
	char* fullPathNoRoot = memchr(fullPath, '/', strlen(fullPath));
	if (fullPathNoRoot == NULL) return NULL;
	
	fullPathNoRoot = &fullPathNoRoot[1];
	char* fullPathRes = (char*) malloc(strlen(fullPathNoRoot) + 1);
	if (fullPathRes == NULL) return NULL;

	strcpy(fullPathRes, fullPathNoRoot);
	free(fullPath);
	return fullPathRes;
}

//return a list of file lines (text)
char** readlines(char* path, int* count) {
	char* text = readFile(path);
	if (text == NULL) return NULL;

	int n;
	char** lines = split(text, '\n', &n);
	if (lines == NULL) return NULL;

	*count = n;
	return lines;
}

//Return the file extension
char *getExtension(char* filepath) {
	
	char *ext;
	if (filepath[strlen(filepath) - 1] == '/') {
		ext = (char *)  malloc(5 * sizeof(char));
		if (ext == NULL) return NULL;

		strcpy(ext, "menu");
		return ext;
	}

	int count;
	char** list = split(filepath, '.', &count);
	if (list == NULL) return NULL;

	if (count == 1) {
		ext = (char *) malloc(1);
		if (ext == NULL) return NULL;
		ext[0] = '\0';
	} else {
		ext = list[count-1];
	}
	return ext;
}

//Concat a list of strings
char *concatList(char **list, u_int count, char separator) {
	char *s = (char *) malloc(sizeof(char));
	if (s == NULL) return NULL;

	s[0] = '\0';
	for (int i = 0; i < count; i++) {
		s = (char *) realloc(s, sizeof(char) * (strlen(s) + strlen(list[i]) + 2));
		if (s == NULL) return NULL;

		sprintf(s, "%s%s%c", s, list[i], separator);
	}
	return s;
}

//Free a list of strings
void freeList(char **list, int count) {
	
	for (int i = 0; i < count; i++) {
		if (list[i] != NULL) {
			free(list[i]);
		}
	}
	free(list);
}

//Print a list of string   ///REMOVE THIS
void printStringList(char **list, int n) {
	for (int i = 0; i < n; i++) {
		printf("%s\n", list[i]);
	}
}

////REMOVE THIS
void printStringListDebug(char **list, int n) {
	for (int i = 0; i < n; i++) {
		printf("%s", list[i]);
		Sleep(500);
	}
}

//Malloc and copy a string
char* cpyalloc(char* s) {

	char* res = (char*) malloc(sizeof(char) * (strlen(s) + 1));
	if (res == NULL) return NULL;

	strcpy(res, s);
	return res;
}

//Build a struct of type Dict
struct Dict buildDict(char** list, int length) {

	struct Dict dict = {NULL, NULL, 0, 0};
	
	char** keys = (char**) malloc(sizeof(char*) * length);
	char** values = (char**) malloc(sizeof(char*) * length);
	if (keys == NULL || values == NULL) {
		dict.err = 1;
		return dict;
	}

	int tmp_len;
	for (int i = 0; i < length; i++) {
		
		char** line = split(list[i], ':', &tmp_len);
		if(line == NULL) {

			freeList(keys, i);
			freeList(values, i);
			//return NULL;
		} 
		keys[i] = line[0];
		values[i] = line[1];
	}
	dict.keys = keys;
	dict.values = values;
	dict.length = length;
	return dict;
}

//Get the associated value
char* getAssocValue(char* key, struct Dict dict) {
	
	for (int i = 0; i < dict.length; i++) {
		if (strcmp(key, dict.keys[i]) == 0) {
			return dict.values[i];
		}
	}
	return NULL;
}

//Free a struct of type Dict
void freeDict(struct Dict dict) {
	
	freeList(dict.keys, dict.length);
	freeList(dict.values, dict.length);
}

//Return the fixed path
char* fixPath(char* path) {
	
	char *fixed_path = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	if (fixed_path == NULL) return NULL;

	int c = 0;
	for (int i = 0; i <= strlen(path); i++) {
		if (path[i] == '/' && path[i+1] == '/') continue;
		fixed_path[c++] = path[i];
	}
	fixed_path = (char *) realloc(fixed_path, sizeof(char) * (strlen(fixed_path) + 1));
	if (fixed_path == NULL) return NULL; 
	return fixed_path;
}

//Search an element in a list
int searchList(char* item, char** list, int length) {
	for (int i = 0; i < length; i++) {
		if (strcmp(item, list[i]) == 0) {
			return i;
		}
	}
	return -1;
}

//Check if a string is numeric
int isNumeric(char* str) {
	
	for (int i = 0; i < strlen(str); i++) {
		if (!(str[i] >= 48 && str[i] < 58)) {
			return -1;
		}
	}
	return 0;
}

//Chech if a list is allocated 
int checkMalloc(char** cmd, int arg_i) {

	for (int i = 0; i < arg_i; i++) {
		if (cmd[i] == NULL) {
			return ALLOC_ERROR;
		}
	}
	return 0;
}