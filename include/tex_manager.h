#ifndef TEX_MANAGER_H
#define TEX_MANAGER_H

#include "songbook_manager.h"
#include "song_manager.h"
#include <stdbool.h>

bool add_song_tex(Songbook *songbook, Song *song);

bool create_tex_song(Song *song, char *song_print, Path *tex_path, Type type);

int get_font_size_split(SongData *song_data, Type type, int values[2]);

bool add_song_to_main(Path *tex_path, char *song_print);

unsigned int utf8_char_length(const unsigned char *s);

#endif