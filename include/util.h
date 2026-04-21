#ifndef UTIL_H
#define UTIL_H

#include "pathwork.h"
#include <stdbool.h>

typedef struct {
    char **strings;
    unsigned int size;
    unsigned int capacity;
} StringArray;

typedef struct {
    float *data;
    unsigned int size;
    unsigned int capacity;
} FloatArray;

StringArray str_arr_ctor(unsigned int capacity);

bool str_arr_resize(StringArray *str_arr);

bool str_arr_add(StringArray *str_arr, char *to_add);

void str_arr_dtor(StringArray *str_arr);

bool read_insert_write(Path *path, char *to_insert, int n);

bool read_remove_write(Path *file_path, char *to_remove);

bool convert_to_ascii(char *to_convert);

void trim_string(char *string);

char check_name(char *name);

#endif