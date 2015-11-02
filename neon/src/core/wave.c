#include <memtrack.h>
#include "core/wave.h"
#include "core/logerr.h"

struct riff_header
{
    char    chunkID[4];
    int     chunkSize;   // size not including chunkSize or chunkID
    char    format[4];
};

struct wave_format
{
    char    subChunkID[4];
    int     subChunkSize;
    short   audioFormat;
    short   numChannels;
    int     sampleRate;
    int     byteRate;
    short   blockAlign;
    short   bitsPerSample;
};

struct wave_data
{
    char    subChunkID[4]; // should contain the word data
    int     subChunk2Size; // Stores the size of the data block
};

extern void*
load_wave(SDL_RWops *rw, ALenum* _format, ALsizei* _frequency, ALsizei* _size) {
    struct riff_header riff;
    struct wave_format format;
    struct wave_data   data;

    if (!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return NULL;
    }

    SDL_RWread(rw, &riff, sizeof(riff), 1);

    if(((riff.chunkID[0] != 'R') || (riff.chunkID[1] != 'I') || (riff.chunkID[2] != 'F') || (riff.chunkID[3] != 'F')) &&
       ((riff.format[0] != 'W') || (riff.format[1] != 'A') || (riff.format[2] != 'V') || (riff.format[3] != 'E'))) {
        LOG_ERROR("%s\n", "Invalid RIFF or WAVE header");
        return NULL;
    }

    SDL_RWread(rw, &format, sizeof(format), 1);

    if((format.subChunkID[0] != 'f') || (format.subChunkID[1] != 'm') || (format.subChunkID[2] != 't') || (format.subChunkID[3] != ' ')) {
        LOG_ERROR("%s\n", "Invalid WAVE format");
        return NULL;
    }

    if(format.subChunkSize > 16)
        SDL_RWseek(rw, sizeof(short), RW_SEEK_CUR);

    SDL_RWread(rw, &data, sizeof(data), 1);

    if(data.subChunkID[0] != 'd' || data.subChunkID[1] != 'a' || data.subChunkID[2] != 't' || data.subChunkID[3] != 'a') {
        fprintf(stderr, "error: Invalid data header\n");
        return NULL;
    }

    void* ptr = malloc(data.subChunk2Size);

    if(!SDL_RWread(rw, ptr, data.subChunk2Size, 1)) {
        LOG_ERROR("%s\n", "Can\'t read WAVE data");
        return NULL;
    }

    *_size = data.subChunk2Size;
    *_frequency = format.sampleRate;

    if(format.numChannels == 1) {
        if(format.bitsPerSample == 8)
            *_format = AL_FORMAT_MONO8;
        else if(format.bitsPerSample == 16)
            *_format = AL_FORMAT_MONO16;
    } else if (format.numChannels == 2) {
        if(format.bitsPerSample == 8)
            *_format = AL_FORMAT_STEREO8;
        else if(format.bitsPerSample == 16)
            *_format = AL_FORMAT_STEREO16;
    }

    SDL_RWclose(rw);

    return ptr;
}
