#include "transpose.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const int chart_size = 12;
const char *trans_chart[] = {"A", "B", "H", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

//transposes song into desired first chord
bool transpose_song(Song *song, char *transpose_to)
{
    if (song == NULL || transpose_to == NULL)
        return false;

    int trans_idx = 0;
    bool trans_idx_acq = false;
    for (unsigned int i = 0; i < song->data.count; i++)
    {
        SongPart *curr_part = &song->data.parts[i];
        for (unsigned int j = 0; j < curr_part->lines.size; j++)
        {
            char *curr_line = curr_part->lines.strings[j];

            //main part of a chord ("F#" from "F#misus2add9")
            char chord_main_part[8] = {'\0'};
            //rest of the chord ("misus2add9" from "F#misus2add9")
            char chord_rest[16] = {'\0'};
            //allocated buffer for transposed line
            char *repl_line = malloc(strlen(curr_line) + 32);

            int chord_read = -1; //-1 means no chord is being read, otherwise it's the cursor in the chord
            int rest_curs = 0; //cursor of chord_rest
            int curr_line_curs = 0; //reading cursor
            int repl_line_curs = 0; //writing cursor

            while (curr_line[curr_line_curs] != '\0')
            {
                if (chord_read == -1)
                {
                    if (curr_line[curr_line_curs] == '[')
                    {
                        repl_line[repl_line_curs++] = '[';
                        chord_read = 0;
                    }
                    else 
                    {
                        repl_line[repl_line_curs++] = curr_line[curr_line_curs];
                    }
                }
                else
                {
                    if (curr_line[curr_line_curs] == ']' || curr_line[curr_line_curs] == '/')
                    {
                        //if first chord, get transposition index first
                        if (!trans_idx_acq)
                        {
                            if (!get_trans_idx(chord_main_part, transpose_to, &trans_idx))
                            {
                                free(repl_line);
                                return false;
                            }
                            trans_idx_acq = true;
                        }
                        chord_rest[rest_curs] = '\0';
                        if (!transpose_chord(chord_main_part, trans_idx))
                        {
                            free(repl_line);
                            return false;
                        }
                        put_chord(repl_line, &repl_line_curs, chord_main_part, chord_rest);
                        rest_curs = 0;
                        repl_line[repl_line_curs++] = curr_line[curr_line_curs];
                        if (curr_line[curr_line_curs] == '/')
                            chord_read = 0;
                        else
                            chord_read = -1;
                    }
                    else if (chord_read == 0)
                    {
                        chord_main_part[0] = curr_line[curr_line_curs];
                        chord_main_part[1] = '\0';
                        chord_read++;
                    }
                    else if (chord_read == 1)
                    {
                        if (curr_line[curr_line_curs] == '#')
                        {
                            chord_main_part[1] = '#';
                            chord_main_part[2] = '\0';
                        }
                        else 
                            chord_rest[rest_curs++] = curr_line[curr_line_curs];
                        chord_read++;
                    }
                    else
                    {
                        chord_rest[rest_curs++] = curr_line[curr_line_curs];
                    }
                }

                curr_line_curs++;
            }
            free(curr_line);
            repl_line[repl_line_curs] = '\0';
            curr_part->lines.strings[j] = repl_line;
        }
    }
    return true;
}

//attempts to get transposition shift, saves to trans_idx
bool get_trans_idx(char *first_chord, char *transpose_to, int *trans_idx)
{
    int start_chord_idx = -1;
    int transposition_idx = -1;
    for (int i = 0; i < chart_size; i++)
    {
        if (strcmp(trans_chart[i], first_chord) == 0)
            start_chord_idx = i;
        if (strcmp(trans_chart[i], transpose_to) == 0)
            transposition_idx = i;
    }

    //check if chord was found in transposition chart
    if (start_chord_idx < 0)
    {
        fprintf(stderr, "Transposition failed: Unknown first chord: %s\n", first_chord);
        return false;
    }
    if (transposition_idx < 0)
    {
        fprintf(stderr, "Transposition failed: Unknown transposition chord: %s\n", transpose_to);
        return false;
    }

    *trans_idx = transposition_idx - start_chord_idx;
    printf("Idx is: %d\n", *trans_idx);
    return true;
}

//puts the chord where it should be in the repl_line
void put_chord(char *repl_line, int *cursor, char *main_part, char *rest)
{
    for (int i = 0; main_part[i] != '\0'; i++)
    {
        repl_line[*cursor] = main_part[i];
        (*cursor)++;
    }
    for (int i = 0; rest[i] != '\0'; i++)
    {
        repl_line[*cursor] = rest[i];
        (*cursor)++;
    }
}

//transposes a single chord
bool transpose_chord(char *chord, int trans_idx)
{
    printf("Transposing this chord: %s\n", chord);
    int chord_idx = -1;
    for (int i = 0; i < chart_size; i++)
    {
        if (strcmp(trans_chart[i], chord) == 0)
            chord_idx = i;
    }
    if (chord_idx < 0)
    {
        fprintf(stderr, "Transposition failed: Unsupported chord: %s\n", chord);
        return false;
    }
    int new_idx = ((chord_idx + trans_idx) + chart_size) % chart_size;
    printf("New_idx is: %d\n", new_idx);
    strcpy(chord, trans_chart[new_idx]);
    return true;
}