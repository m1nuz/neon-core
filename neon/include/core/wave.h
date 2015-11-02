#pragma once

#include <AL/al.h>
#include <SDL2/SDL_rwops.h>

void* load_wave(SDL_RWops *rw, ALenum* _format, ALsizei* _frequency, ALsizei* _size);
