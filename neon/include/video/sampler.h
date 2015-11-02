#pragma once

#include <video/gl.h>

typedef struct SamplerInfo {
    int mag_filter;
    int min_filter;
    int wrap_s, wrap_t, wrap_r;
    int wrap; // same as wrap_s, wrap_t, wrap_r but apply for all
    int compare_mode;
    int compare_func;
    int anisotropy;
} SAMPLER_INFO;

typedef struct Sampler {
    GLuint          id;
    uint32_t        usage;
#ifdef GL_ES_VERSION_2_0
    int mag_filter;
    int min_filter;
    int wrap_s, wrap_t;
#endif
} SAMPLER;

SAMPLER new_sampler(const SAMPLER_INFO *info);
void free_sampler(SAMPLER *sampler);

void bind_sampler(uint32_t unit, SAMPLER *sam);
void unbind_sampler(uint32_t unit, uint32_t target);
