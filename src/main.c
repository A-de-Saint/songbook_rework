#include <stdio.h>
#include <stdlib.h>
#include "pathwork.h"
#include "menu.h"
#include "songbook_manager.h"
#include <curl/curl.h>
#include "song_manager.h"
#include "tex_manager.h"
#include "quick_reader.h"
#include "html_manager.h"
#include "transpose.h"

int main(void)
{
    //load libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    Path *home_path = path_ctor(".");

    printf("---SONGBOOK---\nA program made for automatically creating songbooks\n");

    while(true)
    {
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
            {
                printf("Failed to get songbook to edit.\n");
                continue;
            }
            else
                printf("Songbook chosen: %s\n", songbook.name);

            EditSBChoice edit_action = edit_choice();

            Song song;
            song_ctor(&song);

            if (edit_action == add_song)
            {
                //get a song
                if (!get_song(home_path, &song))
                {
                    fprintf(stderr, "Failed to get song.\n");
                    goto song_edit_end;
                }

                //add the song based on format
                if (songbook.format == tex)
                {
                    if (!add_song_tex(&songbook, &song))
                    {
                        fprintf(stderr, "Failed to add %s to %s\n", song.name_utf, songbook.name);
                        goto song_edit_end;
                    }
                    //add song to songlist
                    if (!add_song_songlist(song.author_ascii, song.name_ascii, songbook.path))
                    {
                        fprintf(stderr, "Song couldn't be added to songlist.txt in %s\n", songbook.name);
                        goto song_edit_end;
                    }
                }
                else if (songbook.format == HTML)
                {
                    //songlist add is handled within here
                    if (!add_song_html(&songbook, &song, true))
                    {
                        fprintf(stderr, "Failed to add %s to %s", song.name_utf, songbook.name);
                        goto song_edit_end;
                    }
                    if (!html_compile(&songbook))
                    {
                        fprintf(stderr, "Failed to 'compile' %s.html. Song was added\n", songbook.name);
                        goto song_edit_end;
                    }
                }

                putchar('\n');
                printf("%s by %s successfully added to %s\n", song.name_utf, song.author_utf, songbook.name);
            }
            else if (edit_action == add_multiple)
            {
                StringArray errors = str_arr_ctor(16);
                int quick_result = get_multiple_songs(home_path, &songbook, &errors);
                if (quick_result == -1)
                {
                    fprintf(stderr, "Could not perform quick song add upon %s\n", songbook.name);
                }
                else 
                {
                    putchar('\n');
                    printf("Quick read successfully read %d songs\n", quick_result);
                    printf("Unsuccessful attempts: (%u)\n", errors.size);
                    for (unsigned int i = 0; i < errors.size; i++)
                    {
                        printf("%s\n", errors.strings[i]);
                    }
                }
                str_arr_dtor(&errors);
            }

        song_edit_end:
            song_dtor(&song);
            songbook_dtor(&songbook);
            continue;
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
        else if (choice == add_transpose)
        {
            Song song;
            song_ctor(&song);
            //get a song
            if (!get_song(home_path, &song))
            {
                fprintf(stderr, "Failed to get song.\n");
                goto add_trans_end;
            }

            printf("\nTranspose to (starting chord): ");
            char *trans_chord = read_line(stdin);
            if (trans_chord == NULL)
            {
                fprintf(stderr, "Invalid chord.\n");
                goto add_trans_end;
            }

            bool trans_res = transpose_song(&song, trans_chord);
            if (trans_res)
                trans_res = add_song_songcollection(home_path, &song);
            if (!trans_res)
                fprintf(stderr, "Transposition failed.\n");
            else
                printf("Song transposed and saved successfully\n");

            free(trans_chord);
        add_trans_end:
            song_dtor(&song);
            continue;
        }
        else
        {
            printf("\nBye!\n");
            break;
        }
    }

    /*Song song;
    song_ctor(&song);
    if (!get_song(home_path, &song))
        printf("something went wrong\n");
    song_dtor(&song);*/

    path_dtor(home_path);
    curl_global_cleanup();

    return 0;
}