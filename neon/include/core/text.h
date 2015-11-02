#pragma once

#include <stddef.h>
#include <SDL2/SDL_rwops.h>

typedef struct TextData {
    char    *text;
    size_t  size;
    size_t  capacity;
} TEXT_DATA;

char * load_text(SDL_RWops *rw, size_t *size);
char * load_shader_text(SDL_RWops *rw, size_t *size);
