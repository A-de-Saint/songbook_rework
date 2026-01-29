#include "songbook_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include <string.h>
#include "remover.h"

bool create_songbook(Songbook *songbook)
{
    if (songbook == NULL)
        return false;
    if (songbook->path == NULL)
        return false;

    //create directory
    if (mkdir(songbook->path->path, 0755) == -1)
        return false;

    Path *songbook_path = path_copy(songbook->path, false);

    //
    //TODO place for setting up format-specific stuff (multiple functions)
    //

    //"touch" songlist.txt (truncates)
    if (!path_add(songbook_path, "songlist.txt", 'f'))
    {
        path_dtor(songbook_path);
        return false;
    }
    FILE *songlist = fopen(songbook_path->path, "w");
    if (songlist == NULL)
    {
        path_dtor(songbook_path);
        return false;
    }
    fclose(songlist);
    path_dtor(songbook_path); //not needed anymore

    //add to songbook_list
    //list_path is songbook_path -> dirback -> +songbook_list.txt
    Path *list_path = path_copy(songbook->path, false);
    path_dirback(list_path);
    if (!path_add(list_path, "songbook_list.txt", 'f'))
    {
        path_dtor(list_path);
        return false;
    }
    char *songbook_print = make_songbook_print(songbook);
    bool res = read_insert_write(list_path, songbook_print);
    path_dtor(list_path);
    free(songbook_print);
    if (!res)
        return false;
    
    return true;
}

void songbook_dtor(Songbook *songbook)
{
    if (songbook == NULL)
        return;
    if (songbook->name != NULL)
        free(songbook->name);
    if (songbook->path != NULL)
        path_dtor(songbook->path);
}

//makes a songbook print, which is "name\type\format"
char *make_songbook_print(Songbook *songbook)
{
    if (songbook == NULL)
        return NULL;
    if (songbook->name == NULL)
        return NULL;

    //+2 for every type (in case of a two-digit number); +2 for two slashes; +1 for '\0'
    char *print = malloc(strlen(songbook->name) + 7);
    if (print == NULL)
        return NULL;
    int res = sprintf(print, "%s\\%d\\%d", songbook->name, songbook->type, songbook->format);
    if (res == -1)
    {
        free(print);
        return NULL;
    }

    return print;
}

//decodes songbook_print, but does NOT create songbook->path
bool decode_songbook_print(char *to_decode, Songbook *save_to)
{
    if (to_decode == NULL || save_to == NULL)
        return false;

    save_to->name = malloc(strlen(to_decode));
    if (save_to->name == NULL)
        return false;
    
    int type_i;
    int format_i;
    int res = sscanf(to_decode, "%[^\\]\\%d\\%d", save_to->name, &type_i, &format_i);
    if (res != 3)
    {
        fprintf(stderr, "Could not decode songbook_list.txt\n");
        free(save_to->name);
        return false;
    }
    save_to->type = (Type)type_i;
    save_to->format = (Format)format_i;

    return true;
}

bool remove_songbook(Songbook *songbook)
{
    if (songbook == NULL || songbook->path == NULL)
    {
        fprintf(stderr, "remove_songbook: NULL pointers\n");
        return false;
    }

    if (!rm_rf(songbook->path))
        return false;

    char *songbook_print = make_songbook_print(songbook);
    if (songbook_print == NULL)
        return false;

    //create path for songbook_list.txt
    Path *sblist_path = path_copy(songbook->path, false);
    if (sblist_path == NULL)
    {
        free(songbook_print);
        return false;
    }
    path_dirback(sblist_path); //now at songbooks/
    if (!path_add(sblist_path, "songbook_list.txt", 'f'))
    {
        free(songbook_print);
        path_dtor(sblist_path);
        return false;
    }
    bool res = read_remove_write(sblist_path, songbook_print);
    path_dtor(sblist_path);
    free(songbook_print);
    return res;
}