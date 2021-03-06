#pragma once

#include <SDL2/SDL_surface.h>

typedef struct AtlasSurface ATLAS_SURFACE;

ATLAS_SURFACE *atlas_alloc(int width, int height, int padding);
void atlas_free(ATLAS_SURFACE *atlas);
SDL_Rect atlas_add(ATLAS_SURFACE *atlas, SDL_Surface *surface);

#include "core/image.h"

IMAGE_DATA atlas_get_image(ATLAS_SURFACE *atlas);
