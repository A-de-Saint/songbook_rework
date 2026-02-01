#ifndef WEB_DOWNLOADER_H
#define WEB_DOWNLOADER_H

#include <curl/curl.h>
#include <stdbool.h>

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} BigBuffer;

bool bb_ctor(BigBuffer *bb, size_t capacity);

bool bb_resize(BigBuffer *bb, size_t new_capacity);

void bb_dtor(BigBuffer *bb);

size_t write_to_buffer(void *ptr, size_t size, size_t nmemb, void *userdata);

char *read_website(char *url);

#endif