#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Screen {
    int     width;
    int     height;
    int     vsync;
    float   aspect;
    bool    fullscreen;
    int     msaa;
    bool    srgb_capable;
} SCREEN;

typedef struct VideoMode {
    int     width;
    int     height;
    int     refresh_rate;
} VIDEO_MODE;

typedef struct VideoInfo {
    int32_t         i_rgb8;
    int32_t         i_rgba8;
    int32_t         i_depth;
    uint32_t        f_rgb;
    uint32_t        f_rgba;
    bool            debug;

    VIDEO_MODE      *modes;
    int             modes_count;

    int             max_supported_samples;
    int             max_uniform_components;
    int             max_vertex_attribs;
    int             max_texture_anisotropy;
    int             max_combined_texture_image_units;
    int             max_uniform_block_size;
    int             max_texture_buffer_size;
    int             max_color_attachments;
    int             max_transform_feedback_interleaved_components;
    int             max_transform_feedback_separate_components;
} VIDEO_INFO;

int find_display_mode(int width, int height);

void video_init(const char *title);
void video_cleanup(void);
void video_swap_buffers(void);
void video_warp_mouse(int x, int y);
void video_show_cursor(bool show);
void video_apply_changes(void);
void video_set_title(const char *title);

extern SCREEN           screen;
extern VIDEO_INFO       video;
