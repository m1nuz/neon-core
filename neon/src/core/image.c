#include <string.h>
#include "core/image.h"
#include "core/asset.h"
#include "core/targa.h"
#include "core/logerr.h"
#include <core/video.h>
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

extern IMAGE_DATA
image_generate_radial_gradient(int width, int height, COLOR c0, COLOR c1, int radius) {
    // the center of the surface
    double cx = (double)width / 2.0;
    double cy = (double)height / 2.0;

    // compute max distance M from center
    double M = (double)radius;//sqrt(cx * cx + cy * cy);

    // the color delta
    double dc = c1 - c0;

    // and constant used in the code....
    double K = dc / M;

    RGB_COLOR *pixels = malloc(width * height * sizeof(RGB_COLOR));

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            // coodinates relative to center, shifted to pixel centers
            double x = i - cx + 0.5;
            double y = j - cy + 0.5;
            double r = sqrt(x * x + y * y);  // the distance

            if (r < M) {
                pixels[i * width + j].r = (COLOR)(r * K + c0);
                pixels[i * width + j].g = pixels[i * width + j].r;
                pixels[i * width + j].b = pixels[i * width + j].r;
            } else {
                pixels[i * width + j].r = c1;
                pixels[i * width + j].g = pixels[i * width + j].r;
                pixels[i * width + j].b = pixels[i * width + j].r;
            }
        }

    return (IMAGE_DATA){.internalformat = video.i_rgb8,
                .width = width,
                .height = height,
                .format = video.f_rgb,
                .type = GL_UNSIGNED_BYTE,
                .pixels = pixels,
                .size = width * height * sizeof(RGB_COLOR),
                .bpp = 24};
}

extern IMAGE_DATA
image_generate_color(int width, int height, RGB_COLOR color) {
    RGB_COLOR *pixels = malloc(width * height * sizeof(RGB_COLOR));

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            pixels[j * width + i] = color;
        }

    return (IMAGE_DATA){.internalformat = video.i_rgb8,
                .width = width,
                .height = height,
                .format = video.f_rgb,
                .type = GL_UNSIGNED_BYTE,
                .pixels = pixels,
                .size = width * height * sizeof(RGB_COLOR),
                .bpp = 24};
}

IMAGE_DATA
image_generate_check(int width, int height, unsigned char mask, RGB_COLOR color) {
    RGB_COLOR *pixels = malloc(width * height * sizeof(RGB_COLOR));

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            unsigned c = (((i & mask) == 0) ^ ((j & mask) == 0));
            pixels[j * width + i] = color;
            pixels[j * width + i].r *= c;
            pixels[j * width + i].g *= c;
            pixels[j * width + i].b *= c;
        }

    return (IMAGE_DATA){.internalformat = video.i_rgb8,
                .width = width,
                .height = height,
                .format = video.f_rgb,
                .type = GL_UNSIGNED_BYTE,
                .pixels = pixels,
                .size = width * height * sizeof(RGB_COLOR),
                .bpp = 24};
}
