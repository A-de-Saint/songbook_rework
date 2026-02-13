#ifndef PATHWORK_H
#define PATHWORK_H

#define PATH_START_CAPACITY 30
#include <stdbool.h>

#ifdef _WIN32
    #define DIFF_CHAR '\\'
#else
    #define DIFF_CHAR '/'
#endif

typedef struct {
    char *path;
    unsigned int length;
    unsigned int capacity;
} Path;

Path *path_ctor(char *start_path);

bool path_resize(Path *path);

void path_dtor(Path *path);

bool path_add(Path *curr_path, char *to_add, char mode);

Path *path_copy(Path *to_copy, bool reduce);

void path_dirback(Path *path);

#endif