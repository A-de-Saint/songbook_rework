#ifndef HTML_MANAGER_H
#define HTML_MANAGER_H

#include "pathwork.h"
#include "songbook_manager.h"
#include "song_manager.h"
#include <stdbool.h>

bool add_song_html(Songbook *songbook, Song *song);

bool html_add_song(Path *html_path, Song *song, char *song_print, Type type);

bool get_horizontal_fontsize_split(SongData *data, Type type, float *max_size);

FILE *open_temp_init(Path *temp_path, Path *template_path, FILE *template_f);

bool close_temp_end(FILE *temp, FILE *template);

void reset_temp(Path *temp_path);

int build_toc_html(FILE *toc, FILE *songlist);

#endif