#pragma once

//#define WAVEFRONT_DEBUG
#define WAVEFRONT_READ_BUFFER_SIZE 256

#include <video/gl.h>
#include <SDL2/SDL_rwops.h>

#include "video/vertex.h"

int load_wavefront(SDL_RWops *rw, void **_vertices, int *_vertices_num, void **_indices, int *_indices_num);
