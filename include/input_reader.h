#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    char *string;
    unsigned int length;
    unsigned int capacity;
} DynBuff;

typedef struct {
    char *string;
    unsigned int length;
    unsigned int start_idx;
    unsigned int end_idx;
} CircBuff;

char *read_line(FILE *file);

bool read_until(FILE *file, char *sequence);

DynBuff buffer_ctor(unsigned int start_capacity);

bool buffer_resize(DynBuff *buffer);

CircBuff circ_buff_ctor(unsigned int size);

#endif