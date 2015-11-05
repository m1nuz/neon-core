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

typedef struct RGB_Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB_COLOR;

typedef uint8_t COLOR;

SDL_Surface* upload_surface(const char *name);

IMAGE_DATA image_generate_radial_gradient(int width, int height, COLOR c0, COLOR c1, int radius);
IMAGE_DATA image_generate_color(int width, int height, RGB_COLOR color);
IMAGE_DATA image_generate_check(int width, int height, unsigned char mask, RGB_COLOR color);
