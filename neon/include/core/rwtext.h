#pragma once

#include <SDL2/SDL_rwops.h>

int rweof(SDL_RWops *ctx);
int rwgetc(SDL_RWops *rw);
char *rwgets(char *buf, int count, SDL_RWops *rw);
