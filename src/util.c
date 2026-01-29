#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "input_reader.h"
#include <string.h>

#define STR_ARR_START_CAPACITY 32

StringArray str_arr_ctor()
{
    StringArray str_arr;
    str_arr.capacity = STR_ARR_START_CAPACITY;
    str_arr.strings = malloc(str_arr.capacity * sizeof(char *));
    str_arr.size = 0;
    return str_arr;
}

bool str_arr_resize(StringArray *str_arr)
{
    if (str_arr == NULL)
        return false;

    char **tmp = realloc(str_arr->strings, str_arr->capacity * 2 * sizeof(char *));
    if (tmp == NULL)
        return false;
    str_arr->strings = tmp;
    str_arr->capacity *= 2;
    return true;
}

bool str_arr_add(StringArray *str_arr, char *to_add)
{
    if (str_arr == NULL || to_add == NULL)
        return false;
    if (str_arr->strings == NULL)
        return false;

    if (str_arr->size >= str_arr->capacity)
    {
        if (!str_arr_resize(str_arr))
            return false;
    }

    str_arr->strings[str_arr->size] = to_add;
    str_arr->size++;

    return true;
}

void str_arr_dtor(StringArray *str_arr)
{
    if (str_arr == NULL)
        return;
    if (str_arr->strings == NULL)
        return;
    for (unsigned int i = 0; i < str_arr->size; i++)
    {
        if (str_arr->strings[i] != NULL)
            free(str_arr->strings[i]);
    }
    free(str_arr->strings);
}

//reads a text file, saves lines, inserts to_insert alphabetically as a line, writes new text file
//cannot add empty string
//path must include filename and suffix
bool read_insert_write(Path *path, char *to_insert)
{
    if (path == NULL || to_insert == NULL)
        return false;
    if (path->path == NULL)
        return false;
    if (to_insert[0] == '\0')
        return false;

    //copy to_insert (so that it doesn't get freed from str_arr_dtor)
    char *to_insert_cpy = malloc(strlen(to_insert) + 1);
    strcpy(to_insert_cpy, to_insert);

    StringArray str_arr = str_arr_ctor();
    FILE *file = fopen(path->path, "r");

    bool added = false;
    if (file != NULL)
    {
        char *string;
        while ((string = read_line(file)) != NULL)
        {
            if (string[0] == '\0')
            {
                free(string);
                continue;
            }

            if (!added && strcmp(to_insert, string) <= 0)
            {
                if (!str_arr_add(&str_arr, to_insert_cpy))
                {
                    free(to_insert_cpy);
                    str_arr_dtor(&str_arr);
                    fclose(file);
                    return false;
                }
                added = true;
            }

            if (!str_arr_add(&str_arr, string))
            {
                if (!added)
                    free(to_insert_cpy);
                str_arr_dtor(&str_arr);
                fclose(file);
                return false;
            }
        }
        fclose(file);
    }

    if (!added)
    {
        if (!str_arr_add(&str_arr, to_insert_cpy))
        {
            free(to_insert_cpy);
            str_arr_dtor(&str_arr);
            return false;
        }
    }

    FILE *file_w = fopen(path->path, "w");
    if (file_w == NULL)
    {
        str_arr_dtor(&str_arr);
        return false;
    }

    for (unsigned int i = 0; i < str_arr.size; i++)
    {
        fprintf(file_w, "%s\n", str_arr.strings[i]);
    }

    fclose(file_w);
    str_arr_dtor(&str_arr);

    return true;
}

bool read_remove_write(Path *file_path, char *to_remove)
{
    if (file_path == NULL || to_remove == NULL)
    {
        return false;
    }

    FILE *file_r = fopen(file_path->path, "r");
    if (file_r == NULL)
    {
        fprintf(stderr, "read_remove_write: Could not open file.\n");
        return false;
    }

    StringArray str_arr = str_arr_ctor();
    char *string;
    while ((string = read_line(file_r)) != NULL)
    {
        if (string[0] == '\0')
        {
            free(string);
            continue;
        }

        if (strcmp(to_remove, string) == 0)
        {
            free(string);
            continue;
        }
        else 
            str_arr_add(&str_arr, string);
    }
    fclose(file_r);

    FILE *file_w = fopen(file_path->path, "w");
    if (file_w == NULL)
    {
        fprintf(stderr, "read_remove_write: Could not open file.\n");
        str_arr_dtor(&str_arr);
        return false;
    }

    for (unsigned int i = 0; i < str_arr.size; i++)
    {
        fprintf(file_w, "%s\n", str_arr.strings[i]);
    }
    str_arr_dtor(&str_arr);
    fclose(file_w);
    return true;
}
