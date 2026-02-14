#ifndef QUICK_READER_H
#define QUICK_READER_H

#include "songbook_manager.h"
#include "song_manager.h"
#include "pathwork.h"
#include <stdbool.h>
#include "input_reader.h"
#include "util.h"

typedef enum {
    AUTHOR,
    NAME
} ComesFirst;

int get_multiple_songs(Path *home_path, Songbook *songbook, StringArray *unsucessful);

char *parse_format(FILE *file);

bool parse_name_author(char *string, char *format, ComesFirst first, char *name, char *author);

void asciize_separator(char *string);

#endif