#pragma once

#include <SDL2/SDL_rwops.h>

int load_lang(SDL_RWops *rw);
const char * lang_get(const char *id);
const char *lang_get_name(size_t lang);
size_t lang_select(const char *name);
size_t lang_count(void);

void lang_init(void);
void lang_cleanup(void);
