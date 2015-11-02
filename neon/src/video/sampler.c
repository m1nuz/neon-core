#include <assert.h>
#include <video/sampler.h>
#include <core/common.h>

#ifndef GL_ES_VERSION_2_0
#include <GL/ext_texture_filter_anisotropic.h>
#endif // NO GL_ES_VERSION_2_0

extern SAMPLER
new_sampler(const SAMPLER_INFO *info) {
#ifndef GL_ES_VERSION_2_0
    GLuint sid;
    glGenSamplers(1, &sid);

    glSamplerParameteri(sid, GL_TEXTURE_MAG_FILTER, info->mag_filter);
    glSamplerParameteri(sid, GL_TEXTURE_MIN_FILTER, info->min_filter);

    if (info->wrap_s != GL_NONE)
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_S, info->wrap_s);

    if (info->wrap_t != GL_NONE)
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_T, info->wrap_t);

    if (info->wrap_r != GL_NONE)
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_R, info->wrap_r);

    if (info->wrap != GL_NONE) {
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_S, info->wrap);
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_T, info->wrap);
        glSamplerParameteri(sid, GL_TEXTURE_WRAP_R, info->wrap);
    }

    if (info->compare_mode != GL_NONE)
        glSamplerParameteri(sid, GL_TEXTURE_COMPARE_MODE, info->compare_mode); // initial GL_NONE

    if (info->compare_func != GL_NONE)
        glSamplerParameteri(sid, GL_TEXTURE_COMPARE_FUNC, info->compare_func); // initial GL_NEVER

    if(info->anisotropy > 0)
        glSamplerParameteri(sid, GL_TEXTURE_MAX_ANISOTROPY_EXT, info->anisotropy);

    return (SAMPLER){.id = sid};
#else
    static GLuint sampler_id = 0;
    sid = sampler_id++;

    return (SAMPLER){.id = sid,
                .mag_filter = mag_filter,
                .min_filter = min_filter,
                .wrap_s = wrap,
                .wrap_t = wrap};
#endif // GL_ES_VERSION_2_0
}

extern void
free_sampler(SAMPLER *sampler) {
    assert(sampler != NULL);

#ifndef GL_ES_VERSION_2_0
    if(glIsSampler(sampler->id))
        glDeleteSamplers(1, &sampler->id);
#endif // NO GL_ES_VERSION_2_0

    sampler->id = 0;
    sampler->usage = 0;
}

extern void
bind_sampler(uint32_t unit, SAMPLER *sam) {
    UNUSED(unit); // you should already bind the active unit
#ifdef GL_ES_VERSION_2_0
    // TODO: no cubemaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sam->mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sam->min_filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sam->wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sam->wrap_t);
#else
    glBindSampler(unit, sam->id);
#endif // NO GL_ES_VERSION_2_0
}

extern void
unbind_sampler(uint32_t unit, uint32_t target) {
    UNUSED(unit);
    UNUSED(target);

    // do nothing
}
