#ifndef SONGBOOK_MANAGER_H
#define SONGBOOK_MANAGER_H

#include "pathwork.h"
#include <stdbool.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
#endif

typedef enum {
    type1 = 1,
    type2
} Type;

typedef enum {
    tex = 1,
    HTML,
    docx
} Format;

typedef struct {
    Path *path;
    char *name;
    Type type;
    Format format;
} Songbook;

bool create_songbook(Songbook *songbook);

#endif