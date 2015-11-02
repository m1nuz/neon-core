#pragma once

#include <stddef.h>
#include <SDL2/SDL_surface.h>

/*typedef enum PixelFormat {
    PF_R8_UB,
    PF_R16_F,
    PF_R32_F,

    PF_RG8_UB,
    PF_RGB8_UB,
    PF_RGBA8_UB,
    PF_BGR8_UB,
    PF_BGRA8_UB,
} PIXEL_FORMAL;*/

typedef struct ImageData {    
    int             internalformat;
    int             width;
    int             height;
    int             depth;
    unsigned int    format;
    int             type;
    void            *pixels;
    size_t          size;
    int             mipmaps;
    int             bpp;
} IMAGE_DATA;

SDL_Surface* upload_surface(const char *name);
