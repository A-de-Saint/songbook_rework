#ifndef MENU_H
#define MENU_H

#include "pathwork.h"
#include <stdio.h>
#include <stdbool.h>
#include "songbook_manager.h"
#include "song_manager.h"

#ifndef _WIN32
    #define COLOR_RED "\x1b[31m"
    #define COLOR_GREEN "\x1b[32m"
    #define COLOR_YELLOW "\x1b[33m"
    #define COLOR_CYAN "\x1b[36m"
    #define COLOR_RESET "\x1b[0m"
#else
    #define COLOR_RED 
    #define COLOR_GREEN 
    #define COLOR_YELLOW 
    #define COLOR_CYAN 
    #define COLOR_RESET 
#endif

typedef enum {
    exit_program,
    new_sb,
    edit_sb,
    delete_sb,
    add_transpose
} ActionChoice;

typedef enum {
    go_back,
    add_song,
    add_multiple,
    remove_song
} EditSBChoice;

int readnum(int min, int max);

ActionChoice action_choice();

bool new_songbook(Path *home_path);

bool choose_songbook(Path *home_path, Songbook *save_to);

int deletion_choice(Path *home_path, Songbook *save_to);

bool get_song(Path *home_path, Song *save_to);

EditSBChoice edit_choice();

#endif