#include "ak_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//reads string until substring is read
//returns number of chars to the start of substring in string
//returns -1 if not found
//sets after_substr to the pointer to the first char after substring in string
int s_read_until(char *string, char *substring, char **after_substr)
{
    if (string == NULL || substring == NULL)
        return -1;
    if (substring[0] == '\0')
    {
        if (after_substr != NULL)
            *after_substr = string;
        return 0;
    }

    int i = 0;
    while (string[i] != '\0')
    {   
        if (string[i] == substring[0])
        {
            int j = 0;
            bool found = true;
            for (; substring[j] != '\0'; j++)
            {
                if (string[i+j] == '\0')
                    return -1;

                if (string[i+j] != substring[j])
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                if (after_substr != NULL)
                    *after_substr = &string[i+j];
                return i;
            }
        }
        
        i++;
    }

    return -1;
}

//checks if ch is whitespace
bool isspace_lite(char ch)
{
    if (ch == ' ' ||
        ch == '\t' ||
        ch == '\n' ||
        ch == '\r' ||
        ch == '\f' ||
        ch == '\v')
        return true;
    return false;
}

//trims whitespace around a string (start and end)
void trim_sides(char *string)
{
    if (string == NULL)
        return;

    int i = 0;
    int j = 0;
    while (isspace_lite(string[i]))
        i++;
    while (string[i] != '\0')
        string[j++] = string[i++];
    while (j > 0 && isspace_lite(string[j-1]))
        j--;
    string[j] = '\0';
}

//parses song description, song author(utf) and song name(utf) from html
//saves last read char into read_until
//description must be at least a 100 chars long or NULL
bool parse_name_author_desc_ak(char *html, Song *song, char **description, char **read_until)
{
    if (html == NULL || song == NULL)
        return false;
    if (song->author_ascii == NULL || song->name_ascii == NULL)
        return false;

    int idx;
    char *curr_string = html;

    //parse description
    if (description != NULL)
    {
        idx = s_read_until(html, "name=\"description\"", &curr_string);
        if (idx == -1)
        {
            fprintf(stderr, "Could not find description\n");
            curr_string = html;
            (*description)[0] = '\0';
        }
        else
        {
            s_read_until(curr_string, "content=\"", &curr_string);
            strncpy(*description, curr_string, 99);
            (*description)[99] = '\0';
        }
    }

    //parse name
    idx = s_read_until(curr_string, "class=\"sheet-title\">", &curr_string);
    if (idx == -1)
    {
        fprintf(stderr, "Could not find song name\n");
        song->name_utf = NULL;
        return false;
    }
    else
    {
        int name_size = s_read_until(curr_string, "</h1>", NULL);
        if (name_size != -1)
            song->name_utf = malloc(name_size + 1);
        else 
            song->name_utf = NULL;

        
        if (song->name_utf == NULL)
            fprintf(stderr, "Couldn't allocate memory for song name\n");
        else
        {
            strncpy(song->name_utf, curr_string, name_size);
            song->name_utf[name_size] = '\0';
            trim_sides(song->name_utf);
        }
    }

    //parse author
    idx = s_read_until(curr_string, "class=\"sheet-author\"", &curr_string);
    if (idx == -1)
    {
        fprintf(stderr, "Could not find author.\n");
        song->author_utf = NULL;
        return false;
    }
    else 
    {
        //read until the start of author string
        idx = s_read_until(curr_string, "<u>", &curr_string);
        if (idx == -1)
        {
            fprintf(stderr, "Unexpected author formatting. Author not read\n");
            song->author_utf = NULL;
        }
        else
        {
            int author_size = s_read_until(curr_string, "</u>", NULL);
            if (author_size != -1)
                song->author_utf = malloc(author_size + 1);
            else 
                song->author_utf = NULL;
            
            if (song->author_utf == NULL)
                fprintf(stderr, "Could not allocate memory for author\n");
            else
            {
                strncpy(song->author_utf, curr_string, author_size);
                song->author_utf[author_size] = '\0';
                trim_sides(song->author_utf);
            }
        }
    }
    *read_until = curr_string;
    return true;
}

//parses all chords and lyrics from a song and saves it to song
bool parse_chords_lyrics_ak(char *html, Song *song)
{
    if (html == NULL || song == NULL)
        return false;
    if (song->data.parts == NULL)
        return false;

    char *curr_string = html;
    int res = 0;

    res = s_read_until(curr_string, "id=\"sheet-content\"", &curr_string);
    if (res == -1)
        return false;
    
    bool all_right = true;
    while (true)
    {
        res = s_read_until(curr_string, "class=\"scs-section\"", &curr_string);
        if (res == -1)
            break;
        //prepare song_part
        SongPart part;
        if (!song_part_ctor(&part))
        {
            all_right = false;
            break;
        }
        bool parse_res = parse_part_ak(curr_string, &curr_string, &part);
        if (!parse_res)
        {
            song_part_dtor(&part);
            break;
        }
        if (!song_data_add(&song->data, part))
        {
            song_part_dtor(&part);
            all_right = false;
            break;
        }
        //TODO recommend: maybe check if it's not the end
    }
    return all_right;
}

//parses one part of a song from curr_html
//curr_html should be after 'class="scs-section"'
//saves pointer to where it read to *read_until
bool parse_part_ak(char *curr_html, char **read_until, SongPart *song_part)
{
    if (curr_html == NULL || read_until == NULL || song_part == NULL)
        return false;

    int res = s_read_until(curr_html, "data-type=\"", &curr_html);
    if (res == -1)
        return false;
    char type[16];
    sscanf(curr_html, "%15[^\"]", type);
    
    //determine type
    if (strcmp(type, "intro") == 0)
        song_part->type = INTRO;
    else if (strcmp(type, "verse") == 0)
        song_part->type = VERSE;
    else if (strcmp(type, "chorus") == 0)
        song_part->type = CHORUS;
    else if (strcmp(type, "interlude") == 0)
        song_part->type = INTERLUDE;
    else if (strcmp(type, "bridge") == 0)
        song_part->type = BRIDGE;
    else if (strcmp(type, "ending") == 0)
        song_part->type = ENDING;
    else
        song_part->type = UNKNOWN;

    //read until the end of element declaration
    res = s_read_until(curr_html, ">", &curr_html);
    if (res == -1)
        return false;

    //read the part
    while (true)
    {
        //inside original div
        res = s_read_until(curr_html, "<", &curr_html);
        if (res == -1) //if this, it's just screwed
            return false;
        if (curr_html[0] == '/') //if this, then </div> -> end
            break;

        //now <og_div><
        res = s_read_until(curr_html, ">", &curr_html);
        if (res == -1)
            return false;
        //now <og_div><div>

        //read div contents
        DynBuff buffer = buffer_ctor(64);
        while (curr_html[0] != '\0') //not really, just in case it ended prematurely
        {
            //if <br || </ -> end of line
            //if <char> -> weird, skip that one
            if (curr_html[0] == '<')
            {
                if (curr_html[1] == 'b' && curr_html[2] == 'r')
                {
                    //just read after the end and return
                    res = s_read_until(&curr_html[2], "/div>", &curr_html);
                    break;
                }
                if (strncmp(curr_html, "<char>", 6) == 0) //skip <char>
                {
                    res = s_read_until(&curr_html[6], "</char>", &curr_html);
                    continue;
                }
                else if (curr_html[1] == '/')
                {
                    //read after end of </div>
                    res = s_read_until(&curr_html[2], ">", &curr_html);
                    break;
                }
                else //we're dealing with a chord
                {
                    res = s_read_until(curr_html, "class=\"scs-chk\"", &curr_html);
                    if (res == -1)
                        break;
                    res = s_read_until(curr_html, ">", &curr_html);
                    if (res == -1)
                        break;
                    buffer_add(&buffer, '[');

                    //now at scs-chk">
                    char chord[11];
                    sscanf(curr_html, "%10[^<]", chord);
                    for (int i = 0; chord[i] != '\0'; i++)
                        buffer_add(&buffer, chord[i]);
                    
                    res = s_read_until(curr_html, ">", &curr_html);
                    if (res == -1)
                        break;
                    res = s_read_until(curr_html, "<", &curr_html);
                    if (res == -1)
                        break;
                    while (curr_html[0] != '/')
                    {
                        res = s_read_until(curr_html, "class=\"scs-ch", &curr_html);
                        if (res == -1)
                            break;
                        //now if chv -> minor; chb -> alternative chord
                        if (curr_html[0] == 'v')
                        {
                            res = s_read_until(curr_html, ">", &curr_html);
                            if (res == -1)
                                break;
                            sscanf(curr_html, "%10[^<]", chord);
                            if (strcmp(chord, "m") == 0)
                            {
                                buffer_add(&buffer, 'm');
                                buffer_add(&buffer, 'i');
                            }
                            else
                            {
                                for (int i = 0; chord[i] != '\0'; i++)
                                    buffer_add(&buffer, chord[i]);
                            }
                        }
                        else if (curr_html[0] == 'b')
                        {
                            res = s_read_until(curr_html, ">", &curr_html);
                            if (res == -1)
                                break;
                            buffer_add(&buffer, '/');
                            sscanf(curr_html, "%10[^<]", chord);
                            for (int i = 0; chord[i] != '\0'; i++)
                                buffer_add(&buffer, chord[i]);
                        }

                        res = s_read_until(curr_html, ">", &curr_html);
                        if (res == -1)
                            break;
                        res = s_read_until(curr_html, "<", &curr_html);
                        if (res == -1)
                            break;
                    }
                    buffer_add(&buffer, ']');
                    s_read_until(curr_html, "/div>", &curr_html);
                    continue;
                }
            }
            //in this case, a char from the lyrics
            buffer_add(&buffer, curr_html[0]);
            curr_html++;
        }
        //close buffer and add to lines
        buffer.string[buffer.length] = '\0';
        if (!str_arr_add(&song_part->lines, buffer.string))
        {
            free(buffer.string);
            break;
        }
    }
    //after all of that, read to the end of </div>
    s_read_until(curr_html, ">", &curr_html);
    *read_until = curr_html;
    return true;
}

void buffer_add(DynBuff *buffer, char ch)
{
    if (buffer == NULL)
        return;
    if (buffer->string == NULL)
        return;

    //+1 for '\0'
    if (buffer->length + 1 >= buffer->capacity)
        if (!buffer_resize(buffer))
            return;
    
    buffer->string[buffer->length] = ch;
    buffer->length++;
}