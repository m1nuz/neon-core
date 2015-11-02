#include "core/targa.h"
#include "core/logerr.h"
#include <core/video.h>
#include <memtrack.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>

extern int
load_targa(SDL_RWops *rw, IMAGE_DATA *image) {
    if (!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return -1;
    }

    Sint64 lenght = SDL_RWsize(rw);

    TARGA_HEADER header;

    SDL_RWread(rw, &header, sizeof(header), 1);

    const uint8_t bytesperpixel = header.bpp / 8;
    uint8_t *data = (uint8_t*)malloc(header.width * header.height * (bytesperpixel + 1));
    uint8_t *pdata = data;

    if (header.data_type == TARGA_DATA_RLE_TRUE_COLOR) {
        uint8_t block = 0;
        size_t readen = 0;

        for (int i = 0; i < header.width * header.height; i++) {
            readen = SDL_RWread(rw, &block, 1, 1);

            if (readen) {
                uint8_t count = (block & 0x7f) + 1;

                if (block & 0x80) {
                    uint8_t bytes[4] = {0};
                    SDL_RWread(rw, bytes, bytesperpixel, 1);

                    for(int j = 0; j < count; j++) {
                        memcpy(pdata, bytes, bytesperpixel);
                        pdata += bytesperpixel;
                    }
                } else {
                    SDL_RWread(rw, pdata, bytesperpixel * count, 1);
                    pdata += bytesperpixel * count;
                }
            }
        }

        SDL_RWclose(rw);
    }
    else if ((header.data_type == TARGA_DATA_TRUE_COLOR) || (header.data_type == TARGA_DATA_BLACK_AND_WHITE)) {
        if (!SDL_RWread(rw, data, lenght - sizeof(header), 1))
            SDL_RWclose(rw);
    }

#ifdef GL_ES_VERSION_2_0
    if (header.bpp != 8)
        for (int i = 0; i < header.width * header.height * bytesperpixel; i+= bytesperpixel) {
            uint8_t t = data[i];
            data[i] = data[i + 2];
            data[i + 2] = t;
        }

    switch(header.bpp) {
    case 8:
        image->internalformat = GL_LUMINANCE;
        image->format = GL_LUMINANCE;
        break;
    case 24:
        image->internalformat = GL_RGB;
        image->format = GL_RGB;
        break;
    case 32:
        image->internalformat = GL_RGBA;
        image->format = GL_RGBA;
        break;
    }
#else

#ifdef VIDEO_SRGB_CAPABLE
    switch(header.bpp) {
    case 8:
        image->internalformat = GL_R8;
        image->format = GL_RED;
        break;
    case 24:
        image->internalformat = video.i_rgb8;
        image->format = GL_BGR;
        break;
    case 32:
        image->internalformat = video.i_rgba8;
        image->format = GL_BGRA;
        break;
    }
#else
    switch(header.bpp)
    {
    case 8:
        image->internalformat = GL_R8;
        image->format = GL_RED;
        break;
    case 24:
        image->internalformat = GL_RGB8;
        image->format = GL_BGR;
        break;
    case 32:
        image->internalformat = GL_RGBA8;
        image->format = GL_BGRA;
        break;
    }
#endif // VIDEO_SRGB_CAPABLE
#endif // NO GL_ES_VERSION_2_0

    image->bpp = header.bpp;
    image->width = header.width;
    image->height = header.height;
    image->pixels = data;
    image->type = GL_UNSIGNED_BYTE;
    image->mipmaps = 0;
    image->size = header.width * header.height * (bytesperpixel + 1);

    return 0;
}

/*extern SDL_Surface *
load_targa_surface(SDL_RWops *rw) {
    IMAGE_DATA image;
    load_targa(rw, &image);

    uint32_t rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface *sf = SDL_CreateRGBSurfaceFrom(image.pixels, image.width, image.height, image.bpp,
                                               (image.bpp / 8) * image.width, rmask, gmask, bmask, amask);

    free(image.pixels);

    return sf;
}*/
