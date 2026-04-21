#include "songbook_manager.h"
#include <stdlib.h>
#include "util.h"
#include <string.h>
#include "remover.h"
#include "input_reader.h"
#include "ak_parser.h"

//creates songbook dir and needed files
bool create_songbook(Songbook *songbook)
{
    if (songbook == NULL)
        return false;
    if (songbook->path == NULL)
        return false;

    //create directory
    if (mkdir(songbook->path->path, 0755) == -1)
        return false;

    //create queue
    Path *queue_path = path_copy(songbook->path, false);
    if (queue_path == NULL)
        return false;
    if (!path_add(queue_path, "queue", 'd'))
    {
        path_dtor(queue_path);
        return false;
    }
    if (mkdir(queue_path->path, 0755) == -1)
    {
        path_dtor(queue_path);
        return false;
    }
    if (path_add(queue_path, "queue.txt", 'f'))
    {
        FILE *queue_tmp = fopen(queue_path->path, "w");
        if (queue_tmp != NULL)
            fclose(queue_tmp);
    }
    path_dtor(queue_path);

    //
    //TODO place for setting up format-specific stuff (multiple functions)
    if (songbook->format == tex)
    {
        if (!songbook_tex_init(songbook))
            return false;
    }
    else if (songbook->format == HTML)
    {
        if (!songbook_html_init(songbook))
            return false;
    }
    //

    Path *songbook_path = path_copy(songbook->path, false);
    //"touch" songlist.txt (truncates)
    if (!path_add(songbook_path, "songlist.txt", 'f'))
    {
        path_dtor(songbook_path);
        return false;
    }
    FILE *songlist = fopen(songbook_path->path, "w");
    if (songlist == NULL)
    {
        path_dtor(songbook_path);
        return false;
    }
    fclose(songlist);
    path_dtor(songbook_path); //not needed anymore

    //add to songbook_list
    //list_path is songbook_path -> dirback -> +songbook_list.txt
    Path *list_path = path_copy(songbook->path, false);
    path_dirback(list_path);
    if (!path_add(list_path, "songbook_list.txt", 'f'))
    {
        path_dtor(list_path);
        return false;
    }
    char *songbook_print = make_songbook_print(songbook);
    bool res = read_insert_write(list_path, songbook_print, -1);
    path_dtor(list_path);
    free(songbook_print);
    if (!res)
        return false;
    
    return true;
}

void songbook_dtor(Songbook *songbook)
{
    if (songbook == NULL)
        return;
    if (songbook->name != NULL)
        free(songbook->name);
    if (songbook->path != NULL)
        path_dtor(songbook->path);
}

//makes a songbook print, which is "name\type\format"
char *make_songbook_print(Songbook *songbook)
{
    if (songbook == NULL)
        return NULL;
    if (songbook->name == NULL)
        return NULL;

    //+2 for every type (in case of a two-digit number); +2 for two slashes; +1 for '\0'
    char *print = malloc(strlen(songbook->name) + 7);
    if (print == NULL)
        return NULL;
    int res = sprintf(print, "%s\\%d\\%d", songbook->name, songbook->type, songbook->format);
    if (res == -1)
    {
        free(print);
        return NULL;
    }

    return print;
}

//decodes songbook_print, but does NOT create songbook->path
bool decode_songbook_print(char *to_decode, Songbook *save_to)
{
    if (to_decode == NULL || save_to == NULL)
        return false;

    save_to->name = malloc(strlen(to_decode));
    if (save_to->name == NULL)
        return false;
    
    int type_i;
    int format_i;
    int res = sscanf(to_decode, "%[^\\]\\%d\\%d", save_to->name, &type_i, &format_i);
    if (res != 3)
    {
        fprintf(stderr, "Could not decode songbook_list.txt\n");
        free(save_to->name);
        return false;
    }
    save_to->type = (Type)type_i;
    save_to->format = (Format)format_i;

    return true;
}

bool remove_songbook(Songbook *songbook)
{
    if (songbook == NULL || songbook->path == NULL)
    {
        fprintf(stderr, "remove_songbook: NULL pointers\n");
        return false;
    }

    if (!rm_rf(songbook->path))
        return false;

    char *songbook_print = make_songbook_print(songbook);
    if (songbook_print == NULL)
        return false;

    //create path for songbook_list.txt
    Path *sblist_path = path_copy(songbook->path, false);
    if (sblist_path == NULL)
    {
        free(songbook_print);
        return false;
    }
    path_dirback(sblist_path); //now at songbooks/
    if (!path_add(sblist_path, "songbook_list.txt", 'f'))
    {
        free(songbook_print);
        path_dtor(sblist_path);
        return false;
    }
    bool res = read_remove_write(sblist_path, songbook_print);
    path_dtor(sblist_path);
    free(songbook_print);
    return res;
}

bool songbook_tex_init(Songbook *songbook)
{
    if (songbook == NULL)
        return false;
    if (songbook->path == NULL || songbook->name == NULL)
        return false;

    //build path to tex_files
    Path *tex_path = path_copy(songbook->path, false);
    if (!path_add(tex_path, "tex_files", 'd'))
    {
        path_dtor(tex_path);
        return false;
    }
    //create tex_files dir
    if (mkdir(tex_path->path, 0755) == -1)
    {
        path_dtor(tex_path);
        return false;
    }
    //copy main.tex
    if (!tex_add_maintex(tex_path, songbook->name))
    {
        path_dtor(tex_path);
        return false;
    }

    //'touch' songs.tex (for certainty)
    if (!path_add(tex_path, "songs.tex", 'f'))
    {
        path_dtor(tex_path);
        return false;
    }
    FILE *songs_tex = fopen(tex_path->path, "w");
    if (songs_tex == NULL)
    {
        path_dtor(tex_path);
        return false;
    }
    fclose(songs_tex);
    path_dirback(tex_path);

    //make songs/ dir
    if (!path_add(tex_path, "songs", 'd'))
    {
        path_dtor(tex_path);
        return false;
    }
    if (mkdir(tex_path->path, 0755) == -1)
    {
        path_dtor(tex_path);
        return false;
    }
    path_dirback(tex_path);

    //build path to [songbook name]/tex_files/songbook.cls
    if (!path_add(tex_path, "songbook.cls", 'f'))
    {
        path_dtor(tex_path);
        return false;
    }

    //build path to .templates/tex/type/
    Path *template_path = path_copy(songbook->path, false);
    path_dirback(template_path); //now at home_p/songbooks
    if (!path_add(template_path, ".templates", 'd') ||
        !path_add(template_path, "tex", 'd'))
    {
        path_dtor(template_path);
        path_dtor(tex_path);
        return false;
    }
    //now at songbooks/.templates/tex
    if (songbook->type == type1)
    {
        if (!path_add(template_path, "type1", 'd'))
        {
            path_dtor(template_path);
            path_dtor(tex_path);
            return false;
        }
    }
    else 
    {
        if (!path_add(template_path, "type2", 'd'))
        {
            path_dtor(template_path);
            path_dtor(tex_path);
            return false;
        }
    }
    //now at correct type
    if (!path_add(template_path, "songbook.cls", 'f'))
    {
        path_dtor(template_path);
        path_dtor(tex_path);
        return false;
    }

    //copy songbook.cls
    bool result = copy_file(template_path, tex_path);
    path_dtor(template_path);
    path_dtor(tex_path);
    
    return result;
}

//adds main.tex to songbook
//tex_path must be at songbooks/[songbook_name]/tex_files
bool tex_add_maintex(Path *tex_path, char *songbook_name)
{
    if (songbook_name == NULL || tex_path == NULL || tex_path->path == NULL)
        return false;
    
    //create path to .templates/tex/main.tex and open file
    Path *template_path = path_copy(tex_path, false);
    if (template_path == NULL)
        return false;
    path_dirback(template_path); //now at songbooks/songbook_name
    path_dirback(template_path); //now at songbooks
    if (!path_add(template_path, ".templates", 'd') ||
        !path_add(template_path, "tex", 'd') ||
        !path_add(template_path, "main.tex", 'f'))
    {
        path_dtor(template_path);
        return false;
    }
    FILE *template_main = fopen(template_path->path, "r");
    path_dtor(template_path);
    if (template_main == NULL)
        return false;

    //create path to songbook_name/tex/main.tex and open file
    Path *newmain_path = path_copy(tex_path, false);
    if (newmain_path == NULL)
    {
        fclose(template_main);
        return false;
    }
    if (!path_add(newmain_path, "main.tex", 'f'))
    {
        fclose(template_main);
        path_dtor(newmain_path);
        return false;
    }
    FILE *main_cpy = fopen(newmain_path->path, "w");
    path_dtor(newmain_path);
    if (main_cpy == NULL)
    {
        fclose(template_main);
        return false;
    }

    //now both files are open
    //read and write
    char *line;
    while ((line = read_line(template_main)) != NULL)
    {
        if (strcmp(line, "\\title{}") == 0)
            fprintf(main_cpy, "\\title{%s}\n", songbook_name);
        else
            fprintf(main_cpy, "%s\n", line);
        free(line);
    }

    fclose(template_main);
    fclose(main_cpy);

    return true;
}

//copies copy_from to copy_to
//both paths must be to valid filenames
bool copy_file(Path *copy_from, Path *copy_to)
{
    if (copy_from == NULL || copy_to == NULL)
        return false;
    if (copy_from->path == NULL || copy_to->path == NULL)
        return false;

    FILE *file_r = fopen(copy_from->path, "rb");
    if (file_r == NULL)
        return false;
    FILE *file_w = fopen(copy_to->path, "wb");
    if (file_w == NULL)
    {
        fclose(file_r);
        return false;
    }

    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), file_r)) > 0)
        fwrite(buffer, 1, n, file_w);

    fclose(file_r);
    fclose(file_w);

    return true;
}

bool songbook_html_init(Songbook *songbook)
{
    if (songbook == NULL)
        return false;
    if (songbook->path == NULL || songbook->name == NULL)
        return false;

    //build html folder path
    Path *html_path = path_copy(songbook->path, false);
    if (html_path == NULL)
        return false;
    if (!path_add(html_path, "html", 'd'))
    {
        path_dtor(html_path);
        return false;
    }

    //mkdir [songbook]/html/
    if (mkdir(html_path->path, 0755) == -1)
    {
        path_dtor(html_path);
        return false;
    }

    //build path toward html/temp and mkdir
    if (!path_add(html_path, "temp", 'd'))
    {
        path_dtor(html_path);
        return false;
    }
    if (mkdir(html_path->path, 0755) == -1)
    {
        path_dtor(html_path);
        return false;
    }
    path_dirback(html_path);

    //build path toward html/songs and mkdir
    if (!path_add(html_path, "songs", 'd'))
    {
        path_dtor(html_path);
        return false;
    }
    if (mkdir(html_path->path, 0755) == -1)
    {
        path_dtor(html_path);
        return false;
    }
    path_dirback(html_path);

    bool res =  html_add_files(html_path, songbook->path, songbook->name, songbook->type);
    
    path_dtor(html_path);

    return res;

}

bool html_add_files(Path *html_path, Path *songbook_path, char *songbook_name, Type songbook_type)
{
    //make template path
    Path *template_path = path_copy(songbook_path, false);
    if (template_path == NULL)
        return false;
    path_dirback(template_path); //now at songbooks
    if (!path_add(template_path, ".templates", 'd') ||
        !path_add(template_path, "html", 'd'))
    {
        path_dtor(template_path);
        return false;
    }

    //now at songbooks/.templates
    //add [songbook].html
    Path *path = path_copy(html_path, false);
    if (path == NULL)
    {
        path_dtor(template_path);
        return false;
    }
    if (!path_add(path, songbook_name, 'f') ||
        !path_add(path, ".html", 's'))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }

    if (!path_add(template_path, "main.html", 'f'))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }

    //add titlepage
    FILE *template = fopen(template_path->path, "r");
    if (template == NULL)
    {
        fprintf(stderr, "Could not open %s\n", template_path->path);
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    FILE *html_main = fopen(path->path, "w");
    if (html_main == NULL)
    {
        fprintf(stderr, "Could not open %s\n", path->path);
        path_dtor(path);
        path_dtor(template_path);
        fclose(template);
        return false;
    }
    bool titlepage_res = html_add_titlepage(template, html_main, songbook_name);
    fclose(template);
    fclose(html_main);
    if (!titlepage_res)
    {
        fprintf(stderr, "Didn't find title comment in main.html in .templates\n");
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    path_dirback(path); //now at [songbook]/html
    path_dirback(template_path); //now at .templates/html

    //add style
    if (!path_add(template_path, "style.css", 'f') ||
        !path_add(path, "style.css", 'f'))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }

    if (!copy_file(template_path, path))
    {
        fprintf(stderr, "Could not copy %s\n", template_path->path);
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    path_dirback(template_path);
    
    //determine what to append based on type
    if (songbook_type == type1)
    {
        if (!path_add(template_path, "type1.css", 'f'))
        {
            path_dtor(path);
            path_dtor(template_path);
            return false;
        }
    }
    else 
    {
        if (!path_add(template_path, "type2.css", 'f'))
        {
            path_dtor(path);
            path_dtor(template_path);
            return false;
        }
    }

    FILE *songbook_style = fopen(path->path, "a");
    if (songbook_style == NULL)
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    FILE *type_style = fopen(template_path->path, "r");
    if (type_style == NULL)
    {
        path_dtor(path);
        path_dtor(template_path);
        fclose(songbook_style);
        return false;
    }

    char *line;
    while ((line = read_line(type_style)) != NULL)
    {
        fprintf(songbook_style, "%s\n", line);
        free(line);
    }
    fclose(songbook_style);
    fclose(type_style);
    path_dirback(path);
    path_dirback(template_path);

    //copy TOC
    if (!path_add(template_path, "toc.html", 'f') ||
        !path_add(path, "toc.html", 'f'))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    if (!copy_file(template_path, path))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    path_dirback(template_path);
    path_dirback(path);

    //add temp utilities
    if (!path_add(path, "temp", 'd') ||
        !path_add(path, "fitDocument.js", 'f') ||
        !path_add(template_path, "fitDocument.js", 'f'))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }

    if (!copy_file(template_path, path))
    {
        path_dtor(path);
        path_dtor(template_path);
        return false;
    }
    path_dtor(template_path);
    path_dirback(path);

    //'touch' temp.html
    if (path_add(path, "temp.html", 'f'))
    {
        FILE *temp_file = fopen(path->path, "w");
        if (temp_file != NULL)
            fclose(temp_file);
    }

    path_dtor(path);
    return true;
}

bool html_add_titlepage(FILE *template, FILE *copy_to, char *songbook_name)
{
    char *line;
    bool didit = false;
    bool title = false;
    while ((line = read_line(template)) != NULL)
    {
        if (!title && strstr(line, "<title>") != NULL)
        {
            fprintf(copy_to, "    <title>%s</title>\n", songbook_name);
            title = true;
        }
        else if (!didit && strcmp(line, "<!--title-->") == 0)
        {
            fprintf(copy_to, "            <h1 id=\"songbook-title\">%s</h1>\n", songbook_name);
            didit = true;
        }
        else 
            fprintf(copy_to, "%s\n", line);
        free(line);
    }
    return didit;
}