#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "video/shader.h"

#define MAX_VERTEX_ARRAY_ATTRIBUTES 5

typedef struct VideoBuffer {
    uint32_t        id;
    uint32_t        target;
    uint32_t        hash;
    uint32_t        usage;

    int             binding_point;
    unsigned int    block_index;
    size_t          size;
    size_t          offset;
} VIDEO_BUFFER;

typedef struct VertexArray {
    uint32_t        id;
    uint32_t        mode;
    uint32_t        type;
    uint32_t        usage;

    uint32_t        vertices_buffer;
    uint32_t        elements_buffer;

    GLintptr        vertices_offset;
    GLintptr        vertices_stride;

    #ifdef GL_ES_VERSION_2_0
    struct VertexArrayAttribute {
        GLuint      attribindex;
        GLint       size;
        GLenum      type;
        GLboolean   normalized;
        size_t      relativeoffset;
    } attributes[MAX_VERTEX_ARRAY_ATTRIBUTES];
    size_t attributes_count;
    #endif // GL_ES_VERSION_2_0
} VERTEX_ARRAY;

#ifndef GL_ES_VERSION_2_0
VIDEO_BUFFER new_uniform_buffer(const void *data, size_t size, unsigned int usage);

void uniform_buffer_connect(int binding_point, const SHADER *shader, VIDEO_BUFFER *buffer, const char *block_name);
void* uniform_buffer_map(const VIDEO_BUFFER *buffer, unsigned int access);
bool uniform_buffer_unmap(void);
#endif // NO GL_ES_VERSION_2_0

VIDEO_BUFFER new_vertex_buffer(const void *vertices, size_t size, unsigned int usage);
VIDEO_BUFFER new_index_buffer(const void *indices, size_t size, unsigned int usage);

#ifndef GL_ES_VERSION_2_0
VIDEO_BUFFER new_texture_buffer(const void *data, size_t size, unsigned int usage);
#endif

void free_video_buffer(VIDEO_BUFFER *buffer);
void bind_video_buffer(VIDEO_BUFFER *buffer);
void unbind_video_buffer(GLuint target);
void video_buffer_subdata(VIDEO_BUFFER *buffer, size_t offset, const void *data, size_t size);
void video_buffer_update(VIDEO_BUFFER *buffer, size_t offset, const void *data, size_t size);

VERTEX_ARRAY new_vertex_array(void);
void free_vertex_array(VERTEX_ARRAY *array);
void bind_vertex_arrays(VERTEX_ARRAY *array);
void unbind_vertex_array(VERTEX_ARRAY *array);
void vertex_array_buffer(VERTEX_ARRAY *array, VIDEO_BUFFER *buffer, GLintptr offset, GLintptr stride);
void vertex_array_format(VERTEX_ARRAY *array, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, size_t relativeoffset);
