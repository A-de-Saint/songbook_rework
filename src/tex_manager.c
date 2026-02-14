#include "tex_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//adds tex files for song to a tex songbook
bool add_song_tex(Songbook *songbook, Song *song)
{
    if (songbook == NULL || song == NULL)
        return false;
    if (songbook->path == NULL || song->author_utf == NULL || song->author_ascii == NULL)
        return false;
    if (song->name_ascii == NULL || song->name_utf == NULL || song->data.parts == NULL)
        return false;

    //build path to songbook/tex_files/
    Path *tex_path = path_copy(songbook->path, false);
    if (tex_path == NULL)
        return false;
    if (!path_add(tex_path, "tex_files", 'd'))
    {
        path_dtor(tex_path);
        return false;
    }

    char *print = create_song_print(song->name_ascii, song->author_ascii);
    if (print == NULL)
    {
        path_dtor(tex_path);
        return false;
    }

    //create [song].tex
    bool res = create_tex_song(song, print, tex_path, songbook->type);
    if (!res)
    {
        path_dtor(tex_path);
        free(print);
        return false;
    }

    //insert song to songs.tex
    res = add_song_to_main(tex_path, print);
    path_dtor(tex_path);
    free(print);

    return res;
}

//creates [song_print].tex file in given songbook of a given song
//tex_path must be at [songbook]/tex_files
//no NULL checks - check beforehand
bool create_tex_song(Song *song, char *song_print, Path *tex_path, Type type)
{
    //build path to song (tex_files/songs/[song_print].tex) and open file
    Path *song_path = path_copy(tex_path, false);
    if (song_path == NULL)
        return false;
    if (!path_add(song_path, "songs", 'd') ||
        !path_add(song_path, song_print, 'f') ||
        !path_add(song_path, ".tex", 's'))
    {
        path_dtor(song_path);
        return false;
    }
    FILE *file = fopen(song_path->path, "w");
    path_dtor(song_path);
    if (file == NULL)
        return false;

    //string literals for types
    const char *types_tex[] = {
        "song_verse", 
        "song_chorus",
        "song_intro",
        "song_interlude",
        "song_ending",
        "song_bridge",
        "song_other"
    };

    //get values for song size and split info
    int values[2];
    int split = get_font_size_split(&song->data, type, values);

    //print the song into the file
    fprintf(file, "{\\songsize{%dpt}{%dpt}\n", values[0], values[1]);
    fprintf(file, "\\songtitle{%s - %s}\n", song->name_utf, song->author_utf);
    fprintf(file, "\\begin{song}\n");
    if (split != -1)
        fprintf(file, "\\begin{minipage}[t]{0.48\\textwidth}\n");
    for (unsigned int i = 0; i < song->data.count; i++)
    {
        if (split == (int)i)
            fprintf(file, "\\end{minipage}\n\\hfill\n\\begin{minipage}[t]{0.48\\textwidth}\n");
        SongPart *curr_part = &song->data.parts[i];
        //print part container
        fprintf(file, "\\begin{%s}\n", types_tex[(int)curr_part->type]);

        //print individual lines
        for (unsigned int j = 0; j < curr_part->lines.size; j++)
        {
            char *curr_line = curr_part->lines.strings[j];
            fprintf(file, "\\lyrics{");

            //stuff for putting whitespace between back-to-back chords
            bool reading_chord = false;
            int chordlen = 0;
            bool prev_chord = false;

            for (int k = 0; curr_line[k] != '\0'; k++)
            {
                if (curr_line[k] == '[')
                {
                    if (prev_chord) //put chordlen whitespaces between back-to-back chords
                    {
                        if (curr_part->type == INTRO || curr_part->type == INTERLUDE || curr_part->type == ENDING)
                            if (chordlen < 3)
                                chordlen = 3;
                        for (int l = 0; l < chordlen; l++)
                            fprintf(file, "\\ ");
                        chordlen = 0;
                    }
                    fprintf(file, "{\\chord{");
                    reading_chord = true;
                    continue;
                }
                if (curr_line[k] == ']')
                {
                    fprintf(file, "}}");
                    reading_chord = false;
                    prev_chord = true;
                    continue;
                }
                if (curr_line[k] == '\\')
                {
                    fprintf(file, "\\textbackslash");
                    goto after_write;
                }
                if (curr_line[k] == '#' || curr_line[k] == '_') //escape chars
                    fputc('\\', file);
                fputc(curr_line[k], file);
            after_write:
                if (reading_chord)
                {
                    chordlen++;
                }
                else if (prev_chord)
                {
                    chordlen = 0;
                    prev_chord = false;
                }
            }
            fprintf(file, "}\n");
        }

        fprintf(file, "\\end{%s}\n", types_tex[(int)curr_part->type]);
    }
    if (split != -1)
        fprintf(file, "\\end{minipage}\n");
    fprintf(file, "\\end{song}\n");
    fprintf(file, "}\n");

    fclose(file);
    return true;
}

//calculates font_size based off row and column sizes
//returns number of part before which to split the page
//returns -1 if no split needed
//fills calculated values in values
int get_font_size_split(SongData *song_data, Type type, int values[2])
{
    int max_len = 0;
    int row_count = 0;
    for (unsigned int i = 0; i < song_data->count; i++)
    {
        SongPart *part = &song_data->parts[i];
        //the ratio for top-index stuff should be about 1.5, therefore +2 on odds, +1 on evens
        //which is why this:
        bool oddLine = true;
        for (unsigned int j = 0; j < part->lines.size; j++)
        {
            char *line = part->lines.strings[j];
            int len = 0;
            bool chord_reading = false;
            while (*line != '\0')
            {
                //skip []
                if (*line == '[')
                {
                    chord_reading = true;
                    line++;
                    continue;
                }
                if (*line == ']')
                {
                    chord_reading = false;
                    line++;
                    continue;
                }

                //get charlen (for utf)
                int charlen = utf8_char_length((unsigned char*)line);
                if (charlen == 0)
                    break;
                line += charlen;
                if (!(chord_reading && type == type1)) //don't read chord conts in type 1
                    len++;
            }
            if (len > max_len)
                max_len = len;

            //increase rows based on type
            if (type == type1 || oddLine)
            {
                row_count += 2;
                oddLine = false;
            }
            else 
            {
                oddLine = true;
                row_count++;
            }
        }
        row_count++;
    }
    //reduce row_count, because there is no space after the last part
    if (row_count > 0)
        row_count--;

    //magic (measured) constants for max font size
    const int cols_constant = 1000;
    const int rows_constant = 710;
    const float first_to_second_ratio = 1.3;
    const int max_allowed_text_size = 18;
    int split_cols_const = 470;
    
    //this approach calculates both split and no-split and returns what is greater
    //calculate split stuff
    unsigned int target = (unsigned)row_count / 2;
    unsigned int measured = 0;
    int return_val = -1;
    int rows_split = row_count;
    for (unsigned int i = 0; i < song_data->count; i++)
    {
        if (type == type1)
            measured += song_data->parts[i].lines.size * 2 + 1;
        else
            measured += (unsigned int)((float)song_data->parts[i].lines.size * 1.5) + 1;

        if (measured >= target)
        {
            if (i + 1 < song_data->count)
                return_val = (int)i+1;
            else 
                return_val = (int)i;
            rows_split = (int)measured; //real row_count in case of split
            break;
        }
    }

    //calculate maximal possible split font size
    int split_x_res = split_cols_const / max_len;
    int split_y_res = rows_constant / rows_split;
    int split_res = split_x_res < split_y_res ? split_x_res : split_y_res;

    //calculate maximal possible nosplit font size
    int nosplit_x_res = cols_constant / max_len;
    int nosplit_y_res = rows_constant / row_count;
    int nosplit_res = nosplit_x_res < nosplit_y_res ? nosplit_x_res : nosplit_y_res;

    if (nosplit_res >= split_res)
    {
        return_val = -1;
        values[0] = nosplit_res;
    }
    else
        values[0] = split_res;

    if (values[0] > max_allowed_text_size)
        values[0] = max_allowed_text_size;

    values[1] = (int)((float)values[0] * first_to_second_ratio);

    return return_val;
}

//adds song to songs.tex (which gets inputted into main.tex)
//tex path must be a path to [songbook]/tex_files
bool add_song_to_main(Path *tex_path, char *song_print)
{
    if (tex_path == NULL || song_print == NULL)
        return false;
    if (tex_path->path == NULL)
        return false;

    //create path to tex_files/songs.tex
    Path *path = path_copy(tex_path, false);
    if (path == NULL)
        return false;
    if (!path_add(path, "songs.tex", 'f'))
    {
        path_dtor(path);
        return false;
    }

    //build line to insert
    const char *command = "\\input{songs/";
    const char *suffix = ".tex}";

    char *full_line = malloc(strlen(command) + strlen(suffix) + strlen(song_print) + 1);
    if (full_line == NULL)
    {
        path_dtor(path);
        return false;
    }
    strcpy(full_line, command);
    strcat(full_line, song_print);
    strcat(full_line, suffix);

    bool res = read_insert_write(path, full_line);

    path_dtor(path);
    free(full_line);
    return res;
}

//clanker-generated utf8 char length
unsigned int utf8_char_length(const unsigned char *s)
{
    if ((s[0] & 0x80) == 0x00) return 1;      // 0xxxxxxx
    if ((s[0] & 0xE0) == 0xC0) return 2;      // 110xxxxx
    if ((s[0] & 0xF0) == 0xE0) return 3;      // 1110xxxx
    if ((s[0] & 0xF8) == 0xF0) return 4;      // 11110xxx
    return 0; // invalid UTF-8 start byte
}