#include "menu.h"
#include "input_reader.h"
#include "songbook_manager.h"

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
    printf("Choose action you wish to do:\n");

    printf("[0] Exit program\n");
    printf("[1] Add new songbook\n");
    printf("[2] Edit songs in an existing songbook\n");
    printf("[3] Remove songbook\n");
    printf("Your choice: ");

    int choice;
    while ((choice = readnum(0, 3)) == -1)
        printf("Invalid input.\nTry again: ");

    putchar('\n');
    return choice;
}

bool new_songbook(Path *home_path)
{
    printf("NEW SONGBOOK\n");

    Songbook songbook;
    Path *path = path_copy(home_path, false); //copy, so home_path stays the same
    if (!path_add(path, "songbooks", 'd'))
    {
        path_dtor(path);
        return false;
    }

    //create path
    printf("Choose name: ");
    char *name = read_line(stdin);
    if (!path_add(path, name, 'd'))
    {
        path_dtor(path);
        free(name);
        return false;
    }
    songbook.name = name;
    songbook.path = path;

    printf("Choose mode:\n");
    printf("[1] Above-text chords\n");
    printf("[2] In-text chords (top-index)\n");
    printf("Your choice: ");
    int choice;
    while ((choice = readnum(1, 2)) == -1)
        printf("Invalid choice\nTry again: ");
    songbook.type = choice;
    
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
        free(songbook.name);
        path_dtor(songbook.path);
        return false;
    }

    free(songbook.name);
    path_dtor(songbook.path);

    return true;
}