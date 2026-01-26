#include <stdio.h>
#include <stdlib.h>
#include "pathwork.h"
#include "menu.h"

int main(void)
{
    Path *home_path = path_ctor(".");
    ActionChoice choice = action_choice();
    //TODO all possible choices
    if (choice == new_sb)
    {
        if (!new_songbook(home_path))
            printf("Failed to create specified songbook.\n");
        else 
            printf("New songbook created successfully\n");
    }

    path_dtor(home_path);
    return 0;
}