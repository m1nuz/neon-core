#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL_rwops.h>

#define RESOURCE_EXTS_LIST ".tga",".obj",".frag",".vert",".geom", ".glsl", ".ttf", ".wav", ".wave", ".map",\
    ".lua", ".lang",

#define OPTIMIZE_FILES_SEARCH

#define MAX_PACKAGES            10
#define MAX_RESOURCE_NAME       100
#define MAX_RESOURCE_PATH       260
#define MAX_PACKAGE_FILEPATH    260
#define FILES_RESERV_NUM        1000

typedef struct ResourceInfo {
    bool            packed;
    char            name[MAX_RESOURCE_NAME];
    char            path[MAX_RESOURCE_PATH];
    uint32_t        name_hash;
    uint32_t        ext_hash;
} RESOURCE_INFO;

int asset_open(const char *path);
void asset_close(void);
void asset_process(void);

size_t asset_query_filelist(const char *ext, RESOURCE_INFO *info);

SDL_RWops* asset_request_hash(uint32_t hash);
SDL_RWops* asset_request(const char *name);
