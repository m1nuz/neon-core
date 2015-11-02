#include <string.h>
#include "core/image.h"
#include "core/asset.h"
#include "core/targa.h"
#include "core/logerr.h"
#include <memtrack.h>

extern SDL_Surface*
upload_surface(const char *name) {
    const char *ext = strrchr(name, '.');

    if (ext == NULL)
        return NULL;

    if (strcmp(ext, ".tga") != 0) {
        LOG_CRITICAL("Can\'t load texture %s\n", name);
        exit(EXIT_FAILURE);
    }

    if (strcmp(ext, ".tga") != 0) {
        LOG_CRITICAL("Can\'t load texture %s\n", name);
        exit(EXIT_FAILURE);
    }

    IMAGE_DATA image;
    memset(&image, 0, sizeof(image));

    load_targa(asset_request(name), &image);

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
    return sf;
}
