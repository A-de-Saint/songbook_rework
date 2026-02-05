#include "song_manager.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pathwork.h"
#include "input_reader.h"
#include "menu.h"

void song_ctor(Song *song)
{
    song->lines = str_arr_ctor();
    song->author_ascii = NULL;
    song->author_utf = NULL;
    song->name_ascii = NULL;
    song->name_utf = NULL;
}

void song_dtor(Song *song)
{
    if (song == NULL)
        return;

    str_arr_dtor(&song->lines);

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
bool decode_song_print(char *to_decode, char *name_save, char *author_save)
{
    if (to_decode == NULL)
        return false;
    
    size_t to_decode_size = strlen(to_decode);
    name_save = malloc(to_decode_size);
    if (name_save == NULL)
        return false;
    author_save = malloc(to_decode_size);
    if (author_save == NULL)
    {
        free(name_save);
        return false;
    }

    int res = sscanf(to_decode, "%[^_]_%s", name_save, author_save);
    if (res != 2)
    {
        free(name_save);
        free(author_save);
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
        if (compar_res > 0)
        {
            free(line);
            break;
        }
        if (compar_res == 0)
        {
            fclose(file);
            path_dirback(path);
            if (!path_add(path, "songs", 'd') ||
                !path_add(path, line, 'f'))
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
    
    if (!path_add(path, "song_collection/songlist.txt", 'f'))
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
        if (decode_song_print(line, name, author))
        {
            int cmp_res = strcmp(song_name, name);
            if (cmp_res > 0)
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
                        !path_add(path, line, 'f'))
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
                else
                {
                    //TODO print part of the song (song_peek)
                    //line needs to be freed, so does author and name (if break or continue)
                    //path still alloc'd
                }
            }
            free(author);
            free(name);
        }
        free(line);
    }
    return NULL;
}

bool song_peek(Path *song_path)
{
    FILE *file = fopen(song_path->path, "r");
    if (file == NULL)
    {
        fprintf("Could not open '%s'\n", song_path->path);
        return false;
    }

    //TODO
}