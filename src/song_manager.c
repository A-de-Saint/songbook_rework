#include "song_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pathwork.h"
#include "input_reader.h"
#include "menu.h"
#include "ak_parser.h"
#include "web_downloader.h"

#define LINES_START_CAPACITY 8
#define PARTS_START_CAPACITY 8

bool song_part_ctor(SongPart *song_part)
{
    if (song_part == NULL)
        return false;

    song_part->lines = str_arr_ctor(LINES_START_CAPACITY);
    if (song_part->lines.strings == NULL)
        return false;

    song_part->type = UNKNOWN;
    return true;
}

void song_part_dtor(SongPart *song_part)
{
    if (song_part == NULL)
        return;

    if (song_part->lines.strings != NULL)
        str_arr_dtor(&song_part->lines);
}

bool song_data_ctor(SongData *song_data)
{
    if (song_data == NULL)
        return false;

    song_data->parts = malloc(PARTS_START_CAPACITY * sizeof(SongPart));
    if (song_data->parts == NULL)
        return false;

    song_data->capacity = PARTS_START_CAPACITY;
    song_data->count = 0;

    return true;
}

bool song_data_add(SongData *song_data, SongPart song_part)
{
    if (song_data == NULL)
        return false;
    
    if (song_data->count >= song_data->capacity)
    {
        if (!song_data_resize(song_data))
            return false;
    }

    song_data->parts[song_data->count] = song_part;
    song_data->count++;

    return true;
}

bool song_data_resize(SongData *song_data)
{
    if (song_data == NULL)
        return false;

    SongPart *tmp = realloc(song_data->parts, song_data->capacity * 2 * sizeof(SongPart));
    if (tmp == NULL)
        return false;
    song_data->parts = tmp;
    song_data->capacity *= 2;

    return true;
}

void song_data_dtor(SongData *song_data)
{
    if (song_data == NULL)
        return;

    if (song_data->parts == NULL)
        return;

    for (unsigned int i = 0; i < song_data->count; i++)
    {
        str_arr_dtor(&song_data->parts[i].lines);
    }

    free(song_data->parts);
}

void song_ctor(Song *song)
{
    song_data_ctor(&song->data);
    song->author_ascii = NULL;
    song->author_utf = NULL;
    song->name_ascii = NULL;
    song->name_utf = NULL;
}

void song_dtor(Song *song)
{
    if (song == NULL)
        return;

    song_data_dtor(&song->data);

    if (song->author_ascii != NULL)
        free(song->author_ascii);
    if (song->author_utf != NULL)
        free(song->author_utf);
    if (song->name_ascii != NULL)
        free(song->name_ascii);
    if (song->name_utf != NULL)
        free(song->name_utf);
}

//returns song print string (needs ascii-input)
//needs to be freed later
char *create_song_print(char *song_name, char *author)
{
    if (song_name == NULL || author == NULL)
        return NULL;

    //+2 for '_' (between) and '\0'
    size_t total = strlen(song_name) + strlen(author) + 2;
    char *print = malloc(total);
    if (print == NULL)
        return NULL;

    int i = 0;
    int j = 0;
    while (song_name[i] != '\0')
        print[j++] = song_name[i++];
    print[j++] = '_';
    i = 0;
    while (author[i] != '\0')
        print[j++] = author[i++];
    print[j] = '\0';
    return print;
}

//allocs name_save and author_save (overwrites if necessary)
//frees everything if return == false
bool decode_song_print(char *to_decode, char **name_save, char **author_save)
{
    if (to_decode == NULL)
        return false;
    
    size_t to_decode_size = strlen(to_decode);
    *name_save = malloc(to_decode_size);
    if (*name_save == NULL)
        return false;
    *author_save = malloc(to_decode_size);
    if (*author_save == NULL)
    {
        free(*name_save);
        return false;
    }

    int res = sscanf(to_decode, "%[^_]_%s", *name_save, *author_save);
    if (res != 2)
    {
        if (*name_save != NULL)
            free(*name_save);
        if (*author_save != NULL)
            free(*author_save);
        return false;
    }

    return true;
}

//tries to find song in song_collection, both author and name known
//returns NULL if error or not found
//returned path needs to be dtor'ed later
Path *song_try_find(char *song_name, char *author, Path *home_path)
{
    if (song_name == NULL || author == NULL || home_path == NULL)
        return NULL;
    if (home_path->path == NULL)
        return NULL;

    if (strcmp(author, "idk") == 0)
        return NULL;

    //copy path to not corrupt home_path
    Path *path = path_copy(home_path, false);
    if (path == NULL)
        return NULL;

    //make path to songlist.txt
    if (!path_add(path, "song_collection/songlist.txt", 'f'))
    {
        path_dtor(path);
        return NULL;
    }

    FILE *file = fopen(path->path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open song_collection/songlist.txt\n");
        path_dtor(path);
        return NULL;
    }

    char *print = create_song_print(song_name, author);
    if (print == NULL)
    {
        path_dtor(path);
        fclose(file);
        return NULL;
    }

    char *line;
    while ((line = read_line(file)) != NULL)
    {
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }

        //since the list is sorted, no need to search where strcmp is > 0
        int compar_res = strcmp(print, line);
        if (compar_res < 0)
        {
            free(line);
            break;
        }
        if (compar_res == 0)
        {
            fclose(file);
            path_dirback(path);
            if (!path_add(path, "songs", 'd') ||
                !path_add(path, line, 'f') ||
                !path_add(path, ".txt", 's'))
            {
                fprintf(stderr, "Path_add failed.\n");
                path_dtor(path);
                free(line);
                return NULL;
            }
            free(line);
            free(print);
            return path;
        }
        free(line);
    }
    fclose(file);
    path_dtor(path);
    free(print);
    return NULL;
}

//tries to find song by song name, no author known (asks user if that's the one)
//returns NULL if error or not found
//returned path needs to be dtor'ed later
Path *song_try_find_noauthor(char *song_name, Path *home_path)
{
    if (song_name == NULL || home_path == NULL)
        return NULL;
    if (home_path->path == NULL)
        return NULL;

    //copy path to keep home_path untouched
    Path *path = path_copy(home_path, false);
    if (path == NULL)
        return NULL;
    
    if (!path_add(path, "song_collection", 'd') || !path_add(path, "songlist.txt", 'f'))
    {
        path_dtor(path);
        return NULL;
    }

    FILE *file = fopen(path->path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open songs/songlist.txt\n");
        path_dtor(path);
        return NULL;
    }

    char *line;
    while ((line = read_line(file)) != NULL)
    {
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }

        char *name = NULL;
        char *author = NULL;
        if (decode_song_print(line, &name, &author))
        {
            int cmp_res = strcmp(song_name, name);
            if (cmp_res < 0)
            {
                free(line);
                free(name);
                free(author);
                fclose(file);
                break;
            }
            //if match, ask if that's the right one
            if (cmp_res == 0)
            {
                printf("Possible match found in song_collection\n");
                printf("Is this the right song?\n");
                printf("%s by %s\n", name, author);
                free(name);
                free(author);
                printf("[0] No\n[1] Yes\n[2] Uncertain (show part of the text)\n");
                printf("Your choice: ");
                int choice;
                while ((choice = readnum(0, 2)) == -1)
                    printf("Invalid input.\nTry again: ");
                //the right one
                if (choice == 1)
                {
                    fclose(file);
                    path_dirback(path);
                    if (!path_add(path, "songs", 'd') ||
                        !path_add(path, line, 'f') ||
                        !path_add(path, ".txt", 's'))
                    {
                        fprintf(stderr, "Path_add failed.\n");
                        path_dtor(path);
                        free(line);
                        return NULL;
                    }
                    free(line);
                    return path;
                }
                else if (choice == 0)
                {
                    free(line);
                    continue;
                }
                else //peek at song
                {
                    //build path
                    Path *peek_path = path_copy(path, false);
                    if (peek_path == NULL)
                    {
                        fprintf(stderr, "song_try_find_noauthor: Malloc failure\n");
                        free(line);
                        continue;
                    }
                    path_dirback(peek_path);
                    if (!path_add(peek_path, "songs", 'd') ||
                        !path_add(peek_path, line, 'f') ||
                        !path_add(peek_path, ".txt", 's'))
                    {
                        fprintf(stderr, "Path_add failed.\n");
                        path_dtor(peek_path);
                        free(line);
                        continue;
                    }

                    //peek
                    if (!song_peek(peek_path))
                    {
                        fprintf(stderr, "Could not peek at song. Wrong formatting likely\n");
                        path_dtor(peek_path);
                        free(line);
                        continue;
                    }

                    printf("Is this the one?\n");
                    printf("[0] No\n[1] Yes\nYour choice: ");
                    int peek_choice;
                    while ((peek_choice = readnum(0,1)) == -1)
                        printf("Invalid choice\nTry again: ");
                    if (peek_choice == 1)
                    {
                        path_dtor(path);
                        free(line);
                        return peek_path;
                    }
                    else
                    {
                        path_dtor(peek_path);
                        free(line);
                        continue;
                    }
                }
            }
            free(author);
            free(name);
        }
        free(line);
    }
    path_dtor(path);
    return NULL;
}

bool song_peek(Path *song_path)
{
    FILE *file = fopen(song_path->path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open '%s'\n", song_path->path);
        return false;
    }

    //get full name and author
    char *line = read_line(file);
    if (line == NULL)
        return false;
    printf("Song: %s\n", line);
    free(line);
    line = read_line(file);
    if (line == NULL)
        return false;
    printf("By: %s\n", line);
    free(line);
    
    //skip chord parts (intro)
    bool read_something = false;
    while (true)
    {
        line = read_line(file);
        if (line == NULL)
            return read_something;
        if (line[0] == '\\')
        {
            read_something = true;
            if (line[1] != '2' && line[1] != '3' && line[1] != '4')
            {
                free(line);
                break;
            }
        }
        free(line);
    }

    for (int i = 0; i < 3; i++)
    {
        line = read_line(file);
        if (line == NULL)
            break;
        if (line[0] == '\0')
        {
            i--;
            continue;
        }
        printf("%s\n", line);
        free(line);
    }

    return true;
}

//downloads given song from akordy-kytary
//song->name_ascii and song->author_ascii must be filled with valid data
//fills the rest of song
bool download_song_ak(Song *song, bool noauthor)
{
    if (song == NULL)
        return false;
    if (song->name_ascii == NULL || song->author_ascii == NULL)
        return false;
        
    //create url
    char url_general[] = "https://akordy.kytary.cz/song/";
    //+4 for '\0' and possible "-11" (two digits max) after song_name in url
    int last_char = strlen(url_general) + strlen(song->name_ascii);
    char *url = malloc(last_char + 4);
    if (url == NULL)
    {
        fprintf(stderr, "download_song_ak: Malloc failure\n");
        return false;
    }
    strcpy(url, url_general);
    strcat(url, song->name_ascii);

    int i = 1;
    char *html = NULL;
    char *read_till = NULL;
    while (i < 100)
    {
        //if not found yet, add next number
        if (i > 1)
            url_addnum(url, last_char, i);

        printf("%s\n", url); //testing//
        
        //download
        html = read_website(url);
        if (html == NULL)
        {
            free(url);
            return false;
        }
        read_till = html;

        //try parsing author and name
        bool res;
        char desc_buff[100];
        char *description = &desc_buff[0];
        description[0] = '\0';
        res = parse_name_author_desc_ak(html, song, &description, &read_till);
        if (!res) //unparseable -> custom 404 on web probably
        {
            free(html);
            html = NULL;
            if (i == 1) //some songs only have the -2 version for some reason
            {
                i++;
                continue;
            }
            free(url);
            return false;
        }

        if (noauthor)
        {
            printf("Found a song!\n");
            printf("%s by %s\nThe song goes:\n%s\n", song->name_utf, song->author_utf, description);
            printf("Is this the one?\n");
            printf("[0] No\n[1] Yes\n");
            printf("Your choice: ");
            int choice;
            while ((choice = readnum(0, 1)) == -1)
                printf("Invalid choice\nTry again: ");

            //if yes, create ascii author and break with good result
            if (choice == 1)
            {
                char *ascii_author = malloc(strlen(song->author_utf) + 1);
                ascii_author = strcpy(ascii_author, song->author_utf);
                convert_to_ascii(ascii_author);
                trim_string(ascii_author);
                free(song->author_ascii);
                song->author_ascii = ascii_author;
                break;
            }
        }
        else
        {
            //check if author matches
            char *author_cpy = malloc(strlen(song->author_utf) + 1);
            strcpy(author_cpy, song->author_utf);
            convert_to_ascii(author_cpy);
            trim_string(author_cpy);
            bool cmp = strcmp(author_cpy, song->author_ascii) == 0;
            free(author_cpy);
            if (cmp)
                break;
        }
        
        //if no match, free html and start again with i+1
        free(html);
        html = NULL;
        read_till = NULL;
        if (song->author_utf != NULL)
        {
            free(song->author_utf);
            song->author_utf = NULL;
        }
        if (song->name_utf != NULL)
        {
            free(song->name_utf);
            song->name_utf = NULL;
        }

        i++;
    }

    free(url);

    //check if html was read
    if (read_till == NULL || html == NULL)
    {
        if (html != NULL)
            free(html);
        return false;
    }

    //at this point, we have valid html, author and name
    bool didit = parse_chords_lyrics_ak(read_till, song);
    free(html);
    
    return didit;
}

bool add_song_songcollection(Path *home_path, Song *song)
{
    if (home_path == NULL || song == NULL)
        return false;
    if (home_path->path == NULL || song->author_ascii == NULL || song->author_utf == NULL)
        return false;
    if (song->name_ascii == NULL || song->name_utf == NULL || song->data.parts == NULL)
        return false;

    //build path
    Path *path = path_copy(home_path, false);
    if (path == NULL)
        return false;
    if (!path_add(path, "song_collection", 'd') || !path_add(path, "songs", 'd'))
    {
        path_dtor(path);
        return false;
    }

    //create print
    char *print = create_song_print(song->name_ascii, song->author_ascii);
    if (print == NULL)
    {
        path_dtor(path);
        return false;
    }

    //complete path
    if (!path_add(path, print, 'f') || !path_add(path, ".txt", 's'))
    {
        path_dtor(path);
        free(print);
        return false;
    }

    FILE *file = fopen(path->path, "w");
    if (file == NULL)
    {
        path_dtor(path);
        free(print);
        return false;
    }

    //print song
    fprintf(file, "%s\n", song->name_utf);
    fprintf(file, "%s\n", song->author_utf);
    for (unsigned int i = 0; i < song->data.count; i++)
    {
        SongPart *part = &song->data.parts[i];
        fprintf(file, "\\%d\\\n", (int)part->type);
        for (unsigned int j = 0; j < part->lines.size; j++)
            fprintf(file, "%s\n", part->lines.strings[j]);
        fprintf(file, "\\%d\\\n", (int)part->type);
    }
    fclose(file);

    //add song to songlist.txt
    //build path
    path_dirback(path);
    path_dirback(path);
    path_add(path, "songlist.txt", 'f');
    bool success = read_insert_write(path, print);
    path_dtor(path);
    free(print);

    return success;
}

void url_addnum(char *url, int last_char, int num)
{
    if (url == NULL)
        return;

    char *add_here = &url[last_char];
    sprintf(add_here, "-%d", num);
}

//decodes a song in song_collection
//does nothing to song's author_ascii and name_ascii
//path is to the song itself in song_collection
//saves the rest of data to song
bool decode_song(Path *path, Song *song)
{
    if (path == NULL || song == NULL)
        return false;
    if (path->path == NULL || song->data.parts == NULL)
        return false;

    FILE *file = fopen(path->path, "r");
    if (file == NULL)
        return false;
    
    //read name
    char *name = read_line(file);
    if (name == NULL || name[0] == '\0')
    {
        if (name != NULL)
            free(name);
        fprintf(stderr, "decode_song: Unexpected format at %s\n", path->path);
        return false;
    }
    trim_sides(name);
    song->name_utf = name;

    //read author
    char *author = read_line(file);
    if (author == NULL || author[0] == '\0')
    {
        if (name != NULL)
            free(name);
        fprintf(stderr, "decode_song: Unexpected format at %s\n", path->path);
        return false;
    }
    trim_sides(author);
    song->author_utf = author;

    //add author_ascii if unknown
    if (strcmp(song->author_ascii, "idk") == 0)
    {
        free(song->author_ascii);
        song->author_ascii = malloc(strlen(song->author_utf));
        if (song->author_ascii == NULL)
            return false;
        strcpy(song->author_ascii, song->author_utf);
        convert_to_ascii(song->author_ascii);
        trim_string(song->author_ascii);
    }

    //read lines
    while (true)
    {
        char *line = read_line(file);
        if (line == NULL)
            break;
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }

        //determine part type
        int type_i;
        int res = sscanf(line, "\\%d\\", &type_i);
        free(line);
        if (res != 1)
        {
            fprintf(stderr, "Unexpected format at %s\n", path->path);
            return false;
        }
        
        SongPart part;
        if (!song_part_ctor(&part))
            return false;
        part.type = (PartState)type_i;

        //read part
        while (true)
        {
            line = read_line(file);
            if (line == NULL)
            {
                fprintf(stderr, "Unexpected format at %s\n", path->path);
                song_part_dtor(&part);
                return false;
            }
            if (line[0] == '\0')
            {
                free(line);
                continue;
            }
            if (line[0] == '\\') //means end of part
            {
                free(line);
                break;
            }
            if (!str_arr_add(&part.lines, line))
            {
                song_part_dtor(&part);
                free(line);
                return false;
            }
        }
        song_data_add(&song->data, part);
    }

    fclose(file);

    return true;
}

//adds song to songlist in a given songbook
bool add_song_songlist(char *author_ascii, char *name_ascii, Path *songbook_path)
{
    if (author_ascii == NULL || name_ascii == NULL || songbook_path == NULL || songbook_path->path == NULL)
        return false;
    
    //build path to songlist.txt
    Path *path = path_copy(songbook_path, false);
    if (path == NULL)
        return false;
    if (!path_add(path, "songlist.txt", 'f'))
    {
        path_dtor(path);
        return false;
    }

    char *print = create_song_print(name_ascii, author_ascii);
    if (print == NULL)
    {
        path_dtor(path);
        return false;
    }

    bool result = read_insert_write(path, print);

    return result;
}