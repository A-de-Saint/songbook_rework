#include "quick_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ak_parser.h"
#include "tex_manager.h"

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
        if (!parse_name_author(line, format, first, name, author))
        {
            fprintf(stderr, "Could not parse %s by format %s\n", line, format);
            free(name);
            free(author);
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
            bool decode_res = decode_song(song_path, &song);
            path_dtor(song_path);
            if (!decode_res)
            {
                fprintf(stderr, "Could not decode song from song_collection.\n");
                goto download;
            }
        }
        else //need to download
        {
            download:
            bool download_res = download_song_ak(&song, false);
            if (!download_res)
            {
                song_dtor(&song);
                all_okay = false;
                goto loop_end;
            }
            if (!add_song_songcollection(home_path, &song))
            {
                fprintf(stderr, "Song could not be added to song_collection\n");
            }
        }

        //add song to songbook
        if (songbook->format == tex)
        {
            if (!add_song_tex(songbook, &song))
            {
                fprintf(stderr, "Could not add song to songbook\n");
                all_okay = false;
                song_dtor(&song);
                goto loop_end;
            }
        }

        //add song to songlist.txt of the given songbook
        if (!add_song_songlist(song.author_ascii, song.name_ascii, songbook->path))
        {
            fprintf(stderr, "Song couldn't be added to songlist.txt in %s\n", songbook->name);
            all_okay = false;
        }

        song_dtor(&song);

    loop_end:
        if (all_okay)
        {
            printf("SUCCESS\n");
            success_count++;
            free(line);
        }
        else
        {
            printf("FAIL\n");
            str_arr_add(unsuccessful, line);
        }
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

    if (percent_count != 2)
    {
        free(line);
        return NULL;
    }

    return line;
}

//parses name_ascii and author_ascii from string based on given format and which comes first
//does not check for NULL
bool parse_name_author(char *string, char *format, ComesFirst first, char *name, char *author)
{
    if (first == AUTHOR)
    {
        if (sscanf(string, format, author, name) != 2)
            return false;
    }
    else
    {
        if (sscanf(string, format, name, author) != 2)
            return false;
    }
    convert_to_ascii(author);
    convert_to_ascii(name);
    trim_string(author);
    trim_string(name);
    return true;
}