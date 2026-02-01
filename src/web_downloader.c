#include "web_downloader.h"
#include <stdlib.h>
#include <string.h>

#define RESIZE_CHUNK_MULT 2
#define BB_START_CAPACITY 64 * 1024

bool bb_ctor(BigBuffer *bb, size_t capacity)
{
    if (bb == NULL)
        return false;

    bb->data = malloc(capacity);
    if (bb->data == NULL)
        return false;

    bb->capacity = capacity;
    bb->size = 0;
    return true;
}

bool bb_resize(BigBuffer *bb, size_t new_capacity)
{
    if (bb == NULL || bb->data == NULL)
        return false;

    //could be optional, but in my case, this is probably the best
    if (new_capacity < bb->capacity)
        return false;

    char *tmp = realloc(bb->data, new_capacity);
    if (tmp == NULL)
        return false;
    bb->data = tmp;
    bb->capacity = new_capacity;
    return true;
}

void bb_dtor(BigBuffer *bb)
{
    if (bb == NULL)
        return;
    if (bb->data != NULL)
        free(bb->data);
}

size_t write_to_buffer(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    if (userdata == NULL)
        return 0;
    
    BigBuffer *bb = (BigBuffer *)userdata;
    size_t total_size = size * nmemb;

    //check if size after addition (including '\0') does not overflow
    if (bb->size + total_size + 1 >= bb->capacity)
    {
        //if resize fails, read as much as possible
        if (!bb_resize(bb, bb->capacity + (total_size * RESIZE_CHUNK_MULT)))
        {
            total_size = (bb->capacity - 1 - bb->size);
            if (total_size <= 0)
                return 0;
        }
    }

    memcpy(bb->data + bb->size, ptr, total_size);
    bb->size += total_size;
    bb->data[bb->size] = '\0';

    return total_size;
}

//reads data (HTML) from a website 
//url must contain the entire url (https://example.com/)
//must be freed later
char *read_website(char *url)
{
    CURL *curl;
    BigBuffer bb;
    if (!bb_ctor(&bb, BB_START_CAPACITY))
        return NULL;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl)
    {
        bb_dtor(&bb);
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bb);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); //should allow following redirects

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        fprintf(stderr, "Curl failed. Error: %s\n", curl_easy_strerror(res));
        bb_dtor(&bb);
        return NULL;
    }

    return bb.data;
}