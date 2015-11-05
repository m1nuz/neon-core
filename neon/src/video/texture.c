#include <assert.h>
#include <math.h>
#include <xxhash.h>
#include <memtrack.h>

#include "core/common.h"
#include "core/logerr.h"
#include "video/texture.h"
#include "core/video.h"

#ifndef GL_ES_VERSION_2_0
#include <GL/ext_texture_filter_anisotropic.h>
#endif // NO GL_ES_VERSION_2_0

extern TEXTURE
new_texture2D(const IMAGE_DATA *image) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, image->internalformat, image->width , image->height, 0, image->format, image->type,
                 image->pixels);

    unsigned int flags = 0;

    if(image->mipmaps) {
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
        glGenerateMipmap(GL_TEXTURE_2D);
        flags |= TEXTURE_MIPMAPS_BIT;
    }
    uint32_t hash = 0;

    if (image->pixels) {
        flags |= TEXTURE_DATA_BIT;
        hash = XXH32(image->pixels, image->size, GL_TEXTURE_2D);
    }

    /*if (image->swizzle == GL_TEXTURE_SWIZZLE_RGBA)
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, image->swizzle_colors);*/

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG("New texture %d\n", tex);

    return (TEXTURE){.id = tex,.target = GL_TEXTURE_2D, .flags = flags, .width = image->width, .height = image->height,
                .hash = hash};
}

extern TEXTURE
new_texture_array(const IMAGE_DATA *images, int count) {
#ifndef GL_ES_VERSION_2_0
    GLsizei depth = count > 1 ? count : images->depth;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, images->internalformat, images->width, images->height, depth, 0,
                 images->format, images->type, NULL);

    for (int i = 0; i < depth; i++)
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, images[i].width, images[i].height, 1, images[i].format,
                        images[i].type, images[i].pixels);

    if(images->mipmaps) {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    unsigned int flags = 0;
    uint32_t hash = 0;

    // http://stackoverflow.com/questions/12372058/how-to-use-gl-texture-2d-array-in-opengl-3-2

    return (TEXTURE){.id = tex,.target = GL_TEXTURE_2D_ARRAY, .flags = flags, .width = images->width,
                .depth = images->depth, .height = images->height, .hash = hash};
#else
    UNUSED(images);
    UNUSED(count);

    LOG_WARNING("%s\n", "GL_TEXTURE_2D_ARRAY not supported");

    return (TEXTURE){.id = 0};
#endif // GL_ES_VERSION_2_0
}

extern TEXTURE
new_texture_cube(const IMAGE_DATA *images) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, images[i].internalformat, images[i].width, images[i].height,
                     0, images[i].format, images[i].type, images[i].pixels);

    if(images->mipmaps)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    unsigned int flags = 0;
    uint32_t hash = 0;

    return (TEXTURE){.id = tex,.target = GL_TEXTURE_CUBE_MAP, .flags = flags, .width = images->width,
                .depth = images->depth, .height = images->height, .hash = hash};
}

extern void
free_texture(TEXTURE *texture) {
    assert(texture != NULL);

    /*if(glIsTexture(texture->id))*/ {
        glDeleteTextures(1, &texture->id);

        LOG("Delete texture %d", texture->id);
    }
    /*else
        LOG_WARNING("Can\'t free texture %d, not texture or already free!\n", texture->id);*/

    texture->id = 0;
    texture->usage = 0;
}

#include "core/asset.h"
#include "core/reader.h"

extern TEXTURE
upload_texture(const char *name) {
    IMAGE_DATA image;
    memset(&image, 0, sizeof(image));

    /*const char *ext = strrchr(name, '.');

    if (strcmp(ext, ".tga") != 0) {
        LOG_CRITICAL("Can\'t load texture %s\n", name);
        exit(EXIT_FAILURE);
    }

    load_targa(asset_request(name), &image);*/

    if (read_texture(name, &image) != 0)
        exit(EXIT_FAILURE);

    image.mipmaps = 1;

    LOG("LOAD %s\n", name);

    TEXTURE tex = new_texture2D(&image);

    free(image.pixels);

    return tex;
}

extern TEXTURE
upload_textures(const char **names) {
    int count = 0;

    LOG("%s\n", "LOAD ARRAY");
    while(names[count]) {
        LOG("\t%s\n", names[count]);
        count++;
    }

    IMAGE_DATA images[count];
    memset(images, 0, sizeof(images));

    for (int i = 0; i < count; i++) {
        /*const char *ext = strrchr(names[i], '.');

        if (strcmp(ext, ".tga") != 0) {
            LOG_CRITICAL("Can\'t load texture %s\n", names[i]);
            exit(EXIT_FAILURE);
        }

        load_targa(asset_request(names[i]), &images[i]);*/

        if (read_texture(names[i], &images[i]) != 0)
            exit(EXIT_FAILURE);

        images[i].mipmaps = 1;
    }

    TEXTURE tex = new_texture_array(images, count);

    for (int i = 0; i < count; i++)
        free(images[i].pixels);

    return tex;
}

extern TEXTURE
upload_cubemap_textures(const char *name) {
    char ns[strlen(name)];
    strcpy(ns, name);

    int count = 0;
    IMAGE_DATA images[6];
    memset(&images, 0, sizeof(images));

    char *ptr;
    char *p = strtok_r(ns, " ", &ptr);

    LOG("%s\n", "LOAD CUBEMAP");

    while (p) {
        /*printf("\t%s\n", p);

        const char *ext = strrchr(p, '.');

        if (strcmp(ext, ".tga") != 0) {
            LOG_CRITICAL("Can\'t load texture %s\n", p);
            exit(EXIT_FAILURE);
        }

        load_targa(asset_request(p), &images[count]);*/
        if (read_texture(p, &images[count]) != 0)
            exit(EXIT_FAILURE);

        images[count].mipmaps = 1;

        count++;
        p = strtok_r(NULL, " ", &ptr);
    }

    TEXTURE tex = new_texture_cube(images);

    for (int i = 0; i < count; i++)
        free(images[i].pixels);

    return tex;
}

extern void
bind_texture(uint32_t unit, TEXTURE *tex) {
    glActiveTexture(GL_TEXTURE0 + (unit));
    glBindTexture(tex->target, tex->id);
}

extern void
unbind_texture(uint32_t unit, uint32_t target) {
    glActiveTexture(GL_TEXTURE0 + (unit));
    glBindTexture(target, 0);
}

extern void
generate_mipmap_texture(TEXTURE *tex) {
    if (tex->flags & TEXTURE_MIPMAPS_BIT)
        glGenerateMipmap(GL_TEXTURE_2D);
}
