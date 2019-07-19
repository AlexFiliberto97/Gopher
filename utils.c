#include <stdio.h>
#include <stdlib.h>

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


/* Function: getFileSize
* -------------------------------
*  
*/
int getFileSize(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	rewind(fp);
	return sz;
}


/* Function: readFile
* -------------------------------
*  
*/
// char *readFile(char *path, size_t *size) {

// 	FIL *fp;E
// 	fp = fopen(path, "rb");

// 	if (fp == NULL) {
// 		printf("file non valido");
// 		return NULL;
// 	}

// 	size_t sz = getFileSize(fp);

// 	if (sz == -1) {
// 		printf("Errore in readFile - getFileSize\n");
// 		fclose(fp);
// 		return NULL;
// 	}

// 	char *buf = (char *) malloc(sz);

// 	if (buf == NULL) {
// 		printf("Errore in readFile - malloc\n");
// 		fclose(fp);
// 		return NULL;
// 	}

// 	size_t read = fread(buf, 1, sz, fp);

// 	if (read != sz) {
// 		printf("Errore in readFile - fread\n");
// 		fclose(fp);
// 		return NULL;
// 	}

// 	*size = sz;
// 	fclose(fp);
// 	return buf;
// }


/* Function: slice
* -------------------------------
*  
*/
char* slice(char *text, int lo, int hi) {

	char* sliced = (char *) malloc(hi - lo + 1);

	if (sliced == NULL) {
		printf("Errore in slice - malloc\n");
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


/* Function: countSplitChar
* -------------------------------
*  
*/
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


/* Function: split
* -------------------------------
*  
*/
char** split(char* text, char delimiter, int* c) {

	int* pos;
	int count = countSplitChar(text, delimiter);

	*c = count;

	char** list = (char**) malloc(sizeof(char*) * count);

	if (list == NULL) {
		printf("Errore in split - malloc\n");
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
				printf("Errore in split - slice\n");
				return NULL;
			}

		}

		if (text[i] == delimiter) {
			list[x++] = slice(text, i - tmp_count, i);
			if (list[x-1] == NULL) {
				printf("Errore in split - slice\n");
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
* -------------------------------
*  
*/
char** readlines(char* path, int* count) {
	#ifndef __linux__
		char* text = readFile(path);
	#else
		size_t size;
		char* text = readFile(path, &size);
	#endif
	if (text == NULL) {
		printf("Errore in readLines - readFileWin32\n");
		return NULL;
	}
	int n;
	char **lines = split(text, '\n', &n);
	if (lines == NULL) {
		printf("Errore in readlines - split\n");
		return NULL;
	}
	*count = n;
	free(text);
	return lines;
}


/* Function: getExtension
* -------------------------------
*  
*/
char* getExtension(char* filename) {
	
	char* ext;

	if (filename[strlen(filename) - 1] == '/') {
		ext = (char*)  malloc(5);
		if (ext == NULL) {
			printf("Errore in getExtension - malloc\n");
			return NULL;
		}
		strcpy(ext, "menu");
		return ext;
	}

	int lastDot = lastOccurrence(filename, '.', 0);

	ext = slice(filename, lastDot + 1, strlen(filename));

	return ext;

}


/* Function: concatList
* -------------------------------
*  
*/
char* concatList(char** list, u_int count, char separator) {
	char* s = (char*) malloc(1);
	if (s == NULL) {
		printf("Errore in concatList - malloc\n");
		return NULL;
	}	
	s[0] = '\0';

	for (int i = 0; i < count; i++) {
		s = (char *) realloc(s, strlen(s) + strlen(list[i]) + 2);
		if (s == NULL) {
			printf("Errore in concatList - realloc\n");
			return NULL;
		}	
		strcat(s, list[i]);
		char tmp[2] = {separator, '\0'};
		strcat(s, tmp);
	}
	return s;
}


/* Function: freeList
* -------------------------------
*  
*/
void freeList(char** list, int count) {
	for (int i = 0; i < count; i++) {
		free(list[i]);
	}
	free(list);
}


/* Function: printStringList
* -------------------------------
*  
*/
void printStringList(char **list, int n) {
	for (int i = 0; i < n; i++) {
		printf("%s\n", list[i]);
	}
}


/* Function: buildDict
* -------------------------------
*  
*/
struct Dict buildDict(char** list, int length) {

	struct Dict dict = {NULL, NULL, 0, 0};
	
	char** keys = (char**) malloc(sizeof(char*) * length);
	char** values = (char**) malloc(sizeof(char*) * length);
	if (keys == NULL || values == NULL) {
		printf("Errore in buildDict - malloc\n");
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


/* Function: getAssocValue
* -------------------------------
*  
*/
char* getAssocValue(char* key, struct Dict dict) {
	for (int i = 0; i < dict.length; i++) {
		if (strcmp(key, dict.keys[i]) == 0) {
			return dict.values[i];
		}
	}
	return NULL;
}


/* Function: freeDict
* -------------------------------
*  
*/
void freeDict(struct Dict dict) {
	freeList(dict.keys, dict.length);
	freeList(dict.values, dict.length);
}


/* Function: fixPath
* -------------------------------
*  
*/
char* fixPath(char* path) {
	char* fixed_path = (char*) malloc(strlen(path) + 1);
	if (fixed_path == NULL) {
		printf("Errore in fixPath - malloc\n");
		return NULL;
	}
	int c = 0;
	for (int i = 0; i <= strlen(path); i++) {
		if (path[i] == '/' && path[i+1] == '/') continue;
		fixed_path[c++] = path[i];
	}
	fixed_path = (char*) realloc(fixed_path, strlen(fixed_path) + 1);
	free(path);
	return fixed_path;
}


/* Function: searchList
* -------------------------------
*  
*/
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
	return 1;
}


void log_output(char* format, int n) {
    printf(format, n);
    #ifdef __linux__
        syslog(LOG_INFO, format, n);
    #endif
}


int checkMalloc (char** list, int c) {

	for(int i = 0; i < c; i++) {
		if(list[i] == NULL) {
			return -1;
		}
	}
	return 0;
}