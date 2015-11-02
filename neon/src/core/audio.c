#include <assert.h>
#include <stdbool.h>

#include <core/audio.h>
#include "core/logerr.h"
#include "core/common.h"

// OpenAL

#ifdef OPENAL_BACKEND

static ALCdevice    *audio_device;
static ALCcontext   *audio_context;

#define AL_LOG_ERROR(f) LOG_CRITICAL(#f" %s\n", alcGetString(audio_device, alcGetError(audio_device)))

ALuint       sound_sources[MAX_SOUND_SOURCES];

void
openal_init(void) {
    if (!(audio_device = alcOpenDevice(0))) {
        LOG_CRITICAL("Default sound device not present %p\n", (void*)audio_device);
        exit(EXIT_FAILURE);
    }

    if(!(audio_context = alcCreateContext(audio_device, 0))) {
        AL_LOG_ERROR("alcCreateContext");
        exit(EXIT_FAILURE);
    }

    if(!alcMakeContextCurrent(audio_context)) {
        AL_LOG_ERROR("alcMakeContextCurrent");
        exit(EXIT_FAILURE);
    }

    // Create empty sources
    alGenSources(MAX_SOUND_SOURCES, sound_sources);

    // Init default listener
    const ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, ori);
}

void
openal_cleanup(void) {
    alDeleteSources(MAX_SOUND_SOURCES, sound_sources);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio_context);

    alcCloseDevice(audio_device);
}

void
openal_pause(void) {
    for (int i = 0; i < MAX_SOUND_SOURCES; i++)
        alSourcePause(sound_sources[i]);
}

void
openal_unpause(void) {
    for (int i = 0; i < MAX_SOUND_SOURCES; i++)
        alSourcePlay(sound_sources[i]);
}

extern void
audio_init(void) {
#ifdef OPENAL_BACKEND
    openal_init();
#endif // OPENAL_BACKEND

#ifdef SDL_AUDIO_BACKEND
    SDL_audio_init();
#endif // SDL_AUDIO_BACKEND
}

extern void
audio_cleanup(void) {
#ifdef OPENAL_BACKEND
    openal_cleanup();
#endif // OPENAL_BACKEND

#ifdef SDL_AUDIO_BACKEND
    SDL_audio_cleanup();
#endif // SDL_AUDIO_BACKEND
}

#endif // OPENAL_BACKEND
