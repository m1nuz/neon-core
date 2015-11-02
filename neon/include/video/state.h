#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ColorBlendState {
    bool        enable;
    uint32_t    sfactor;
    uint32_t    dfactor;
} COLOR_BLEND_STATE;

typedef struct RasterizerState {
    uint32_t    fill_mode;
    uint32_t    cull_mode;
    bool        discard;
} RASTERIZER_STATE;

typedef struct DepthStencilState {
    bool        depth_test;
    bool        depth_write;
    bool        depth_clamp;
    uint32_t    depth_func;
    bool        stencil_test;
    struct {
        uint32_t    func;
        uint8_t     mask;

        uint32_t    sfail;
        uint32_t    dpfail;
        uint32_t    dppass;
    } front, back;
} DEPTH_STENCIL_STATE;

void set_color_blend_state(const COLOR_BLEND_STATE *restrict state);
void set_rasterizer_state(const RASTERIZER_STATE *restrict state);
void set_depth_stencil_state(const DEPTH_STENCIL_STATE *restrict state);

void clear_color_blend_state(void);
void clear_rasterizer_state(void);
void clear_depth_stencil_state(void);
