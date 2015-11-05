#pragma once

#include <video/gl.h>
#include "core/image.h"
#include <video/sampler.h>

enum TextureFlags {
    TEXTURE_DATA_BIT    = 0x00000001,
    TEXTURE_MIPMAPS_BIT = 0x00000002
};

typedef struct TextureInputs {
    int             internalformat;
    unsigned int    format;
    int             type;
    int             mipmaps;
} TEXTURE_INPUTS;

typedef struct Texture {
    unsigned int    id;
    unsigned int    target;
    unsigned int    flags;

    int             width;
    int             height;
    int             depth;

    uint32_t        hash;
    uint32_t        usage;
} TEXTURE;

TEXTURE new_texture2D(const IMAGE_DATA *image);
TEXTURE new_texture_array(const IMAGE_DATA *images, int count);
TEXTURE new_texture_cube(const IMAGE_DATA *images);
void free_texture(TEXTURE *texture);

TEXTURE upload_texture(const char *name);
TEXTURE upload_textures(const char **names);
TEXTURE upload_cubemap_textures(const char *name);

void bind_texture(uint32_t unit, TEXTURE *tex);
void unbind_texture(uint32_t unit, uint32_t target);
void generate_mipmap_texture(TEXTURE *tex);
