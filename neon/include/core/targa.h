#pragma once

#include <stdint.h>
#include <video/gl.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>

#include "core/image.h"

enum TARGA_DATA_TYPE
{
    TARGA_DATA_NO = 0,
    TARGA_DATA_COLOR_MAPPED = 1,        // indexed
    TARGA_DATA_TRUE_COLOR = 2,          // RGB
    TARGA_DATA_BLACK_AND_WHITE = 3,     // grayscale
    TAGRA_DATA_RLE_COLOR_MAPPED = 9,
    TARGA_DATA_RLE_TRUE_COLOR = 10,
    TARGA_DATA_RLE_BLACK_AND_WITE = 11
};

#pragma pack(push, tga_header_align)
#pragma pack(1)
typedef struct TargaHeader
{
    uint8_t     length;
    uint8_t     color_map;
    uint8_t     data_type;
    uint16_t    colormap_index;
    uint16_t    colormap_length;
    uint8_t     colormap_entry_size;
    uint16_t    x;
    uint16_t    y;
    uint16_t    width;
    uint16_t    height;
    uint8_t     bpp;
    uint8_t     decription;
} TARGA_HEADER;
#pragma pack(pop, tga_header_align)

#ifdef __cplusplus
extern "C" {
#endif

int load_targa(SDL_RWops *rw, IMAGE_DATA *image);

#ifdef __cplusplus
}
#endif
