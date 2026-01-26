#include "songbook_manager.h"
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

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
    bool res = read_insert_write(list_path, songbook->name);
    path_dtor(list_path);
    if (!res)
        return false;
    
    return true;
}