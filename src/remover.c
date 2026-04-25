#include "remover.h"
#include <stdlib.h>

bool fs_opendir(fs_dir *dir, const char *path)
{
    #ifdef _WIN32
        //TODO Windows part
        size_t path_len = strlen(path);
        char *pattern_path = malloc(path_len + 3); //+3 for '\\', '*' and '\0'
        if (pattern_path == NULL)
            return false;
        pattern_path[path_len] = '\\';
        pattern_path[path_len + 1] = '*';
        pattern_path[path_len + 2] = '\0';

        dir->handle = FindFirstFileA(pattern_path, &dir->data);

        free(pattern_path);

        if (dir->handle == INVALID_HANDLE_VALUE)
            return false;

        dir->first = 1;
        return true;
        
    #else
        dir->dir = opendir(path);
        if (dir->dir == NULL)
            return false;
        return true;
    #endif
}

bool fs_readdir(fs_dir *dir, char *buffer, unsigned int b_size)
{
    #ifdef _WIN32
        //TODO Windows part
        if (dir->first)
            dir->first = 0;
        else
        {
            if (!FindNextFileA(dir->handle, &dir->data))
                return false;
        }

        strncpy(buffer, dir->data.cFileName, b_size-1);
        buffer[b_size-1] = '\0';
        return true;
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
        if (dir != NULL && dir->handle != INVALID_HANDLE_VALUE)
        {
            FindClose(dir->handle);
            dir->handle = INVALID_HANDLE_VALUE;
        }
    #else
        if (dir != NULL && dir->dir != NULL)
        {
            closedir(dir->dir);
            dir->dir = NULL;
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
        DWORD attributes = GetFileAttributesA(path);

        if (attributes == INVALID_FILE_ATTRIBUTES)
        {
            fprintf(stderr, "fs_isdir: Not a file or a directory\n");
            return -1;
        }

        return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    #else 
        struct stat st;

        if (lstat(path, &st) != 0)
        {
            fprintf(stderr, "fs_isdir: Not a file or a directory\n");
            return -1;
        }

        return S_ISDIR(st.st_mode);
    #endif
}

//removes path recursively
//it is recommended pass a copy of path, just to be sure
bool rm_rf(Path *path)
{
    int res = fs_isdir(path->path);
    if (res == -1)
    {
        fprintf(stderr, "rm_rf: nonexistent path\n");
        return false;
    }
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