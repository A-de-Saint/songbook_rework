#ifndef AK_PARSER_H
#define AK_PARSER_H

#include "song_manager.h"
#include <stdbool.h>
#include "input_reader.h"

int s_read_until(char *string, char *substring, char **after_substr);

bool isspace_lite(char ch);

void trim_sides(char *string);

bool parse_name_author_desc_ak(char *html, Song *song, char **description, char **read_until);

bool parse_chords_lyrics_ak(char *html, Song *song);

bool parse_part_ak(char *curr_html, char **read_until, SongPart *song_part);

void buffer_add(DynBuff *buffer, char ch);

#endif