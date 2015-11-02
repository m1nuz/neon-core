#pragma once

#include <core/image.h>
#include <core/text.h>
#include <AL/al.h>

typedef SDL_RWops* (*DATA_READ_FN)(SDL_RWops *rw);
typedef char* (*TEXT_READ_FN)(SDL_RWops *rw, size_t *size);
typedef int (*TEXTURE_READ_FN)(SDL_RWops *rw, IMAGE_DATA *image);
typedef void* (*SOUND_READ_FN)(SDL_RWops *rw, ALenum* _format, ALsizei* _frequency, ALsizei* _size);
typedef int (*MODEL_READ_FN)(SDL_RWops *rw, void **_vertices, int *_vertices_num, void **_indices, int *_indices_num);

typedef struct DataReader {
    char                ext[20];
    DATA_READ_FN        read;
} DATA_READER;

typedef struct TextReader {
    char                ext[20];
    TEXT_READ_FN        read;
} TEXT_READER;

typedef struct TextureReader {
    char                ext[20];
    TEXTURE_READ_FN     read;
} TEXTURE_READER;

typedef struct SoundReader {
    char                ext[20];
    SOUND_READ_FN       read;
} SOUND_READER;

typedef struct ModelReader {
    char                ext[20];
    MODEL_READ_FN       read;
} MODEL_READER;


typedef struct AssetReader {
    DATA_READER     *datas;
    TEXT_READER     *texts;
    TEXTURE_READER  *textures;
    SOUND_READER    *sounds;
    MODEL_READER    *models;
    /*size_t          datas_count;
    size_t          texts_count;
    size_t          textures_count;
    size_t          sounds_count;
    size_t          models_count;*/
} ASSET_READER;

extern ASSET_READER asset_reader;

//int read_data(const char *name, SDL_RWops *rw);
int read_text(const char *name, TEXT_DATA *text);
int read_texture(const char *name, IMAGE_DATA *image);
//int read_sound(const char *name, SOUND_DATA *sound);
//int read_vertices(const char *name, VERTICES_INFO *vi);
