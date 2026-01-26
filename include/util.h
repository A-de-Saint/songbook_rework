#ifndef UTIL_H
#define UTIL_H

#include "pathwork.h"
#include <stdbool.h>

typedef struct {
    char **strings;
    unsigned int size;
    unsigned int capacity;
} StringArray;

StringArray str_arr_ctor();

bool str_arr_resize(StringArray *str_arr);

bool str_arr_add(StringArray *str_arr, char *to_add);

void str_arr_dtor(StringArray *str_arr);

bool read_insert_write(Path *path, char *to_insert);

#endif