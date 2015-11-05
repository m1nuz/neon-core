#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "core/atlas.h"
#include "core/video.h"
#include "memtrack.h"

struct AtlasSurfaceNode {
    struct AtlasSurfaceNode   *left;
    struct AtlasSurfaceNode   *right;

    int                 x;
    int                 y;
    int                 width;
    int                 height;    
    bool                usage;
};

struct AtlasSurface {
    SDL_Surface                 *surface;
    struct AtlasSurfaceNode     *root;
    int                 width;
    int                 height;
    int                 padding;
};

static struct AtlasSurfaceNode *
new_atlas_node(int x, int y, int width, int height) {
    assert(width > 0);
    assert(height > 0);

    struct AtlasSurfaceNode *node = malloc(sizeof(struct AtlasSurfaceNode));

    if (!node)
        return NULL;

    node->x = x;
    node->y = y;
    node->width = width;
    node->height = height;
    node->left = NULL;
    node->right = NULL;
    node->usage = 0;

    return node;
}

static struct AtlasSurfaceNode *
insert_child_atlas_node(struct AtlasSurfaceNode *node, SDL_Surface *surface, int padding) {
    assert(node != NULL);

    /*if(!node)
        return NULL;*/

    if ((node->left != NULL) || (node->right != NULL)) {
        struct AtlasSurfaceNode *lr = insert_child_atlas_node(node->left, surface, padding);

        if (lr == NULL)
            lr = insert_child_atlas_node(node->right, surface, padding);

        return lr;
    }

    int image_w = surface->w + padding * 2;
    int image_h = surface->h + padding * 2;

    if (node->usage || (image_w > node->width) || (image_h > node->height))
        return NULL;

    if((image_w == node->width) && (image_h == node->height)) {
        node->usage = 1;
        return node;
    }

    // extend to the right
    if (node->width - image_w > node->height - image_h) {
        node->left = new_atlas_node(node->x, node->y, image_w, node->height);
        node->right = new_atlas_node(node->x + image_w, node->y, node->width - image_w, node->height);
    } else  {
        // extend to bottom
        node->left = new_atlas_node(node->x, node->y, node->width, image_h);
        node->right = new_atlas_node(node->x, node->y + image_h, node->width, node->height - image_h);
    }

    return insert_child_atlas_node(node->left, surface, padding);
}

static void
delete_atlas_node(struct AtlasSurfaceNode *node) {
    free(node);
}

ATLAS_SURFACE *
atlas_alloc(int width, int height, int padding) {
    assert(width > 0);
    assert(height > 0);
    assert(padding >= 0);

    struct AtlasSurface *atlas = malloc(sizeof(struct AtlasSurface));

    if (!atlas)
        return NULL;

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

    atlas->surface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);

    if (!atlas->surface)
        return NULL;

    atlas->root = new_atlas_node(0, 0, width, height);

    if (!atlas->root)
        return NULL;

    atlas->padding = padding;
    atlas->width = width;
    atlas->height = height;

    return atlas;
}

static void
recursive_delete(struct AtlasSurfaceNode *node) {
    assert(node != NULL);

    if (node->left)
        recursive_delete(node->left);

    if (node->right)
        recursive_delete(node->right);

    delete_atlas_node(node);
}

extern void
atlas_free(ATLAS_SURFACE *atlas) {
    if (!atlas)
        return;

    recursive_delete(atlas->root);

    SDL_FreeSurface(atlas->surface);
    free(atlas);
}

SDL_Rect
atlas_add(ATLAS_SURFACE *atlas, SDL_Surface *surface) {
    assert(atlas != NULL);
    assert(surface != NULL);

    struct AtlasSurfaceNode *rv = insert_child_atlas_node(atlas->root, surface, atlas->padding);

    if (!rv)
        return (SDL_Rect){0};

    SDL_Rect rect = (SDL_Rect){rv->x + atlas->padding, rv->y + atlas->padding, surface->w, surface->h};

    SDL_BlitSurface(surface, 0, atlas->surface, &rect);

    return rect;
}

#include <video/gl.h>

IMAGE_DATA
atlas_get_image(ATLAS_SURFACE *atlas) {
    assert(atlas != NULL);

    size_t sz = atlas->surface->format->BytesPerPixel * atlas->surface->w * atlas->surface->h;

    // TODO: auto free flag for image
    return (IMAGE_DATA){.internalformat = video.i_rgba8,
                .width = atlas->surface->w,
                .height = atlas->surface->h,
                .format = video.f_rgba,
                .type = GL_UNSIGNED_BYTE,
                .pixels = atlas->surface->pixels,
                .size = sz,
                .bpp = atlas->surface->format->BytesPerPixel};
}
