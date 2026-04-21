#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include <stdbool.h>
#include "song_manager.h"

bool transpose_song(Song *song, char *transpose_to);

bool get_trans_idx(char *first_chord, char *transpose_to, int *trans_idx);

void put_chord(char *repl_line, int *cursor, char *main_part, char *rest);

bool transpose_chord(char *chord, int trans_idx);

static inline void first_to_upper(char *str)
{
    if (str[0] >= 'a' && str[0] <= 'z')
        str[0] -= 32;
}

void normalize_chord(char *chord);

bool get_first_chord(Song *song, char first_chord[3]);

#endif