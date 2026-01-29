#include "remover.h"
#include <stdlib.h>

bool fs_opendir(fs_dir *dir, const char *path)
{
    #ifdef _WIN32
        //TODO Windows part
    #else
        dir->dir = opendir(path);
        if (dir == NULL)
            return false;
        return true;
    #endif
}

bool fs_readdir(fs_dir *dir, char *buffer, unsigned int b_size)
{
    #ifdef _WIN32
        //TODO Windows part
    #else
        struct dirent *e = readdir(dir->dir);
        if (e == NULL)
            return false;
        strncpy(buffer, e->d_name, b_size-1);
        buffer[b_size-1] = '\0';
        return true;
    #endif
}

void fs_closedir(fs_dir *dir)
{
    #ifdef _WIN32
        //TODO Windows part
    #else
        if (dir != NULL && dir->dir != NULL)
        {
            closedir(dir->dir);
            dir = NULL;
        }
    #endif
}

//return: 1 - dir; 0 - file; -1 - neither
int fs_isdir(const char *path)
{
    if (path == NULL)
        return -1;
    #ifdef _WIN32
        //TODO Windows part
    #else 
        struct stat st;

        if (lstat(path, &st) != 0)
            return -1;

        return S_ISDIR(st.st_mode);
    #endif
}

//removes path recursively
//it is recommended pass a copy of path, just to be sure
bool rm_rf(Path *path)
{
    int res = fs_isdir(path->path);
    if (res == -1)
        return false;
    if (res == 1)
    {
        fs_dir dir;
        if (!fs_opendir(&dir, path->path))
            return false;
        char name[NAME_MAX + 1];
        while (fs_readdir(&dir, name, NAME_MAX + 1))
        {
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
                continue;

            //in case of a file, 'file.txt/' is bad, but a dir without 'dirname/' is good to go
            path_add(path, name, 'f');
            if (!rm_rf(path))
            {
                fs_closedir(&dir);
                path_dirback(path);
                fprintf(stderr, "An error occured during file removal.\n");
                return false;
            }
            path_dirback(path);
        }

        fs_closedir(&dir);
        return fs_rmdir(path->path) == 0;
    }
    return fs_unlink(path->path) == 0;
}