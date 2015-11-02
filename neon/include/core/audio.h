#pragma once

#ifdef OPENAL_BACKEND

#include <AL/al.h>
#include <AL/alc.h>

#define MAX_SOUND_SOURCES 16

typedef struct Sound {
    ALuint      buffer;
    ALsizei     size;
    ALenum      format;
    ALsizei     frequency;
} SOUND;

extern ALuint sound_sources[MAX_SOUND_SOURCES];
#elif defined(SDL_AUDIO_BACKEND)
typedef struct Sound {
    Uint8   *buffer;
    Uint32  size;
    bool    looped;
    bool    playing;
    SDL_AudioSpec spec;
} SOUND;
#endif

void audio_init(void);
void audio_cleanup(void);

void openal_init(void);
void openal_cleanup(void);
void openal_pause(void);
void openal_unpause(void);

