#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "help.h"
#include "menu.h"

void songbooks_help(Path *help_path, size_t size, char buff[size]);
void song_format_help(Path *help_path, size_t size, char buff[size]);
void song_add_help(Path *help_path, size_t size, char buff[size]);
void song_trans_help(Path *help_path, size_t size, char buff[size]);
bool read_help(size_t size, char buff[size], char *filename, Path *help_path);
void songbooks_remove_help(Path *help_path, size_t size, char buff[size]);

//opens interactive help menu and takes the user from there
void help_menu(Path *home_path)
{
    printf(COLOR_CYAN"\nWhat do you need help with?\n\n"COLOR_RESET);

    printf("[0] Nothing, exit help menu\n");
    printf("[1] Songbooks and their finalised versions\n");
    printf("[2] Song format, manual edits\n");
    printf("[3] Adding songs\n");
    printf("[4] Song transposition\n");
    printf("[5] Removing songbooks\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 5)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);
    printf(COLOR_CYAN"\nWhat would you like to know?\n\n"COLOR_RESET);
    Path *help_path = path_copy(home_path, false);
    if (!path_add(help_path, ".help_texts", 'd'))
        return;

    char buff[4096];
    buff[0] = '\0';
    switch (choice)
    {
        case 0:
            path_dtor(help_path);
            return;
        case 1:
            songbooks_help(help_path, sizeof(buff), buff);
            break;
        case 2:
            song_format_help(help_path, sizeof(buff), buff);
            break;
        case 3:
            song_add_help(help_path, sizeof(buff), buff);
            break;
        case 4:
            song_trans_help(help_path, sizeof(buff), buff);
            break;
        case 5:
            songbooks_remove_help(help_path, sizeof(buff), buff);
            break;
    }

    path_dtor(help_path);

    if (buff[0] == '\0')
        return;

    printf("\n%s\n", buff);
    
    printf("[0] Exit help menu...\n");
    while(readnum(0, 0) == -1);
}

void songbooks_help(Path *help_path, size_t size, char buff[size])
{
    printf("[0] Nothing, exit help menu\n");
    printf("[1] Where is my songbook located?\n");
    printf("[2] How do I use a songbook's queue?\n");
    printf("[3] How does my TEX songbook work?\n");
    printf("[4] Why can't I change my TEX songboook's font?\n");
    printf("[5] How does my HTML songbook work?\n");
    printf("[6] How can I edit my HTML songbook (for example font)?\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 6)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);
    
    switch(choice)
    {
        case 0:
            return;
        case 1:
            if (!read_help(size, buff, "sb_1.txt", help_path))
                return;
            break;
        case 2:
            if (!read_help(size, buff, "q_help.txt", help_path))
                return;
            break;
        case 3:
            if (!read_help(size, buff, "sb_3.txt", help_path))
                return;
            break;
        case 4:
            if (!read_help(size, buff, "sb_4.txt", help_path))
                return;
            break;
        case 5:
            if (!read_help(size, buff, "sb_5.txt", help_path))
                return;
            break;
        case 6:
            if (!read_help(size, buff, "sb_6.txt", help_path))
                return;
            break;
        default:
            return;
    }
}

void song_format_help(Path *help_path, size_t size, char buff[size])
{
    printf("[0] Nothing, exit help menu\n");
    printf("[1] How are songs saved locally?\n");
    printf("[2] Can I edit a song in songcollection?\n");
    printf("[3] Can I add my own song manually?\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 3)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);

    switch(choice)
    {
        case 0:
            return;
        case 1:
            if (!read_help(size, buff, "sf_1.txt", help_path))
                return;
            break;
        case 2:
            if (!read_help(size, buff, "sf_2.txt", help_path))
                return;
            break;
        case 3:
            if (!read_help(size, buff, "sf_3.txt", help_path))
                return;
            break;
        default:
            return;
    }
}

void song_add_help(Path *help_path, size_t size, char buff[size])
{
    printf("[0] Nothing, exit help menu\n");
    printf("[1] How does adding a song work?\n");
    printf("[2] Can I manually add my own song?\n");
    printf("[3] I can't add a song, what should I do?\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 3)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);

    switch(choice)
    {
        case 0:
            return;
        case 1:
            if (!read_help(size, buff, "sa_1.txt", help_path))
                return;
            break;
        case 2:
            if (!read_help(size, buff, "sf_3.txt", help_path))
                return;
            break;
        case 3:
            if (!read_help(size, buff, "sa_3.txt", help_path))
                return;
            break;
        default:
            return;
    }
}

void song_trans_help(Path *help_path, size_t size, char buff[size])
{
    printf("[0] Nothing, exit help menu\n");
    printf("[1] How does song transposition work?\n");
    printf("[2] How can I transpose a song that is already added in a songbook?\n");
    printf("[3] Transposition is failing, what should I do?\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 3)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);

    switch(choice)
    {
        case 0:
            return;
        case 1:
            if (!read_help(size, buff, "st_1.txt", help_path))
                return;
            break;
        case 2:
            if (!read_help(size, buff, "st_3.txt", help_path))
                return;
            break;
        case 3:
            if (!read_help(size, buff, "st_3.txt", help_path))
                return;
            break;
        default:
            return;
    }
}

void songbooks_remove_help(Path *help_path, size_t size, char buff[size])
{
    printf("[0] Nothing, exit help menu\n");
    printf("[1] What does removing a songbook do?\n");
    printf("[2] Does removing a songbook also remove all of the songs that are in it?\n");
    printf("[3] Can I recover a removed songbook?\n");

    printf(COLOR_CYAN"Your choice: "COLOR_RESET);
    int choice = -1;
    while ((choice = readnum(0, 3)) == -1)
        printf(COLOR_RED"Invalid choice.\n"COLOR_CYAN"Try again: "COLOR_RESET);

    switch(choice)
    {
        case 0:
            return;
        case 1:
            if (!read_help(size, buff, "sbr_1.txt", help_path))
                return;
            break;
        case 2:
            if (!read_help(size, buff, "sbr_2.txt", help_path))
                return;
            break;
        case 3:
            if (!read_help(size, buff, "sbr_3.txt", help_path))
                return;
            break;
        default:
            return;
    }
}

//tries to read size-1 characters from help_path/filename
//returns false if anything is wrong
bool read_help(size_t size, char buff[size], char *filename, Path *help_path)
{
    if (help_path == NULL || help_path->path == NULL)
        return false;

    if (!path_add(help_path, filename, 'f'))
        return false;

    FILE *help_file = fopen(help_path->path, "r");
    if (help_file == NULL)
    {
        fprintf(stderr, "Could not open '%s'\n", help_path->path);
        return false;
    }

    size_t res = fread(buff, 1, size - 1, help_file);

    fclose(help_file);

    buff[res] = '\0';

    if (res == 0)
    {
        fprintf(stderr, "Reading from '%s' failed\n", help_path->path);
        return false;
    }

    return true;
}
