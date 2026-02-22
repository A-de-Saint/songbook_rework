#ifndef SONG_MANAGER_H
#define SONG_MANAGER_H

#include "util.h"
#include <stdbool.h>

typedef enum {
    VERSE,
    CHORUS,
    INTRO,
    INTERLUDE,
    ENDING,
    BRIDGE,
    UNKNOWN
} PartState;

typedef struct {
    StringArray lines;
    PartState type;
} SongPart;

typedef struct {
    SongPart *parts;
    unsigned int count;
    unsigned int capacity;
} SongData;

typedef struct {
    SongData data;
    char *name_utf;
    char *name_ascii;
    char *author_utf;
    char *author_ascii;
} Song;

bool song_part_ctor(SongPart *song_part);

void song_part_dtor(SongPart *song_part);

bool song_data_ctor(SongData *song_data);

bool song_data_add(SongData *song_data, SongPart song_part);

bool song_data_resize(SongData *song_data);

void song_data_dtor(SongData *song_data);

void song_ctor(Song *song);

void song_dtor(Song *song);

char *create_song_print(char *song_name, char *author);

char *create_song_print_extended(char *name_ascii, char *author_ascii, char *name_utf, char *author_utf);

bool decode_song_print(char *to_decode, char **name_save, char **author_save);

bool decode_song_print_extended(char *to_decode, char **song_print, char **name_utf, char **author_utf);

Path *song_try_find(char *song_name, char *author, Path *home_path);

Path *song_try_find_noauthor(char *song_name, Path *home_path);

bool song_peek(Path *song_path);

bool download_song_ak(Song *song, bool noauthor);

void url_addnum(char *url, int last_char, int num);

bool add_song_songcollection(Path *home_path, Song *song);

bool decode_song(Path *path, Song *song);

bool add_song_songlist(char *author_ascii, char *name_ascii, Path *songbook_path);

bool add_song_songlist_extended(char *name_ascii, char *author_ascii, char *name_utf, char *author_utf, Path *songbook_path);

#endif