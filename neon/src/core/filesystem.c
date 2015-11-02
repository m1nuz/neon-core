#include <sys/stat.h>
#include <dirent.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <core/filesystem.h>

int
filesystem_list(const char *dir, FILESYSTEM_LISTDIR_CALLBACK callback, void *user) {
    assert(dir != NULL);

    DIR *d = opendir(dir);

    if(!d)
        return -1;

    char buffer[1024 * 2];

    snprintf(buffer, sizeof(buffer), "%s/", dir);
    size_t length = strlen(buffer);

    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        strncpy(buffer + length, entry->d_name, sizeof(buffer) - length);

        if (callback(entry->d_name, buffer, filesystem_is_directory(buffer), user))
            break;
    }

    // close the directory and return
    closedir(d);
    return 0;
}

bool filesystem_is_directory(const char *dir) {
    struct stat sb;
    if (stat(dir, &sb) == -1)
        return false;

    if (S_ISDIR(sb.st_mode))
        return true;
    else
        return false;
}

bool filesystem_is_file(const char *filepath) {
    struct stat buffer;
    return stat(filepath, &buffer) == 0;
}
