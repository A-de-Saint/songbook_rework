#include "html_manager.h"

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
    fprintf(file, "            <h3 class=\"song-name\">%s - %s</h3>\n", song->name_utf, song->author_utf);
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