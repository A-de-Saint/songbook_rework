#ifndef SONG_MANAGER_H
#define SONG_MANAGER_H

#include "util.h"
#include <stdbool.h>

typedef struct {
    StringArray lines;
    char *name_utf;
    char *name_ascii;
    char *author_utf;
    char *author_ascii;
} Song;

void song_ctor(Song *song);

void song_dtor(Song *song);

char *create_song_print(char *song_name, char *author);

bool decode_song_print(char *to_decode, char *name_save, char *author_save);

Path *song_try_find(char *song_name, char *author, Path *home_path);

Path *song_try_find_noauthor(char *song_name, Path *home_path);

bool song_peek(Path *song_path);

#endif