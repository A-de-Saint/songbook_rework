#include "html_manager.h"
#include <stdlib.h>
#include "input_reader.h"
#include <string.h>
#include <stdio.h>
#include "ak_parser.h"

//adds a single song to songbook (of format HTML)
//takes care of adding the song to songlist
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

    //add song to songlist
    if (res)
    {
        res = add_song_songlist_extended(song->name_ascii, song->author_ascii,
                song->name_utf, song->author_utf, songbook->path);
    }

    //measure song fontsize
    if (res)
    {
        if (!path_add(path, "songs", 'd') ||
            !path_add(path, song_print, 'f') ||
            !path_add(path, ".html", 's'))
        {
            res = false;
            goto add_end;
        }

        FILE *song = fopen(path->path, "r");
        if (song == NULL)
        {
            res = false;
            goto add_end;
        }

        SongFiles sf;
        song_files_ctor(&sf, 1);
        if (!song_files_add(&sf, song, path))
        {
            res = false;
            fclose(song);
            goto add_end;
        }

        Path *main_path = path_copy(songbook->path, false);
        if (!path_add(main_path, "html", 'd') ||
            !path_add(main_path, songbook->name, 'f') ||
            !path_add(main_path, ".html", 's'))
        {
            res = false;
            song_files_dtor(&sf);
            path_dtor(main_path);
            fclose(song);
            goto add_end;
        }

        res = fix_fontsizes_fclose(&sf, main_path);
        path_dtor(main_path);
        printf("Did it return early? Fuck...\n"); //testing//
        if (!res)
            fclose(song);
        song_files_dtor(&sf);
    }

add_end:
    path_dtor(path);
    free(song_print);
    printf("got here?\n"); //testing//

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
    fprintf(file, "        <div class=\"page-number\"></div>\n");
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
    while (entries_count > 0)
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

    str_arr_dtor(&str_arr);

    return page_count + 2; //+1 for titlepage, +1 because return val is next page
}

//adds songs to temp (main must be open for read, temp must be opened for write)
//temp is based off main for some fonts
bool add_songs_to_temp(SongFiles *song_files, FILE *temp, FILE *main)
{
    printf("I got here at least\n"); //testing//
    if (song_files == NULL || temp == NULL || song_files->songs == NULL)
        return false;

    //copy main until <body>
    char *line;
    bool added_style = false;
    while ((line = read_line(main)) != NULL)
    {
        if (*line == '\0')
        {
            free(line);
            continue;
        }
        fprintf(temp, "%s\n", line);
        if (!added_style && (strstr(line, "<head>") != NULL))
        {
            fprintf(temp, "    <link rel=\"stylesheet\" type=\"text/css\" href=\"../style.css\">");
            added_style = true;
        }
        bool end = strstr(line, "<body>") != NULL;
        free(line);
        if (end)
            break;
    }

    //copy all songs into temp
    int c;
    for (unsigned int i = 0; i < song_files->count; i++)
    {
        FILE *file = song_files->songs[i];
        while ((c = fgetc(file)) != EOF)
            fputc(c, temp);
    }

    //end the stuff
    fprintf(temp, "</body>\n");
    fprintf(temp, "<script src=\"fitDocument.js\"></script>\n");
    fprintf(temp, "</html>\n");

    return true;
}

//pipes to chrome, gets DOM and parses calculated values
//returns number of successfully parsed floats
//returns floats in results - MUST BE ALLOCATED BEFORE
int get_temp_results(Path *temp_path, float *results, int results_count)
{
    printf("Did get here\n"); //testing//
    FILE *DOM = NULL;

    #ifdef _WIN32
        //TODO Windows version
    #else
        //TODO support more commands (chromium, chrome...)
        const char *command_names[] = {"google-chrome ", "chromium ", "chromium-browser "};
        int command_count = 3;
        const char *flags = "--headless=new --disable-gpu --no-sandbox --dump-dom --virtual-time-budget=15000 --run-all-compositor-stages-before-draw ";
        const char *silence = " 2>/dev/null";
        char *command = malloc(32 + strlen(flags) + strlen(temp_path->path) + strlen(silence) + 1); //32 represents command_name limit
        if (command == NULL)
            return -1;
        
        for (int i = 0; i < command_count; i++)
        {
            strcpy(command, command_names[i]);
            strcat(command, flags);
            strcat(command, temp_path->path);
            strcat(command, silence);
            DOM = popen(command, "r");
            if (DOM != NULL)
            {
                free(command);
                break;
            }
        }
        
    #endif
    
    if (DOM == NULL)
        return -1;

    //read computed values from DOM and save to results
    int i = 0;
    char buffer[16];
    while (i < results_count)
    {
        if (!read_until(DOM, "song-content"))
            break;
        if (!read_until(DOM, "font-size:"))
            break;
        
        int c;
        int j = 0;
        while ((c = fgetc(DOM)) != EOF)
        {
            if (c == ' ')
                continue;
            if (c == '.')
            {
                buffer[j++] = '.';
                continue;
            }
            if (c < '0' || c > '9')
                break;

            if (j < (int)(sizeof(buffer) - 1))
                buffer[j++] = c;
        }
        buffer[j] = '\0';
        char *check;
        float f = 12.0; //in case it were to be saved as garbage
        f = strtof(buffer, &check);
        if (*check != '\0')
        {
            fprintf(stderr, "Could not parse '%s' into a float", buffer);
            if (f < 12.0) //TODO check if this is good
                f = 12.0;
        }
        results[i] = f;
        i++;
    }

    #ifdef _WIN32
        //TODO Windows pclose
    #else
        pclose(DOM);
    #endif

    return i;
}

//fixes fontsizes for selected songs
//songs must be a collection of valid fopened (for read) songs
//songs WILL be closed if true
//songs WILL NOT be closed if false
bool fix_fontsizes_fclose(SongFiles *songs, Path *main_path)
{
    printf("Fixing fontsizes...\n");
    if (songs == NULL || main_path == NULL)
        return false;

    printf("here???\n"); //testing//
    printf("%s\n", main_path->path);
    //open main
    FILE *main = fopen(main_path->path, "r");
    if (main == NULL)
        return false;

    printf("here\n"); //testing//

    //open temp
    Path *path = path_copy(main_path, false);
    if (path == NULL)
    {
        fclose(main);
        return false;
    }
    path_dirback(path); //now at songbook/html
    if (!path_add(path, "temp", 'd') ||
        !path_add(path, "temp.html", 'f'))
    {
        fclose(main);
        path_dtor(path);
        return false;
    }
    printf("here\n"); //testing//
    FILE *temp = fopen(path->path, "w");
    if (temp == NULL)
    {
        path_dtor(path);
        fclose(main);
        return false;
    }

    printf("Here as well\n"); //testing//

    bool res = add_songs_to_temp(songs, temp, main);
    fclose(main);
    fclose(temp);

    printf("surely not here tho\n"); //testing//

    if (!res)
    {
        path_dtor(path);
        return false;
    }

    reset_files(songs); //resets back, so they can be read again

    //create float buffer for results
    int fl_arr_size = (int)songs->count;
    float *fl_arr = malloc(fl_arr_size * sizeof(float));
    if (fl_arr == NULL)
    {
        path_dtor(path);
        return false;
    }

    int successful = get_temp_results(path, fl_arr, fl_arr_size);
    path_dtor(path);
    if (successful == -1)
    {
        fprintf(stderr, "Could not open temp.html and measure song fontsizes\n");
        free(fl_arr);
        return false;
    }

    if (successful < fl_arr_size)
    {
        fprintf(stderr, "WARN: %d song fontsizes to be measured, only %d really measured\n", fl_arr_size, successful);
    }

    //just to be sure, shouldn't happen tho
    if (successful > fl_arr_size)
        successful = fl_arr_size;

    //update songs' fontsizes
    for (int i = 0; i < successful; i++)
    {
        FILE *song = songs->songs[i];
        char *line;
        bool updated = false;
        StringArray str_arr = str_arr_ctor(128);
        while ((line = read_line(song)) != NULL)
        {
            if (!updated)
            {
                //try find font-size in song-content
                char *breakpoint;
                if (s_read_until(line, "song-content", &breakpoint) != -1)
                {
                    line = realloc(line, strlen(line) + 16); //make room for the float
                    if (s_read_until(line, "font-size:", &breakpoint) != -1)
                    {
                        //print to the part of the string
                        sprintf(breakpoint, " %fpx;\">", fl_arr[i]);
                        updated = true;
                    }
                }
            }
            str_arr_add(&str_arr, line);
        }
        if (!updated)
        {
            fprintf(stderr, "The song at %s is missing 'song-content' or 'font-size'\n", songs->paths[i]);
            fclose(song);
            str_arr_dtor(&str_arr);
            continue;
        }

        //reopen song for write and... write
        fclose(song);
        song = fopen(songs->paths[i], "w");
        if (song == NULL)
        {
            fprintf(stderr, "Could not reopen %s\n", songs->paths[i]);
            str_arr_dtor(&str_arr);
            continue;
        }

        for (unsigned int j = 0; j < str_arr.size; j++)
        {
            fprintf(song, "%s\n", str_arr.strings[j]);
        }
        str_arr_dtor(&str_arr);
        fclose(song);
    }

    free(fl_arr);
    return true;
}

bool build_main_html(Path *html_path, char *songbook_name, int first_songpage)
{
    Path *path = path_copy(html_path, false);
    if (path == NULL)
        return false;
    
    //open main
    if (!path_add(path, songbook_name, 'f') ||
        !path_add(path, ".html", 's'))
    {
        path_dtor(path);
        return false;
    }
    FILE *main_r = fopen(path->path, "r");
    if (main_r == NULL)
    {
        fprintf(stderr, "Could not open %s\n", path->path);
        path_dtor(path);
        return false;
    }

    //buffers for main parts
    StringArray main_pre = str_arr_ctor(32);
    StringArray main_post = str_arr_ctor(8);

    char *line;
    //read pre-body
    while ((line = read_line(main_r)) != NULL)
    {
        str_arr_add(&main_pre, line);
        if (strstr(line, "<!--content-->") != NULL)
            break;
    }
    if (line == NULL)
    {
        fprintf(stderr, "Could not parse main HTML properly\n");
        str_arr_dtor(&main_pre);
        str_arr_dtor(&main_post);
        fclose(main_r);
        path_dtor(path);
        return false;
    }

    //skip body
    while ((line = read_line(main_r)) != NULL)
    {
        if (strstr(line, "</body>") != NULL)
            break;
        free(line);
    }
    if (line == NULL)
    {
        fprintf(stderr, "Could not parse main HTML properly\n");
        str_arr_dtor(&main_pre);
        str_arr_dtor(&main_post);
        fclose(main_r);
        path_dtor(path);
        return false;
    }

    //add post-body
    str_arr_add(&main_post, line);
    while((line = read_line(main_r)) != NULL)
    {
        str_arr_add(&main_post, line);
    }

    fclose(main_r);

    //open main for write
    FILE *main_w = fopen(path->path, "w");
    if (main_w == NULL)
    {
        str_arr_dtor(&main_pre);
        str_arr_dtor(&main_post);
        path_dtor(path);
        return false;
    }

    //copy main-pre
    for (unsigned int i = 0; i < main_pre.size; i++)
    {
        fprintf(main_w, "%s\n", main_pre.strings[i]);
    }
    str_arr_dtor(&main_pre);

    //add toc
    path_dirback(path); //now at html
    if (!path_add(path, "toc.html", 'f'))
        goto post_toc;
    FILE *toc = fopen(path->path, "r");
    if (toc == NULL)
    {
        path_dirback(path); //now at html
        goto post_toc;
    }
    
    while ((line = read_line(toc)) != NULL)
    {
        fprintf(main_w, "%s\n", line);
        free(line);
    }
    fclose(toc);
    path_dirback(path); //now at html

post_toc:

    //get to songlist.txt (remember that this one is extended)
    Path *songlist_path = path_copy(path, 'f');
    if (songlist_path == NULL)
        goto post_songs;
    path_dirback(songlist_path); //now at [songbook]/
    if (!path_add(songlist_path, "songlist.txt", 'f'))
    {
        path_dtor(songlist_path);
        goto post_songs;
    }
    FILE *songlist = fopen(songlist_path->path, "r");
    path_dtor(songlist_path);
    if (songlist == NULL)
    {
        fprintf(stderr, "Could not open songlist at %s\n", songlist_path->path);
        goto post_songs;
    }

    //build path to html/songs
    Path *song_path = path_copy(path, false);
    if (!path_add(song_path, "songs", 'd'))
    {
        path_dtor(song_path);
        fclose(songlist);
        goto post_songs;
    }

    //get individual songs
    int i = 0;
    while ((line = read_line(songlist)) != NULL)
    {
        if (*line == '\0')
        {
            free(line);
            continue;
        }

        //decode line
        char *song_print;
        char *author;
        char *name;
        if (!decode_song_print_extended(line, &song_print, &name, &author))
        {
            fprintf(stderr, "Could not decode %s\n", line);
            goto loop_end;
        }
        free(name);
        free(author);

        //open song
        if (!path_add(song_path, song_print, 'f') ||
            !path_add(song_path, ".html", 's'))
        {
            free(song_print);
            goto loop_end;
        }
        free(song_print);
        FILE *song = fopen(song_path->path, "r");
        if (song == NULL)
        {
            fprintf(stderr, "Could not open %s\n", song_path->path);
            goto loop_end;
        }

        //copy song and add page number
        char *ln;
        while ((ln = read_line(song)) != NULL)
        {
            //add pagenumber
            char *breakpoint;
            if (s_read_until(ln, "class=\"page-number\">", &breakpoint) != -1)
            {
                char *ln_cpy = ln;
                while (ln_cpy != breakpoint)
                {
                    fputc(*ln_cpy, main_w);
                    ln_cpy++;
                }
                fprintf(main_w, "%d%s\n", first_songpage + i, breakpoint);
                free(ln);
                continue;
            }
            fprintf(main_w, "%s\n", ln);
            free(ln);
        }

        fclose(song);
        path_dirback(song_path);

    loop_end:
        free(line);
        i++;
    }

    fclose(songlist);
    path_dtor(song_path);

post_songs:

    //print post_body
    for (unsigned int i = 0; i < main_post.size; i++)
    {
        fprintf(main_w, "%s\n", main_post.strings[i]);
    }
    str_arr_dtor(&main_post);
    path_dtor(path);
    fclose(main_w);

    return true;
}

//builds table of contents and finishes main
bool html_compile(Songbook *songbook)
{
    Path *path = path_copy(songbook->path, false);
    if (path == NULL)
        return false;

    //open songlist.txt
    if (!path_add(path, "songlist.txt", 'f'))
    {
        path_dtor(path);
        return false;
    }
    FILE *songlist = fopen(path->path, "r");
    if (songlist == NULL)
    {
        fprintf(stderr, "Could not open %s\n", path->path);
        path_dtor(path);
        return false;
    }
    path_dirback(path); //now at [songbook]

    //open toc.html
    if (!path_add(path, "html", 'd') ||
        !path_add(path, "toc.html", 'f')) //now at [songbook]/html/toc.html
    {
        path_dtor(path);
        fclose(songlist);
        return false;
    }
    FILE *toc = fopen(path->path, "w");
    if (toc == NULL)
    {
        fprintf(stderr, "Could not open %s\n", path->path);
        fclose(songlist);
        path_dtor(path);
        return false;
    }

    int firstpage = build_toc_html(toc, songlist);
    fclose(toc);
    fclose(songlist);

    printf("toc added\n"); //testing//

    if (firstpage == -1)
    {
        fprintf(stderr, "Failed to create table of contents\n");
        path_dtor(path);
        return false;
    }

    path_dirback(path); //now at [songbook]/html

    bool res = build_main_html(path, songbook->name, firstpage);

    path_dtor(path);
    return res;
}