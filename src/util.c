#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "input_reader.h"
#include <string.h>

#define STR_ARR_START_CAPACITY 32
#define UPPERCASE_LOWERCASE_DIFF 32

StringArray str_arr_ctor(unsigned int capacity)
{
    StringArray str_arr;
    str_arr.capacity = capacity;
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
//if already there, won't add again (no duplicates allowed)
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

    StringArray str_arr = str_arr_ctor(STR_ARR_START_CAPACITY);
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

            int comp = strcmp(to_insert, string);
            if (comp == 0) //prevents duplicates
            {
                free(to_insert_cpy);
                added = true;
            }
            if (!added && comp < 0)
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

    StringArray str_arr = str_arr_ctor(STR_ARR_START_CAPACITY);
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

//converts to lowercase ascii chars (utf8 -> ascii equivalents, other chars -> '-')
bool convert_to_ascii(char *to_convert)
{
    if (to_convert == NULL)
        return false;
    
    int i = 0;
    int j = 0;
    while (to_convert[i] != '\0')
    {
        //keep lowercase and numbers
        if ((to_convert[i] >= 'a' && to_convert[i] <= 'z')
            || (to_convert[i] >= '0' && to_convert[i] <= '9'))
        {
            to_convert[j] = to_convert[i];
            j++;
            i++;
            continue;
        }

        //uppercase -> lowercase
        if (to_convert[i] >= 'A' && to_convert[i] <= 'Z')
        {
            to_convert[j] = to_convert[i] + UPPERCASE_LOWERCASE_DIFF;
            j++;
            i++;
            continue;
        }

        //other ascii chars get converted to '-'
        unsigned char ch = (unsigned char)to_convert[i];
        if (ch < 0x80)
        {
            to_convert[j] = '-';
            j++;
            i++;
            continue;
        }

        unsigned int cp;
        
        //convert chars to utf-8 int
        if ((ch & 0xE0) == 0xC0) //2 bytes
        {
            cp = ((ch & 0x1F) << 6) |
                ((unsigned char)to_convert[i+1] & 0x3F);
            i += 2;
        }
        else if ((ch & 0xF0) == 0xE0) //3 bytes
        {
            cp = ((ch & 0x0F) << 12) |
                (((unsigned char)to_convert[i+1] & 0x3F) << 6) |
                ((unsigned char)to_convert[i+2] & 0x3F);
            i += 3;
        }
        else if ((ch & 0xF8) == 0xF0) //4 bytes
        {
            cp = ((ch & 0x07) << 18) |
                (((unsigned char)to_convert[i+1] & 0x3F) << 12) |
                (((unsigned char)to_convert[i+2] & 0x3F) << 6) |
                ((unsigned char)to_convert[i+3] & 0x3F);
            i += 4;
        }
        else 
        {
            to_convert[j] = '-';
            j++;
            i++;
            continue;
        }

        //convert characters of interest to their ascii equivalents
        switch (cp)
        {
            //č, Č
            case 0x010D:
            case 0x010C:
                to_convert[j++] = 'c';
                break;

            //ř, Ř
            case 0x0159:
            case 0x0158:
                to_convert[j++] = 'r';
                break;

            //ď, Ď
            case 0x010F:
            case 0x010E:
                to_convert[j++] = 'd';
                break;

            //ě, Ě, é, É
            case 0x011B:
            case 0x011A:
            case 0x00E9:
            case 0x00C9:
                to_convert[j++] = 'e';
                break;

            //ň, Ň
            case 0x0148:
            case 0x0147:
                to_convert[j++] = 'n';
                break;

            //š, Š
            case 0x0161:
            case 0x0160:
                to_convert[j++] = 's';
                break;

            //ť, Ť
            case 0x0165:
            case 0x0164:
                to_convert[j++] = 't';
                break;

            //ž, Ž
            case 0x017E:
            case 0x017D:
                to_convert[j++] = 'z';
                break;

            //ů, Ů, ú, Ú
            case 0x016F:
            case 0x016E:
            case 0x00FA:
            case 0x00DA:
                to_convert[j++] = 'u';
                break;

            //á, Á
            case 0x00E1:
            case 0x00C1:
                to_convert[j++] = 'a';
                break;

            //í, Í
            case 0x00ED:
            case 0x00CD:
                to_convert[j++] = 'i';
                break;

            //ó, Ó
            case 0x00F3:
            case 0x00D3:
                to_convert[j++] = 'o';
                break;

            //ý, Ý
            case 0x00FD:
            case 0x00DD:
                to_convert[j++] = 'y';
                break;

            default:
                to_convert[j++] = '-';
        }
    }
    to_convert[j] = '\0';

    return true;
}

//trims string, so that there is no ' ' or '-' before or after the string, 
//and so that there are no multiple ' ' or '-' in a row
void trim_string(char *string)
{
    if (string == NULL)
        return;

    int i = 0;
    int j = 0;

    //trim start
    while (string[i] == ' ' || string[i] == '-')
        i++;

    bool prev = false;
    while (string[i] != '\0')
    {
        if (string[i] == ' ' || string[i] == '-')
        {
            if (!prev)
            {
                string[j++] = string[i];
                prev = true;
            }
            i++;
            continue;
        }

        string[j++] = string[i++];
        prev = false;
    }

    if (j > 0)
    {
        if (string[j-1] == ' ' || string[j-1] == '-')
        {
            string[j-1] = '\0';
            return;
        }
    }

    string[j] = '\0';
}

//checks string for unsupported chars (such as '\' or '/' or '|')
//returns [unsupported char] if !okay, else returns '\0'
char check_name(char *name)
{
    for (int i = 0; name[i] != '\0'; i++)
    {
        char c = name[i];
        if (c == '\\' || c == '/' || c == '|')
            return c;
    }
    return '\0';
}