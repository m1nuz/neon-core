#pragma once

#include <stddef.h>
#include <sys/types.h>

#include "video/state.h"

#define MAX_COMMANDS 512

enum UniformType {
    UNIFORM_INT,
    UNIFORM_FLOAT,
    UNIFORM_FLOAT2,
    UNIFORM_FLOAT3,
    UNIFORM_FLOAT4,
    UNIFORM_MATRIX3,
    UNIFORM_MATRIX4,
};

enum VideoCommandType {
    VC_VIEWPORT_COMMAND,
    VC_CLEAR_COMMAND,

    VC_DRAW_ARRAY_COMMAND,
    VC_DRAW_ELEMENTS_COMMAND,
    VC_DRAW_ELEMENTS_INSTANCED_COMMAND,

    VC_TARGET_DRAW_BUFFER_COMMAND,

    VC_BIND_FRAMEBUFFER_COMMAND,
    VC_BIND_TARGET_COMMAND,
    VC_BIND_SHADER_COMMAND,
    VC_BIND_TEXTURE_COMMAND,
    VC_BIND_VERTEX_ARRAY_COMMAND,

    VC_UNIFORM_COMMAND
};

// TODO: set flags for seperate state usage
enum VideoCommandBufferFlags {
    VCB_NO_STATES_BIT = 0x0001
};

typedef struct VideoCommand {
    int type;
    union {
        struct {
            int32_t x;
            int32_t y;
            int32_t width;
            int32_t height;
        } viewport;

        /*struct {
        } clear;*/

        struct {
            uint32_t mode;
            uint32_t count;
            uint32_t first;
        } draw_array;

        struct {
            uint32_t mode;
            uint32_t type;
            uint32_t count;
            uint32_t base_vertex;
            uint32_t indices_offset;
        } draw_elements;

        struct {
            uint32_t mode;
            uint32_t type;
            uint32_t count;
            uint32_t instances;
            uint32_t base_vertex;
            uint32_t indices_offset;
        } draw_elements_instanced;

        struct {
            uint32_t target;
            uint32_t buf;
        } target_draw_buffer;

        struct {
            uint32_t framebuffer;
        } bind_framebuffer;

        struct {
            uint32_t attachment;
            uint32_t target;
            uint32_t texture;
        } bind_target;

        struct {
            uint32_t program;
        } bind_shader;

        struct {
            uint32_t unit;
            uint32_t target;
            uint32_t texture;
            uint32_t sampler;
            int location;
        } bind_texture;

        struct {
            uint32_t array;
        } bind_vertex_array;

        struct {
            int location;
            uint32_t type;
            uint32_t size;
            uint32_t offset;
            uint32_t count;
        } uniform;
    };
} VIDEO_COMMAND;

typedef struct VideoCommandBuffer {
    int                 target; // framenuffer
    int                 result; // framenuffer
    unsigned int        mask;
    float               color[4]; // rgba

    uint32_t                id;
    uint32_t                flags;

    VIDEO_COMMAND       commands[MAX_COMMANDS]; // TODO: make dynamic
    int                 commands_count;

    size_t              shader_bindings;
    size_t              texture_bindings;

    COLOR_BLEND_STATE   blend;
    RASTERIZER_STATE    rasterizer;
    DEPTH_STENCIL_STATE depth;

    // for unifroms
    void                *memory;
    size_t              memory_size;
    size_t              memory_offset;
} VIDEO_COMMAND_BUFFER;

typedef struct VideoCommandBufferInfo {
    size_t memory_cache_size;
} VIDEO_COMMAND_BUFFER_INFO;

unsigned int get_gl_uniform_type(unsigned int type);

VIDEO_COMMAND_BUFFER new_command_buffer(const VIDEO_COMMAND_BUFFER_INFO *info);
void free_command_buffer(VIDEO_COMMAND_BUFFER *buffer);

void clear_command_buffers(size_t count, VIDEO_COMMAND_BUFFER **buffers);
int submit_command_buffers(size_t count, VIDEO_COMMAND_BUFFER **buffers);

void clear_command(VIDEO_COMMAND_BUFFER *cb);
void viewport_command(VIDEO_COMMAND_BUFFER *cb, int viewport[4]);

void draw_array_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t count);
void draw_elements_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t type, uint32_t count, uint32_t offset, uint32_t base_vertex);
void draw_elements_instanced_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t type, uint32_t count, uint32_t offset, uint32_t base_vertex, uint32_t instances);

void target_draw_buffer_command(VIDEO_COMMAND_BUFFER *cb, uint32_t target, uint32_t buf);

void bind_framebuffer_command(VIDEO_COMMAND_BUFFER *cb, uint32_t franmebuffer);
void bind_target_command(VIDEO_COMMAND_BUFFER *cb, uint32_t attachment, uint32_t target, uint32_t texture);
void bind_shader_command(VIDEO_COMMAND_BUFFER *cb, uint32_t program);
void bind_texture_command(VIDEO_COMMAND_BUFFER *cb, uint32_t target, uint32_t unit, uint32_t texture, uint32_t sampler, int location);
void bind_vertex_array_command(VIDEO_COMMAND_BUFFER *cb, uint32_t va);

void uniform_command(VIDEO_COMMAND_BUFFER *cb, int location, uint32_t type, const void *ptr, uint32_t count);
