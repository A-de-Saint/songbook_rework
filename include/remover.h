#ifndef REMOVER_H
#define REMOVER_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "pathwork.h"
#include <limits.h>

#ifndef NAME_MAX
    #define NAME_MAX 255
#endif

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>

    #define fs_unlink(path) _unlink(path)
    #define fs_rmdir(path) _rmdir(path)
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>

    #define fs_unlink(path) unlink(path)
    #define fs_rmdir(path) rmdir(path)
#endif

typedef struct {
    #ifdef _WIN32
        HANDLE handle;
        WIN32_FIND_DATAA data;
        int first;
    #else
        DIR* dir;
    #endif
} fs_dir;

bool fs_opendir(fs_dir *dir, const char *path);

bool fs_readdir(fs_dir *dir, char *buffer, unsigned int b_size);

void fs_closedir(fs_dir *dir);

int fs_isdir(const char *path);

bool rm_rf(Path *path);

#endif