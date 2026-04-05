#include "menu.h"
#include "input_reader.h"
#include "songbook_manager.h"
#include "util.h"
#include <string.h>
#include "ak_parser.h"
#include "remover.h"

#define OPTIONS_START_CAPACITY 8

//reads one non-negative integer from stdin
//returns -1 if illegal
int readnum(int min, int max)
{
    if (min < 0)
        return -1;
    if (max < min)
        return -1;

    char *line = read_line(stdin);
    if (line == NULL)
        return -1;

    char *c;
    int num = strtol(line, &c, 10);

    if (*c != '\0')
    {
        free(line);
        return -1;
    }
    free(line);
    if (num < min || num > max)
        return -1;
    return num;
}

//shows first menu screen and reads input
ActionChoice action_choice()
{
    putchar('\n');
    printf("Choose action you wish to do:\n");

    printf("[0] Exit program\n");
    printf("[1] Add new songbook\n");
    printf("[2] Edit songs in an existing songbook\n");
    printf("[3] Remove songbook\n");
    printf("[4] Add song to global song_collection (transposition possible)\n");
    printf("Your choice: ");

    int choice;
    while ((choice = readnum(0, 4)) == -1)
        printf("Invalid input.\nTry again: ");

    return choice;
}

bool new_songbook(Path *home_path)
{
    putchar('\n');
    printf("NEW SONGBOOK\n");

    Songbook songbook;

    //create path
    printf("Choose name: ");
    char c;
    char *name = read_line(stdin);
    if (name == NULL)
        return false;
    trim_sides(name);
    //check name for empty string or invalid chars
    while ((c = check_name(name)) != '\0' || name[0] == '\0')
    {
        if (c != '\0')
            printf("Name contains invalid characted '%c'\n", c);
        else
            printf("Name cannot be empty\n");
        printf("Try again: ");
        name = read_line(stdin);
        if (name == NULL)
            return false;
        trim_sides(name);
    }

    //create songbook path
    Path *path = path_copy(home_path, false); //copy, so home_path stays the same
    if (!path_add(path, "songbooks", 'd'))
    {
        path_dtor(path);
        return false;
    }
    if (!path_add(path, name, 'd'))
    {
        path_dtor(path);
        free(name);
        return false;
    }
    songbook.name = name;
    songbook.path = path;

    putchar('\n');
    printf("Choose mode:\n");
    printf("[1] Above-text chords\n");
    printf("[2] In-text chords (top-index)\n");
    printf("Your choice: ");
    int choice;
    while ((choice = readnum(1, 2)) == -1)
        printf("Invalid choice\nTry again: ");
    songbook.type = choice;
    
    putchar('\n');
    printf("Choose songbook format:\n");
    printf("[1] LaTeX (PDF)\n");
    printf("[2] HTML\n");
    printf("[3] .docx\n");
    printf("Your choice: ");
    while ((choice = readnum(1,3)) == -1)
        printf("Invalid choice\nTry again: ");
    songbook.format = choice;

    //create specified songbook
    if (!create_songbook(&songbook))
    {
        rm_rf(songbook.path);
        free(songbook.name);
        path_dtor(songbook.path);
        return false;
    }

    free(songbook.name);
    path_dtor(songbook.path);

    return true;
}

//chooses songbook, returns true if well done, saves to save_to Songbook
//nothing gets allocated if return = false
bool choose_songbook(Path *home_path, Songbook *save_to)
{
    //get path to songbook_list.txt
    Path *songbooks_path = path_copy(home_path, false);
    if (!path_add(songbooks_path, "songbooks/songbook_list.txt", 'f'))
    {
        path_dtor(songbooks_path);
        return false;
    }

    FILE *file = fopen(songbooks_path->path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open songbook_list.txt\n");
        return false;
    }
    path_dtor(songbooks_path);

    //enumerate and save options
    StringArray str_arr = str_arr_ctor(OPTIONS_START_CAPACITY);
    if (str_arr.strings == NULL)
    {
        fclose(file);
        return false;
    }
    int i = 0;
    char *string;
    putchar('\n');
    printf("Choose songbook:\n");
    while ((string = read_line(file)) != NULL)
    {
        if (string[0] == '\0')
        {
            free(string);
            continue;
        }

        //print number and name of songbook
        printf("[%d] ", i+1);
        for (int j = 0; string[j] != '\\' && string[j] != '\0'; j++)
            putchar(string[j]);
        putchar('\n');
        i++;

        //save string to str_arr
        if (!str_arr_add(&str_arr, string))
        {
            str_arr_dtor(&str_arr);
            fclose(file);
            free(string);
            return false;
        }
    }
    fclose(file);

    //check for 'no songbooks yet' option
    if (str_arr.size == 0)
    {
        printf("You have no existing songbooks\n");
        str_arr_dtor(&str_arr);
        return false;
    }

    //get user choice
    printf("Your choice: ");
    int choice;
    while ((choice = readnum(1, (int)str_arr.size)) == -1)
        printf("Invalid choice.\nTry again: ");
    
               //does not create path for songbook
    bool res = decode_songbook_print(str_arr.strings[choice-1], save_to);
    str_arr_dtor(&str_arr);
    if (res == false)
        return false;

    //make path for songbook
    Path *sngbk_path = path_copy(home_path, false);
    res = path_add(sngbk_path, "songbooks", 'd');
    if (res)
        res = path_add(sngbk_path, save_to->name, 'd');
    if (!res)
    {
        path_dtor(sngbk_path);
        return false;
    }

    save_to->path = sngbk_path;
    return true;
}

//returns: -1 if error; 0 if delete nothing, save_to allocated; 1 delete chosen songbook, save_to allocated
int deletion_choice(Path *home_path, Songbook *save_to)
{
    if (home_path == NULL || save_to == NULL)
    {
        fprintf(stderr, "deletion_choice: NULL pointers among parameters\n");
        return -1;
    }

    if (!choose_songbook(home_path, save_to))
        return -1;

    putchar('\n');
    printf("Are you sure you want to remove %s?\n", save_to->name);
    printf("Songbook is located at %s\n", save_to->path->path);
    printf("[0] No\n[1] Yes\n");
    printf("Your choice: ");

    int choice;
    while ((choice = readnum(0, 1)) == -1)
        printf("Invalid choice.\nTry again: ");

    return choice;    
}

//function for adding a song to song_collection
//if return == false, still needs to be freed
//save_to should be initialized
bool get_song(Path *home_path, Song *save_to)
{
    if (home_path == NULL || save_to == NULL)
        return false;
    if (save_to->data.parts == NULL)
        return false;

    //get author and convert to ascii
    //save_to->author_ascii == "idk" is possible
    putchar('\n');
    printf("Author (type 'idk' if unclear): ");
    char *author = read_line(stdin);
    if (author == NULL)
        return false;
    convert_to_ascii(author);
    trim_string(author);
    save_to->author_ascii = author;

    //get song name and convert to ascii
    printf("Song name: ");
    char *sng_name = read_line(stdin);
    if (sng_name == NULL)
        return false;
    convert_to_ascii(sng_name);
    trim_string(sng_name);
    save_to->name_ascii = sng_name;

    //try finding song in song_collection
    Path* song_path;
    if (strcmp(author, "idk") == 0)
    {
        song_path = song_try_find_noauthor(sng_name, home_path);
    }
    else 
    {
        song_path = song_try_find(sng_name, author, home_path);
    }
    
    if (song_path != NULL)
    {
        putchar('\n');
        printf("Song found in song_collection!\n");
        bool decoding_res = decode_song(song_path, save_to);
        if (!decoding_res)
        {
            fprintf(stderr, "Failed to decode song from song_collection.\n");
        }
        path_dtor(song_path);
        return decoding_res;
    }
    else //song not found in song_collection
    {
        bool noauthor = strcmp(author, "idk") == 0;
        bool download_res = download_song_ak(save_to, noauthor);
        if (download_res)
        {
            if (!add_song_songcollection(home_path, save_to))
            {
                fprintf(stderr, "Failed to add song file to song_collection\n");
                download_res = false;
            }
        }
        if (!download_res)
            return false;
    }

    return true;
}

EditSBChoice edit_choice()
{
    putchar('\n');
    printf("\nChoose action to perform upon songbook:\n");
    printf("[0] Undo\n[1] Add song\n[2] Add multiple songs (using queue)\n[3] Remove song\n");
    printf("Your choice: ");
    int choice;
    while ((choice = readnum(0,3)) == -1)
        printf("Invalid choice.\n Try again: ");
    return choice;
}