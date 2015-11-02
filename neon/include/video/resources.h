/*
 * Video objects manager
 */
#pragma once

#include "video/framebuffer.h"
#include "video/texture.h"
#include "video/shader.h"
#include "video/buffer.h"
#include "video/vertices.h"

enum VideoLimits {
    MAX_TEXTURES        = 20,
    MAX_SAMPLERS        = 5,
    MAX_RENDERBUFFERS   = 5,
    MAX_FRAMEBUFFERS    = 10,
    MAX_BUFFERS         = 20,
    MAX_VERTEX_ARRAYS   = 10,
    MAX_SHADERS         = 20
};

typedef struct VideoResources {
    TEXTURE         textures[MAX_TEXTURES];
    SAMPLER         samplers[MAX_SAMPLERS];
    RENDERBUFFER    renderbuffers[MAX_RENDERBUFFERS];
    FRAMEBUFFER     framebuffers[MAX_FRAMEBUFFERS];
    VIDEO_BUFFER    buffers[MAX_BUFFERS];
    VERTEX_ARRAY    arrays[MAX_VERTEX_ARRAYS];
    SHADER          shaders[MAX_SHADERS];        
} VIDEO_RESOURCES;

#define store_new_texture2D(...) store_texture(new_texture2D(&(IMAGE_DATA){__VA_ARGS__}))
#define store_new_sampler(...) store_sampler(new_sampler(&(SAMPLER_INFO){__VA_ARGS__}))

TEXTURE* store_texture(const TEXTURE texture);
SAMPLER* store_sampler(const SAMPLER sampler);
RENDERBUFFER* store_renderbuffer(const RENDERBUFFER renderbuffer);
FRAMEBUFFER* store_framebuffer(const FRAMEBUFFER framebuffer);
VIDEO_BUFFER* store_buffer(const VIDEO_BUFFER buffer);
VERTEX_ARRAY* store_vertex_array(const VERTEX_ARRAY array);
SHADER* store_shader(const SHADER shader);
VERTEX_ARRAY* store_vertices_buffers(const VIDEO_VERTICES_INFO *info);

extern VIDEO_RESOURCES all_resources;
