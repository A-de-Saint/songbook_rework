#include "html_manager.h"
#include <stdlib.h>
#include "input_reader.h"
#include <string.h>

//adds a single song to songbook (of format HTML)
bool add_song_html(Songbook *songbook, Song *song)
{
    if (songbook == NULL || song == NULL)
        return false;
    if (songbook->path == NULL || song->data.parts == NULL || song->name_utf == NULL)
        return false;
    if (song->author_ascii == NULL || song->name_ascii == NULL || song->author_utf == NULL)
        return false;

    //build path to [songbook]/html
    Path *path = path_copy(songbook->path, false);
    if (path == NULL)
        return false;
    if (!path_add(path, "html", 'd'))
    {
        path_dtor(path);
        return false;
    }
    
    char *song_print = create_song_print(song->name_ascii, song->author_ascii);
    if (song_print == NULL)
    {
        path_dtor(path);
        return false;
    }

    bool res = html_add_song(path, song, song_print, songbook->type);
    path_dtor(path);
    free(song_print);

    return res;
}

//adds song to songs/[song].html
//does not check for NULLs
bool html_add_song(Path *html_path, Song *song, char *song_print, Type type)
{
    //make path to songs/[song].html
    Path *path = path_copy(html_path, false);
    if (path == NULL)
        return false;
    if (!path_add(path, "songs", 'd') ||
        !path_add(path, song_print, 'f') ||
        !path_add(path, ".html", 's'))
    {
        path_dtor(path);
        return false;
    }
    printf("%s\n", path->path); //testing//

    FILE *file = fopen(path->path, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open/create %s\n", path->path);
        path_dtor(path);
        return false;
    }
    path_dtor(path);

    float mm_size;
    bool split = get_horizontal_fontsize_split(&song->data, type, &mm_size);

    const char *types_html[] = {
        "verse", 
        "chorus",
        "intro",
        "interlude",
        "ending",
        "bridge",
        "other"
    };

    //add stuff before lyrics
    fprintf(file, "    <div class=\"page\">\n");
    fprintf(file, "        <div class=\"page-content\">\n");
    fprintf(file, "            <h3 class=\"song-name\" id=\"%s\">%s - %s</h3>\n", song_print, song->name_utf, song->author_utf);
    fprintf(file, "            <div class=\"song-content");
    if (split)
        fprintf(file, " split");
    fputc('\"', file);
    fprintf(file, " style=\"font-size: %fmm\">\n", mm_size);
    
    //print lines
    for (unsigned int i = 0; i < song->data.count; i++)
    {
        SongPart *part = &song->data.parts[i];
        fprintf(file, "                <div class=\"song-part %s\">\n", types_html[(int)part->type]);
        for (unsigned int j = 0; j < part->lines.size; j++)
        {
            fprintf(file, "                    ");
            fprintf(file, "<div class=\"lyrics\">");
            char *curr_line = part->lines.strings[j];

            int prev_chordlen = 0;
            int underchord_chars = 0;
            int temp = 0;
            bool reading_chord = false;
            bool prev_chord = false;

            for (int k = 0; curr_line[k] != '\0'; k++)
            {
                if (curr_line[k] == '[')
                {
                    if (type == type1)
                        fprintf(file, "<span");
                    else
                        fprintf(file, "<sup");
                    fprintf(file, " class=\"chord\"");
                    //calculate offset for back to back chords in type1
                    if (type == type1 && prev_chordlen > 0)
                    {
                        float offset = ((float)prev_chordlen - (float)underchord_chars * 0.75) / 2.0 + 0.85;
                        if (offset > 0)
                            fprintf(file, " style=\"margin-left: %fem\"", offset);
                    }
                    else if (prev_chord)
                        fprintf(file, "  ");
                    fputc('>', file);
                    reading_chord = true;
                    prev_chordlen = 0;
                    continue;
                }
                if (curr_line[k] == ']')
                {
                    fprintf(file, "</");
                    if (type == type1)
                        fprintf(file, "span>");
                    else
                        fprintf(file, "sup>");
                    reading_chord = false;
                    temp = prev_chordlen;
                    prev_chord = true;
                    continue;
                }

                fputc(curr_line[k], file);
                if (reading_chord)
                    prev_chordlen++;
                else
                {
                    prev_chord = false;
                    if (temp > 0)
                    {
                        temp--;
                        underchord_chars++;
                    }
                    else
                    {
                        prev_chordlen = 0;
                        underchord_chars = 0;
                    }
                }
            }
            fprintf(file, "</div>\n");
        }
        fprintf(file, "                </div>\n");
    }

    fprintf(file, "            </div>\n");
    fprintf(file, "        </div>\n");
    fprintf(file, "        <div class=\"page-number\"></div>");
    fprintf(file, "    </div>\n");

    fclose(file);

    return true;
}

//calculates maximal fontsize (vertically), returned to max_size
//returns false if no split is needed
//returns true if split is needed
//does not check for NULL
bool get_horizontal_fontsize_split(SongData *data, Type type, float *max_size)
{
    float line_height = 1.3;
    float break_height = 1.3;
    if (type == type1)
        line_height = 2.5;

    float current_height = 0;
    for (unsigned int i = 0; i < data->count; i++)
    {
        current_height += (float)(data->parts[i].lines.size) * line_height + break_height;
    }

    float mm_height = 245.0;
    float min_text_size = 5.0;

    if (type == type1)
        line_height = 2.5;
    else
        line_height = 1.3;

    //determine whether to split or not
    bool split = false;
    if (current_height * min_text_size > mm_height)
    {
        split = true;
        float split_height = 0;
        float target = current_height / 2;
        for (unsigned int i = 0; i < data->count; i++)
        {
            split_height += (float)(data->parts[i].lines.size) * line_height + break_height;
            if (split_height >= target)
            {
                current_height = split_height;
                break;
            }
        }
    }

    *max_size = mm_height / current_height;

    return split;
}

//opens temp and writes to the part where songs are to be inserted
//returns FILE *temp, in which the start is already written
//fopens template temp and saves it to FILE *template_f (caller closes)
//temp_path is to html/temp/temp.html, template_path is to templates/html/temp.html
FILE *open_temp_init(Path *temp_path, Path *template_path, FILE *template_f)
{
    FILE *temp = fopen(temp_path->path, "w");
    if (temp == NULL)
    {
        fprintf(stderr, "Could not write file to %s\n", temp_path->path);
        return NULL;
    }

    #if 0
    Path *template = path_copy(songbook_path, false);
    if (template == NULL)
    {
        fclose(temp);
        return NULL;
    }
    path_dirback(template);
    if (!path_add(template, "templates", 'd') ||
        !path_add(template, "html", 'd') ||
        !path_add(template, "temp.html", 'f'))
    {
        fclose(temp);
        path_dtor(template);
        return NULL;
    }
    #endif

    template_f = fopen(template_path->path, "r");
    if (template_f == NULL)
    {
        fclose(temp);
        return NULL;
    }

    char *line;
    bool found = false;
    while ((line = read_line(template_f)) != NULL)
    {
        if (strcmp(line, "<!--songs-->") == 0)
        {
            found = true;
            free(line);
            break;
        }
        fprintf(temp, "%s\n", line);
        free(line);
    }

    if (!found)
    {
        fclose(template_f);
        fclose(temp);
        return NULL;
    }

    return temp;
}

//writes the rest of template to temp and fcloses both files
//if return == false, files still closed
bool close_temp_end(FILE *temp, FILE *template)
{
    if (temp == NULL || template == NULL)
    {
        if (temp != NULL)
            fclose(temp);
        if (template != NULL)
            fclose(template);
        return false;
    }

    char *line;
    while ((line = read_line(template)) != NULL)
    {
        fprintf(temp, "%s\n", line);
        free(line);
    }

    fclose(temp);
    fclose(template);
    return true;
}

//truncates temp (removes all contents)
void reset_temp(Path *temp_path)
{
    FILE *file = fopen(temp_path->path, "w");
    if (file != NULL)
        fclose(file);
}

//builds toc based on song_collection
//toc must be open for write, songlist must be open for read
//returns page number of the first song
//returns -1 if failed
int build_toc_html(FILE *toc, FILE *songlist)
{
    if (toc == NULL || songlist == NULL)
        return -1;

    StringArray str_arr = str_arr_ctor(64);
    if (str_arr.strings == NULL)
        return -1;

    //read all songs
    char *line;
    while ((line = read_line(songlist)) != NULL)
    {
        if (*line == '\0')
        {
            free(line);
            continue;
        }
        str_arr_add(&str_arr, line);
    }

    //calculate number of pages
    //rules: fill first page fits 34 entries, 68 in split mode
    //calculated using: (page_content_height - pagenumber_height - title_height) / (entry_line_height*entry_font_size + entry_margin_bottom)
    //if first page splits, every next page should also split
    //next pages fit 36 entries therefore 72 (because split mode is mandatory)
    int first_page_capacity = 68;
    int page_capacity = 72;

    bool split = true;
    int entries_count = (int)str_arr.size;
    if (entries_count <= (first_page_capacity / 2))
        split = false;
    int page_count = 1; //1 for first toc page
    entries_count -= first_page_capacity;
    while (entries_count < 0)
    {
        page_count++;
        entries_count -= page_capacity;
    }

    //write toc start
    bool first_page = true;
    unsigned int entry_idx = 0;
    for (int i = 1; i <= page_count; i++)
    {
        fprintf(toc, "    <div class=\"page toc-page\">\n");
        fprintf(toc, "        <div class=\"page-content\">\n");
        if (first_page)
            fprintf(toc, "            <h2 id=\"contents-name\">Obsah:</h2>\n");
        
        fprintf(toc, "            <ul class=\"toc");
        if (split)
            fprintf(toc, " toc-split");
        fputc('\"', toc);
        if (first_page)
            fprintf(toc, " id=\"toc-first\"");
        fprintf(toc, ">\n");
        
        int page_entries = first_page ? first_page_capacity : page_capacity;
        while (page_entries > 0)
        {
            if (entry_idx >= str_arr.size)
                break;

            char *print;
            char *author;
            char *name;
            if (!decode_song_print_extended(str_arr.strings[entry_idx], &print, &name, &author))
            {
                fprintf(stderr, "build_toc: Could not decode %s\n", str_arr.strings[entry_idx]);
                entry_idx++;
                page_entries--;
                continue;
            }
            
            fprintf(toc, "                <li>\n");
            fprintf(toc, "                    <a href=\"#%s\">\n", print);
            fprintf(toc, "                        <span class=\"toc-title\">%s - %s</span>\n", name, author);
            free(print);
            free(author);
            free(name);
            fprintf(toc, "                        <span class=\"toc-dots\"></span>\n");
            fprintf(toc, "                        <span class=\"toc-pagenumber\">%d</span>\n", page_count+entry_idx+2);
            fprintf(toc, "                    </a>\n");
            fprintf(toc, "                </li>\n");

            entry_idx++;
            page_entries--;
        }

        fprintf(toc, "            </ul>\n");
        fprintf(toc, "        </div>\n");
        fprintf(toc, "        <div class=\"page-number\">%d</div>\n", i+1); //+1 for title_page (is is pagenumber here)
        fprintf(toc, "    </div>\n");

        if (first_page)
            first_page = false;
    }

    return page_count + 2; //+1 for titlepage, +1 because return val is next page
}