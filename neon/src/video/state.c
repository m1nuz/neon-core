#include <assert.h>
#include <video/gl.h>
#include "video/state.h"

void
set_color_blend_state(const COLOR_BLEND_STATE *restrict state) {
    assert(state != NULL);

    if (!state->enable)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(state->sfactor, state->dfactor); // TODO : more blend control
}

void
set_rasterizer_state(const RASTERIZER_STATE *restrict state) {
    assert(state != NULL);

    if (state->cull_mode != GL_NONE) {
        glEnable(GL_CULL_FACE);
        glCullFace(state->cull_mode);
    }

#ifndef GL_ES_VERSION_2_0
    if (state->fill_mode != GL_NONE) {
        glPolygonMode(GL_FRONT_AND_BACK, state->fill_mode);
    }

    if (state->discard)
        glEnable(GL_RASTERIZER_DISCARD);
#endif // NO GL_ES_VERSION_2_0
}

void
set_depth_stencil_state(const DEPTH_STENCIL_STATE *restrict state) {
    assert(state != NULL);

    if (state->depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(state->depth_func);
    }

    if (state->depth_write)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

#ifndef GL_ES_VERSION_2_0
    if (state->depth_clamp) {
        glEnable(GL_DEPTH_CLAMP);
        //glDepthRange(1.f, 1000.f);
    }
    else
        glDisable(GL_DEPTH_CLAMP);
#endif // NO GL_ES_VERSION_2_0

    if (state->stencil_test) {
        glEnable(GL_STENCIL_TEST);

        // initial func GL_ALWAYS
        glStencilFuncSeparate(GL_FRONT, state->front.func, 0, state->front.mask);
        glStencilFuncSeparate(GL_BACK, state->back.func, 0, state->back.mask);

        // initial value is GL_KEEP for all
        glStencilOpSeparate(GL_FRONT, state->front.sfail, state->front.dpfail, state->front.dppass);
        glStencilOpSeparate(GL_BACK, state->back.sfail, state->back.dpfail, state->back.dppass);
    }
}

void
clear_color_blend_state(void) {
    glDisable(GL_BLEND);
}

void
clear_rasterizer_state(void) {
#ifndef GL_ES_VERSION_2_0
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_RASTERIZER_DISCARD);
#endif // NO GL_ES_VERSION_2_0
    glDisable(GL_CULL_FACE);
}

void
clear_depth_stencil_state(void) {
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

#ifndef GL_ES_VERSION_2_0
    glDisable(GL_DEPTH_CLAMP);
#endif

    glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
}
