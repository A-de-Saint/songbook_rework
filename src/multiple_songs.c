#include "multiple_songs.h"
#include <string.h>

bool song_files_ctor(SongFiles *song_files, unsigned int capacity)
{
    if (song_files == NULL || capacity <= 0)
        return false;

    song_files->songs = malloc(sizeof(FILE *) * capacity);
    if (song_files->songs == NULL)
        return false;
    song_files->paths = malloc(sizeof(char *) * capacity);
    if (song_files->paths == NULL)
    {
        free(song_files->songs);
        return false;
    }

    song_files->count = 0;
    song_files->capacity = capacity;

    return true;
}

bool song_files_resize(SongFiles *sf)
{
    //realloc songs
    FILE ** tmp = realloc(sf->songs, sf->capacity * 2 * sizeof(SongFiles));
    if (tmp == NULL)
        return false;
    sf->songs = tmp;

    //realloc paths
    char ** temp = realloc(sf->paths, sf->capacity * 2 * sizeof(char *));
    if (temp == NULL)
        return false;
    sf->paths = temp;

    sf->capacity *= 2;
    return true;
}

//copies path's path (so that path_dtor still works)
bool song_files_add(SongFiles *song_files, FILE *file, Path *path)
{
    if (song_files == NULL || file == NULL || path == NULL)
        return false;

    if (song_files->count >= song_files->capacity)
        if (!song_files_resize(song_files))
            return false;

    song_files->songs[song_files->count] = file;
    song_files->paths[song_files->count] = malloc(path->length + 1);
    if (song_files->paths[song_files->count] == NULL)
        return false;
    strcpy(song_files->paths[song_files->count], path->path);
    song_files->count++;
    return true;
}

//frees song_files' FILE array and path array
void song_files_dtor(SongFiles *song_files, bool close_files)
{
    if (song_files == NULL)
        return;
    
    if (close_files)
    {
        for (unsigned int i = 0; i < song_files->count; i++)
        {
            if (song_files->songs[i] != NULL)
                fclose(song_files->songs[i]);
        }
    }

    if (song_files->songs != NULL)
        free(song_files->songs);
    song_files->songs = NULL;

    if (song_files->paths != NULL)
    {
        for (unsigned int i = 0; i < song_files->count; i++)
        {
            if (song_files->paths[i] != NULL)
                free(song_files->paths[i]);
        }
        free(song_files->paths);
    }
}

//resets all files (switches from read to write and goes to the start)
//does NOT truncate, so make sure that the result is >= that the prev content (if overwriting is the goal)
void reset_files(SongFiles *song_files)
{
    if (song_files == NULL)
        return;
    for (unsigned int i = 0; i < song_files->count; i++)
    {
        fflush(song_files->songs[i]);
        rewind(song_files->songs[i]);
    }
}