#include "recompiler.h"
#include <string.h>

//recompiles a songbook - loads entire songlist.txt as a queue and performs quick read
int recompile_songbook(Path *home_path, Songbook *songbook, StringArray *unsuccessful)
{
    if (home_path == NULL || songbook == NULL || songbook->path == NULL || unsuccessful == NULL)
        return  -1;
    
    int return_val = -1;
    Path *path = path_copy(songbook->path, false);
    if (path == NULL)
        goto function_end;
    if (!path_add(path, "songlist.txt", 'f'))
        goto path_free_end;
    
    FILE *songlist = fopen(path->path, "r");
    if (songlist == NULL)
        goto path_free_end;

    
    char *format = malloc(32);
    if (format == NULL)
        goto path_free_end;
    if (songbook->format == HTML)
        strcpy(format, "%[^_]_%[^\\]\\%*[^;];%s"); //name_author\dontcare\dontcare;chord
    else 
        strcpy(format, "%[^_]_%[^;];%s"); //name_author;chord
    
    ComesFirst first = NAME;

    return_val = get_multiple_songs_parsed(home_path, songbook, songlist, format, first, unsuccessful);
    
  path_free_end:
    path_dtor(path);
  function_end:
    return return_val;
}