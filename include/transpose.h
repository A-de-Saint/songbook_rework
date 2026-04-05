#ifndef TRANSPOSE_H
#define TRANSPOSE_H

#include <stdbool.h>
#include "song_manager.h"

static int chart_size = 12;
static char *trans_chart[] = {"A", "A#", "H", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

bool transpose_song(Song *song, char *transpose_to);

bool get_trans_idx(char *first_chord, char *transpose_to, int *trans_idx);

void put_chord(char *repl_line, int *cursor, char *main_part, char *rest);

bool transpose_chord(char *chord, int trans_idx);

#endif