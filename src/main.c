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
#include "help.h"

int main(void)
{
    //load libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    Path *home_path = path_ctor(".");

    printf(COLOR_CYAN"---SONGBOOK---\nA program made for automatically creating songbooks\n"COLOR_RESET);

    while(true)
    {
        ActionChoice choice = action_choice();
        //TODO all possible choices
        if (choice == new_sb)
        {
            if (!new_songbook(home_path))
                printf(COLOR_RED"Failed to create specified songbook.\n"COLOR_RESET);
            else 
                printf(COLOR_GREEN"New songbook created successfully\n"COLOR_RESET);
        }
        else if (choice == edit_sb)
        {
            Songbook songbook;
            if (!choose_songbook(home_path, &songbook))
            {
                printf(COLOR_RED"Failed to get songbook to edit.\n"COLOR_RESET);
                continue;
            }
            else
                printf(COLOR_CYAN"Songbook chosen: %s\n"COLOR_RESET, songbook.name);

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
                printf(COLOR_GREEN"%s by %s successfully added to %s\n"COLOR_RESET, song.name_utf, song.author_utf, songbook.name);
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
                    printf(COLOR_GREEN"Quick read successfully read %d songs\n"COLOR_RESET, quick_result);
                    printf(COLOR_YELLOW"Unsuccessful attempts: (%u)\n"COLOR_RESET, errors.size);
                    for (unsigned int i = 0; i < errors.size; i++)
                    {
                        printf(COLOR_RED"%s\n"COLOR_RESET, errors.strings[i]);
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
                printf(COLOR_RED"Failed to get songbook to delete\n"COLOR_RESET);
            else if (del_choice == 1)
            {
                if (!remove_songbook(&songbook))
                    printf(COLOR_RED"Could not delete %s\n"COLOR_RESET, songbook.name);
                else 
                    printf(COLOR_GREEN"%s successfully removed\n"COLOR_RESET, songbook.name);
                songbook_dtor(&songbook);
            }
            else 
            {
                printf(COLOR_YELLOW"%s will not be deleted.\n"COLOR_YELLOW, songbook.name);
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
            first_to_upper(trans_chord);
            if (trans_chord == NULL)
            {
                fprintf(stderr, "Invalid chord.\n");
                goto add_trans_end;
            }

            bool trans_res = transpose_song(&song, trans_chord);
            if (trans_res)
                trans_res = add_song_songcollection(home_path, &song);
            if (!trans_res)
                printf(COLOR_RED"Transposition failed.\n"COLOR_RESET);
            else
                printf(COLOR_GREEN"Song transposed and saved successfully\n"COLOR_RESET);

            free(trans_chord);
        add_trans_end:
            song_dtor(&song);
            continue;
        }
        else if (choice == print_help)
        {
            help_menu(home_path);
        }
        else
        {
            printf(COLOR_CYAN"\nBye!\n"COLOR_RESET);
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