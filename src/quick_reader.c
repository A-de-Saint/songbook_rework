#include "quick_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ak_parser.h"
#include "tex_manager.h"
#include "html_manager.h"
#include "multiple_songs.h"
#include "transpose.h"
#include "menu.h"

//downloads all songs in songbooks queue/queue.txt
//returns -1 if function failed
//else returns number of songs successfully added
//fills unsuccessful with songs that failed
int get_multiple_songs(Path *home_path, Songbook *songbook, StringArray *unsuccessful)
{
    if (home_path == NULL || songbook == NULL)
        return false;
    if (home_path->path == NULL || songbook->path == NULL || songbook->path->path == NULL || songbook->name == NULL)
        return false;

    //get path to queue.txt
    Path *queue_path = path_copy(songbook->path, false);
    if (queue_path == NULL)
        return false;
    if (!path_add(queue_path, "queue", 'd') ||
        !path_add(queue_path, "queue.txt", 'f'))
    {
        path_dtor(queue_path);
        return false;
    }

    FILE *file = fopen(queue_path->path, "r");
    path_dtor(queue_path);
    if (file == NULL)
    {
        fprintf(stderr, "Could not open queue.txt in %s\n", songbook->name);
        return -1;
    }

    char *format = parse_format(file);
    if (format == NULL)
    {
        fprintf(stderr, "Could not parse format\n");
        fclose(file);
        return -1;
    }
    asciize_separator(format);

    //get what goes first
    ComesFirst first;
    char *line = read_line(file);
    bool success = (line != NULL);
    if (success)
    {
        trim_string(line);
        if (strcmp(line, "author") == 0)
            first = AUTHOR;
        else if (strcmp(line, "name") == 0)
            first = NAME;
        else
            success = false;
    }
    free(line);
    if (!success)
    {
        fprintf(stderr, "Could not determine whether author or name comes first in the format.\n");
        fclose(file);
        free(format);
        return -1;
    }

    //html-specific
    SongFiles sf;
    if (songbook->format == HTML)
    {
        if (!song_files_ctor(&sf, 64))
        {
            fclose(file);
            free(format);
            return -1;
        }
    }

    //now we have format and what comes first
    int success_count = 0;
    while ((line = read_line(file)) != NULL)
    {
        if (*line == '\0')
        {
            free(line);
            continue;
        }

        printf("\n%s\n", line);
        bool all_okay = true;

        //parse author and name
        size_t len = strlen(line);
        char *name = malloc(len + 1);
        char *author = malloc(len + 1);
        char *transpose_to = malloc(len + 1);
        if (!parse_name_author(line, format, first, name, author, transpose_to))
        {
            fprintf(stderr, "Could not parse %s by format %s\n", line, format);
            free(name);
            free(author);
            free(transpose_to);
            all_okay = false;
            goto loop_end;
        }

        Song song;
        song_ctor(&song);
        song.author_ascii = author;
        song.name_ascii = name;
        printf("%s\t%s\n", song.author_ascii, song.name_ascii);
        Path *song_path = song_try_find(name, author, home_path);
        if (song_path != NULL)
        {
            printf("Found in song_collection\n");
            bool decode_res = decode_song(song_path, &song);
            path_dtor(song_path);
            if (!decode_res)
            {
                fprintf(stderr, "Could not decode song from song_collection.\n");
                goto download;
            }
            if (transpose_to[0] != '\0')
            {
                if (!transpose_song(&song, transpose_to))
                {
                    fprintf(stderr, "Could not transpose locally saved version to %s\n", transpose_to);
                    goto download;
                }
            }
        }
        else //need to download
        {
            download:
            bool download_res = download_song_ak(&song, false);
            if (!download_res)
            {
                song_dtor(&song);
                free(transpose_to);
                all_okay = false;
                goto loop_end;
            }
            if (transpose_to[0] != '\0')
            {
                if (!transpose_song(&song, transpose_to))
                {
                    fprintf(stderr, "Could not transponse to %s\n", transpose_to);
                    song_dtor(&song);
                    free(transpose_to);
                    all_okay = false;
                    goto loop_end;
                }
            }
            if (!add_song_songcollection(home_path, &song))
            {
                fprintf(stderr, "Song could not be added to song_collection\n");
            }
        }
        free(transpose_to);

        //add song to songbook
        if (songbook->format == tex)
        {
            if (!add_song_tex(songbook, &song))
            {
                fprintf(stderr, "Could not add song to songbook\n");
                all_okay = false;
                goto pre_loop_end;
            }
            //add song to songlist.txt of the given songbook
            if (!add_song_songlist(song.author_ascii, song.name_ascii, song.first_chord, songbook->path))
            {
                fprintf(stderr, "Song couldn't be added to songlist.txt in %s\n", songbook->name);
                all_okay = false;
                goto pre_loop_end;
            }
        }
        else if (songbook->format == HTML)
        {
            //already adds to songlist
            if (!add_song_html(songbook, &song, false))
            {
                fprintf(stderr, "Could not add song to songbook\n");
                all_okay = false;
                goto pre_loop_end;
            }

            char *print = create_song_print_restricted(song.name_ascii, song.author_ascii);
            if (print == NULL)
            {
                all_okay = false;
                goto pre_loop_end;
            }
            //build path to song and fopen it
            Path *path = path_copy(songbook->path, false);
            if (!path_add(path, "html", 'd') ||
                !path_add(path, "songs", 'd') ||
                !path_add(path, print, 'f') ||
                !path_add(path, ".html", 's'))
            {
                path_dtor(path);
                free(print);
                all_okay = false;
                goto pre_loop_end;
            }
            //open the song
            FILE *song_file = fopen(path->path, "r");
            if (song_file == NULL)
            {
                fprintf(stderr, "Could not reopen %s\n", print);
                path_dtor(path);
                free(print);
                goto pre_loop_end;
            }

            bool html_res = song_files_add(&sf, song_file, path);
            free(print);
            path_dtor(path);
            if (!html_res)
            {
                all_okay = false;
                goto pre_loop_end;
            }
        }

    pre_loop_end:
        song_dtor(&song);

    loop_end:
        if (all_okay)
        {
            printf(COLOR_GREEN"SUCCESS\n"COLOR_RESET);
            success_count++;
            free(line);
        }
        else
        {
            printf(COLOR_RED"FAIL\n"COLOR_RESET);
            str_arr_add(unsuccessful, line);
        }
    }

    //after the loop, HTML needs to do stuff
    if (songbook->format == HTML)
    {
        bool html_res = true;
        bool main_res = false;
        bool files_closed = false;
        //build path to main.html
        Path *html_path = path_copy(songbook->path, false);
        if (!path_add(html_path, "html", 'd') ||
            !path_add(html_path, songbook->name, 'f') ||
            !path_add(html_path, ".html", 's'))
        {
            html_res = false;
            goto after_html_end;
        }

        //fix fontsizes
        printf("\nThe next step might take a while, please be patient...\n");
        html_res = fix_fontsizes_fclose(&sf, html_path);

        //build main (compile)
        if (html_res)
        {
            files_closed = true;
            main_res = html_compile(songbook);
        }

    after_html_end:
        path_dtor(html_path);
        song_files_dtor(&sf, !files_closed); //need to negate (obvi)
        if (!html_res)
            printf(COLOR_YELLOW"WARN: songs added, but not with fixed sizes\n"COLOR_RESET);
        if (!main_res)
            printf(COLOR_YELLOW"WARN: everything went good, but main html could not be assembled\n"COLOR_RESET);
    }

    free(format);
    fclose(file);
    return success_count;
}

//parses format from the current line
//NULL if unparseable
char *parse_format(FILE *file)
{
    char *line = read_line(file);
    if (line == NULL)
        return NULL;

    if (strncmp(line, "Format:", 7) != 0)
    {
        return NULL;
    }

    int i = 6;

    //read until '"'
    for (; line[i] != '"'; i++)
    {
        if (line[i] == '\0')
        {
            free(line);
            return NULL;
        }
    }
    i++;

    //parse format while counting %s
    bool escaped = false;
    int percent_count = 0; //counts %
    int j = 0;
    for (; line[i] != '"' && !escaped; i++)
    {
        if (line[i] == '\0')
        {
            free(line);
            return NULL;
        }
        line[j++] = line[i];
        if (line[i] == '%' && !escaped)
            percent_count++;
        if (line[i] == '\\' && !escaped)
            escaped = true;
        else
            escaped = false;
    }
    line[i] = '\0';

    //2 for just author and name, 3 for transposition
    if (percent_count != 2 && percent_count != 3)
    {
        free(line);
        return NULL;
    }

    return line;
}

//parses name_ascii and author_ascii from string based on given format and which comes first
//does not check for NULL
bool parse_name_author(char *string, char *format, ComesFirst first, char *name, char *author, char *transpose_to)
{
    asciize_separator(string);
    int res = 0;
    if (first == AUTHOR)
    {
        res = sscanf(string, format, author, name, transpose_to);
        if (res == 2)
            transpose_to[0] = '\0';
        else if (res != 3)
            return false;
    }
    else
    {
        res = sscanf(string, format, name, author, transpose_to);
        if (res == 2)
            transpose_to[0] = '\0';
        else if (res != 3)
            return false;
    }
    convert_to_ascii(author);
    convert_to_ascii(name);
    trim_string(author);
    trim_string(name);
    if (transpose_to[0] != '\0')
        first_to_upper(transpose_to);
    return true;
}

//turns weird utf-8 dashes from string into normal '-'
void asciize_separator(char *string)
{
    if (string == NULL)
        return;
    
    int i = 0;
    int j = 0;
    while (string[i] != '\0')
    {
        //check for E2 80 93 || E2 80 93
        if ((unsigned char)string[i] == 0xE2 &&
            (unsigned char)string[i+1] == 0x80 &&
            ((unsigned char)string[i+2] == 0x93 ||
            (unsigned char)string[i+2] == 0x94))
        {
            string[j++] = '-';
            i += 3;
            continue;
        }
        string[j++] = string[i++];
    }
}