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

void songbook_dtor(Songbook *songbook);

char *make_songbook_print(Songbook *songbook);

bool decode_songbook_print(char *to_decode, Songbook *save_to);

bool remove_songbook(Songbook *songbook);

bool songbook_tex_init(Songbook *songbook);

bool tex_add_maintex(Path *tex_path, char *songbook_name);

bool copy_file(Path *copy_from, Path *copy_to);

#endif