#ifndef MENU_H
#define MENU_H

#include "pathwork.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    exit_program,
    new_sb,
    edit_sb,
    delete_sb
} ActionChoice;

int readnum(int min, int max);

ActionChoice action_choice();

bool new_songbook(Path *home_path);

#endif