#pragma once

#include <stdbool.h>

typedef int (*FILESYSTEM_LISTDIR_CALLBACK)(const char *name, const char *path, bool isdir, void *user);

int filesystem_list(const char *dir, FILESYSTEM_LISTDIR_CALLBACK callback, void *user);
bool filesystem_is_directory(const char *dir);
bool filesystem_is_file(const char *filepath);
