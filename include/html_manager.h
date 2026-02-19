#ifndef HTML_MANAGER_H
#define HTML_MANAGER_H

#include "pathwork.h"
#include "songbook_manager.h"
#include "song_manager.h"
#include <stdbool.h>

bool add_song_html(Songbook *songbook, Song *song);

bool html_add_song(Path *html_path, Song *song, char *song_print, Type type);

bool get_horizontal_fontsize_split(SongData *data, Type type, float *max_size);

#endif