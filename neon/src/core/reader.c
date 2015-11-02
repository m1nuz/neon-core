#include <assert.h>
#include <core/reader.h>
#include <core/targa.h>
#include <core/asset.h>
#include <core/logerr.h>

ASSET_READER asset_reader = {
    .datas = (DATA_READER[]) {
        {.ext = ".ttf", .read = NULL},
        {.ext = {0}, .read = NULL}
    },
    .texts = (TEXT_READER[]) {
        {.ext = ".frag", .read = load_shader_text},
        {.ext = ".vert", .read = load_shader_text},
        {.ext = ".frag", .read = load_shader_text},
        {.ext = ".geom", .read = load_shader_text},
        {.ext = ".glsl", .read = load_shader_text},
        {.ext = ".map", .read = load_text},
        {.ext = ".lua", .read = load_text},
        {.ext = ".lang", .read = load_text},
        {.ext = {0}, .read = NULL}
    },
    .textures = (TEXTURE_READER[]) {
        {.ext = ".tga", .read = load_targa},
        {.ext = ".tpic",.read =  load_targa},
        {.ext = {0}, .read = NULL}
    },
    .sounds = (SOUND_READER[]) {
        {.ext = ".wav", .read = NULL},
        {.ext = ".wave", .read = NULL},
        {.ext = {0}, .read = NULL}
    },
    .models = (MODEL_READER[]) {
        {.ext = ".obj", .read = NULL},
        {.ext = {0}, .read = NULL}
    }
};

int
read_text(const char *name, TEXT_DATA *text) {
    assert(name != NULL);
    assert(text != NULL);

    const char *ext = strrchr(name, '.');

    for (size_t i = 0; asset_reader.texts[i].read; i++)
        if (strcasecmp(ext, asset_reader.texts[i].ext) == 0) {
            SDL_RWops *rw = asset_request(name);

            if ((text->text = asset_reader.texts->read(rw, &text->size)) == NULL) {
                LOG_ERROR("Can\'t read text %s\n", name);
                return -1;
            }

            text->capacity = text->size + 1;

            return 0;
        }

    LOG_ERROR("Can\'t find reader for %s", name);
    return -1;
}

int
read_texture(const char *name, IMAGE_DATA *image) {
    assert(name != NULL);
    assert(image != NULL);

    const char *ext = strrchr(name, '.');

    for (size_t i = 0; asset_reader.textures[i].read; i++)
        if (strcasecmp(ext, asset_reader.textures[i].ext) == 0) {
            SDL_RWops *rw = asset_request(name);

            if(asset_reader.textures->read(rw, image) != 0) {
                LOG_ERROR("Can\'t read texture %s\n", name);
                return -1;
            }

            return 0;
        }

    LOG_ERROR("Can\'t find reader for %s", name);
    return -1;
}
