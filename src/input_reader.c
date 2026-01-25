#include "input_reader.h"
#include <string.h>

//creates buffer with allocated string of start_capacity
DynBuff buffer_ctor(unsigned int start_capacity)
{
    DynBuff buffer;
    buffer.string = malloc(start_capacity);
    buffer.capacity = start_capacity;
    buffer.length = 0;

    return buffer;
}

bool buffer_resize(DynBuff *buffer)
{
    if (buffer == NULL)
        return false;
    if (buffer->string == NULL)
        return false;

    char *tmp = realloc(buffer->string, buffer->capacity * 2);
    if (tmp == NULL)
        return false;
    buffer->string = tmp;

    buffer->capacity *= 2;
    return true;
}

//reads one line until '\n' or EOF
//needs to be freed later
char *read_line(FILE *file)
{
    if (file == NULL)
        return NULL;

    DynBuff buffer = buffer_ctor(64);
    if (buffer.string == NULL)
        return NULL;

    int i;
    while((i = fgetc(file)) != EOF)
    {
        char c = (char)i;
        if (c == '\n')
            break;
        if (c == '\r')
            continue;

        //resize if needed (+1 for '\0')
        if (buffer.capacity <= buffer.length + 1)
        {
            bool res = buffer_resize(&buffer);
            if (!res)
            {
                free(buffer.string);
                return NULL;
            }
        }

        buffer.string[buffer.length] = c;
        buffer.length++;
    }
    buffer.string[buffer.length] = '\0';

    return buffer.string;
}

bool read_until(FILE *file, char *sequence)
{
    if (file == NULL || sequence == NULL)
        return false;

    CircBuff c_buff = circ_buff_ctor(strlen(sequence));
    if (c_buff.string == NULL)
        return false;

    while (true)
    {
        //load sequence-length chars into circular buffer
        //do-while because we start at start_idx == end_idx
        do
        {
            int c = fgetc(file);
            if (c == EOF)
            {
                free(c_buff.string);
                return false;
            }

            c_buff.string[c_buff.end_idx] = (char)c;
            c_buff.end_idx = (c_buff.end_idx + 1) % c_buff.length; //incrementing circular index
        }
        while (c_buff.end_idx != c_buff.start_idx);

        //compare c_buff string against sequence
        bool found = true;
        unsigned int i = 0;
        while (sequence[i] != '\0')
        {
            unsigned int idx = (c_buff.start_idx + i) % c_buff.length;
            if (sequence[i] != c_buff.string[idx])
            {
                found = false;
                break;
            }
            i++;
        }

        if (found)
        {
            free(c_buff.string);
            return true;
        }
        c_buff.start_idx = (c_buff.start_idx + 1) % c_buff.length;
    }
}

CircBuff circ_buff_ctor(unsigned int size)
{
    CircBuff c_buff;
    c_buff.string = malloc(size);
    c_buff.length = size;
    c_buff.start_idx = 0;
    c_buff.end_idx = 0;
    return c_buff;
}