#include "pathwork.h"
#include <stdlib.h>
#include <string.h>

//allocates new path with start_path at the beginning
//start_path == NULL for empty new path
Path *path_ctor(char *start_path)
{
    unsigned int start_length = 0;
    if (start_path != NULL)
        start_length = strlen(start_path);

    Path *path = malloc(sizeof(Path));
    if (path == NULL)
        return NULL;

    path->path = malloc(PATH_START_CAPACITY + start_length);
    if (path->path == NULL)
    {
        free(path);
        return NULL;
    }

    if (start_length > 0)
        strcpy(path->path, start_path);
    else
        path->path[0] = '\0';

    path->capacity = PATH_START_CAPACITY + start_length;
    path->length = start_length;
    return path;
}

bool path_resize(Path *path)
{
    if (path == NULL)
        return false;
    if (path->path == NULL)
        return false;
    
    char *tmp = realloc(path->path, path->capacity * 2);
    if (tmp == NULL)
        return false;
    path->path = tmp;
    path->capacity *= 2;
    return true;
}

void path_dtor(Path *path)
{
    if (path == NULL)
        return;
    if (path->path != NULL)
        free(path->path);
    free(path);
}

//adds next part of path to curr_path
//mode can be 'd' for directory, 'f' for file (default), 's' for suffix (must include '.' before the suffix)
bool path_add(Path *curr_path, char *to_add, char mode)
{
    if (to_add == NULL || curr_path == NULL || curr_path->path == NULL)
        return false;
    
    if (curr_path->length > 0)
    {
        //add DIFF_CHAR if needed
        if (curr_path->path[curr_path->length-1] != DIFF_CHAR 
            && to_add[0] != DIFF_CHAR
            && mode != 's')
        {
            if (curr_path->length + 2 >= curr_path->capacity)
                if (!path_resize(curr_path))
                    return false;
            curr_path->path[curr_path->length++] = DIFF_CHAR;
        }

        //check if both don't have DIFF_CHAR
        if (curr_path->path[curr_path->length - 1] == DIFF_CHAR
            && to_add[0] == DIFF_CHAR)
            to_add++;
    }

    for (unsigned int i = 0; to_add[i] != '\0'; i++)
    {
        //+3 for char to be added, possible DIFF_CHAR and '\0'
        if (curr_path->length + 3 >= curr_path->capacity)
            if (!path_resize(curr_path))
                return false;
        curr_path->path[curr_path->length] = to_add[i];
        curr_path->length++;
    }

    if (mode == 'd')
    {
        if (curr_path->path[curr_path->length - 1] != DIFF_CHAR)
        {
            curr_path->path[curr_path->length] = DIFF_CHAR;
            curr_path->length++;
        }
    }

    curr_path->path[curr_path->length] = '\0';
    return true;
}

//returns a copy of to_copy (useful for path branching)
//reduce = true for making capacity == length + 1 (useful if the copy is not to be expanded)
Path *path_copy(Path *to_copy, bool reduce)
{
    if (to_copy == NULL)
        return NULL;

    Path *new_path = malloc(sizeof(Path));
    if (new_path == NULL)
        return NULL;
    
    if (reduce)
        new_path->capacity = to_copy->length + 1;
    else
        new_path->capacity = to_copy->capacity;

    new_path->path = malloc(new_path->capacity);
    if (new_path->path == NULL)
    {
        free(new_path);
        return NULL;
    }

    strcpy(new_path->path, to_copy->path);
    
    new_path->length = to_copy->length;
    return new_path;
}

//loses the last directory/file od the path
//returns path of the same capacity ending with DIFF_CHAR
void path_dirback(Path *path)
{
    if (path == NULL || path->length == 0)
        return;

    path->length--;
    while (path->length != 0 && path->path[path->length - 1] != DIFF_CHAR)
        path->length--;

    path->path[path->length] = '\0';
}