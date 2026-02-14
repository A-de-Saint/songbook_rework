#ifndef MENU_H
#define MENU_H

#include "pathwork.h"
#include <stdio.h>
#include <stdbool.h>
#include "songbook_manager.h"
#include "song_manager.h"

typedef enum {
    exit_program,
    new_sb,
    edit_sb,
    delete_sb
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