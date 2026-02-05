#include <stdio.h>
#include <stdlib.h>
#include "pathwork.h"
#include "menu.h"
#include "songbook_manager.h"
#include <curl/curl.h>
#include "song_manager.h"

int main(void)
{
    //load libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    Path *home_path = path_ctor(".");
    ActionChoice choice = action_choice();
    //TODO all possible choices
    if (choice == new_sb)
    {
        if (!new_songbook(home_path))
            printf("Failed to create specified songbook.\n");
        else 
            printf("New songbook created successfully\n");
    }
    else if (choice == edit_sb)
    {
        Songbook songbook;
        if (!choose_songbook(home_path, &songbook))
            printf("Failed to get songbook to edit.\n");
        else
            printf("Songbook chosen: %s\n", songbook.name);

        songbook_dtor(&songbook);
    }
    else if (choice == delete_sb)
    {
        Songbook songbook;
        int del_choice = deletion_choice(home_path, &songbook);
        if (del_choice == -1)
            printf("Failed to get songbook to delete\n");
        else if (del_choice == 1)
        {
            if (!remove_songbook(&songbook))
                printf("Could not delete %s\n", songbook.name);
            else 
                printf("%s successfully removed\n", songbook.name);
            songbook_dtor(&songbook);
        }
        else 
        {
            printf("%s will not be deleted.\n", songbook.name);
            songbook_dtor(&songbook);
        }   
    }

    /*Song song;
    song_ctor(&song);
    add_song(home_path, &song);
    song_dtor(&song);*/

    path_dtor(home_path);
    curl_global_cleanup();

    return 0;
}