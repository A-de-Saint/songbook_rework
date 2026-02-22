#ifndef MULTIPLE_SONGS_H
#define MULTIPLE_SONGS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pathwork.h"
#include "song_manager.h"

typedef struct {
    FILE **songs;
    char **paths;
    unsigned int count;
    unsigned int capacity;
} SongFiles;

bool song_files_ctor(SongFiles *song_files, unsigned int capacity);

bool song_files_resize(SongFiles *sf);

bool song_files_add(SongFiles *song_files, FILE *file, Path *path);

void song_files_dtor(SongFiles *song_files, bool close_files);

void reset_files(SongFiles *song_files);

#endif