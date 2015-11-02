#include <assert.h>
#include <string.h>

#include <video/gl.h>

#include "core/common.h"
#include "core/logerr.h"
#include "video/buffer.h"

#ifndef GL_ES_VERSION_2_0

extern VIDEO_BUFFER
new_uniform_buffer(const void *data, size_t size, unsigned int usage) {
    GLuint buffer = 0;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferData(GL_UNIFORM_BUFFER, size, data, usage);

    return (VIDEO_BUFFER){.id = buffer, .target = GL_UNIFORM_BUFFER, .binding_point = -1, .block_index = -1, .size = size};
}

extern void
uniform_buffer_connect(int binding_point, const SHADER *shader, VIDEO_BUFFER *buffer, const char *block_name) {
    buffer->binding_point = binding_point;
    glBindBufferRange(GL_UNIFORM_BUFFER, buffer->binding_point, buffer->id, 0, buffer->size);

    buffer->block_index = glGetUniformBlockIndex(shader->pid, block_name);
    glUniformBlockBinding(shader->pid, buffer->block_index, buffer->binding_point);
}

extern void*
uniform_buffer_map(const VIDEO_BUFFER *buffer, unsigned int access) {
    glBindBuffer(GL_UNIFORM_BUFFER, buffer->id);
    return glMapBuffer(GL_UNIFORM_BUFFER, access);
}

extern bool
uniform_buffer_unmap(void) {
    bool result = glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return result;
}

#endif // NO GL_ES_VERSION_2_0

extern VIDEO_BUFFER
new_vertex_buffer(const void *vertices, size_t size, unsigned int usage) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);

    return (VIDEO_BUFFER){.id = buffer, .target = GL_ARRAY_BUFFER, .size = size};
}

extern VIDEO_BUFFER
new_index_buffer(const void *indices, size_t size, unsigned int usage) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);

    return (VIDEO_BUFFER){.id = buffer, .target = GL_ELEMENT_ARRAY_BUFFER, .size = size};
}

#ifndef GL_ES_VERSION_2_0
extern VIDEO_BUFFER
new_texture_buffer(const void *data, size_t size, unsigned int usage) {
    GLuint buffer;
    glGenBuffers(1, &buffer);

    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, size, data, usage);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    return (VIDEO_BUFFER){.id = buffer, .target = GL_TEXTURE_BUFFER, .size = size};
}
#endif // NO GL_ES_VERSION_2_0

extern void
free_video_buffer(VIDEO_BUFFER *buffer) {
    if (glIsBuffer(buffer->id))
        glDeleteBuffers(1, &buffer->id);

    buffer->id = 0;
}

extern void
bind_video_buffer(VIDEO_BUFFER *buffer) {
    glBindBuffer(buffer->target, buffer->id);
}

extern void
unbind_video_buffer(GLuint target) {
    glBindBuffer(target, 0);
}

extern void
video_buffer_subdata(VIDEO_BUFFER *buffer, size_t offset, const void *data, size_t size) {
    glBindBuffer(buffer->target, buffer->id);
    glBufferSubData(buffer->target, offset, size, data);
    //glBindBuffer(buffer->target, 0);
}

extern void
video_buffer_update(VIDEO_BUFFER *buffer, size_t offset, const void *data, size_t size) {    
    glBindBuffer(buffer->target, buffer->id);
#ifndef GL_ES_VERSION_2_0
    void *ptr = glMapBufferRange(buffer->target, offset, size, GL_MAP_WRITE_BIT);

    memcpy(ptr, data, size);

    glUnmapBuffer(buffer->target);

    //glBindBuffer(buffer->target, 0);
#else
    glBufferSubData(buffer->target, offset, size, data);
#endif // GL_ES_VERSION_2_0
}

extern VERTEX_ARRAY
new_vertex_array(void) {
#ifndef GL_ES_VERSION_2_0
    GLuint va;
    glGenVertexArrays(1, &va);

    return (VERTEX_ARRAY){.id = va};
#else
    static GLuint vertex_array_id = 1;
    return (VERTEX_ARRAY){.id = vertex_array_id++};
#endif // GL_ES_VERSION_2_0
}

extern void
free_vertex_array(VERTEX_ARRAY *array) {
#ifndef GL_ES_VERSION_2_0
    if (glIsVertexArray(array->id))
        glDeleteVertexArrays(1, &array->id);

    array->id = 0;
#else
    memset(array, 0, sizeof(VERTEX_ARRAY));
#endif // GL_ES_VERSION_2_0
}

extern void
bind_vertex_arrays(VERTEX_ARRAY *array) {
#ifndef GL_ES_VERSION_2_0
    glBindVertexArray(array->id);
#else
    if (array->vertices_buffer) {
        glBindBuffer(GL_ARRAY_BUFFER, array->vertices_buffer);

        for (size_t i = 0; i < array->attributes_count; i++) {
            glVertexAttribPointer(array->attributes[i].attribindex, array->attributes[i].size,
                                  array->attributes[i].type, array->attributes[i].normalized,
                                  array->vertices_stride, (const GLvoid*)(array->vertices_offset + array->attributes[i].relativeoffset));
            glEnableVertexAttribArray(array->attributes[i].attribindex);
        }
    }

    if (array->elements_buffer)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, array->elements_buffer);
#endif // GL_ES_VERSION_2_0
}

extern void
unbind_vertex_array(VERTEX_ARRAY *array) {
#ifndef GL_ES_VERSION_2_0
    UNUSED(array);

    glBindVertexArray(0);
#else
    for (size_t i = 0; i < array->attributes_count; i++)
        glDisableVertexAttribArray(array->attributes[i].attribindex);
#endif // GL_ES_VERSION_2_0
}

extern void
vertex_array_buffer(VERTEX_ARRAY *array, VIDEO_BUFFER *buffer, GLintptr offset, GLintptr stride) {
    assert(array != NULL);
    assert(array != NULL);

    if (buffer->target == GL_ARRAY_BUFFER) {
        array->vertices_buffer = buffer->id;
        array->vertices_offset = offset;
        array->vertices_stride = stride;

        //glBindBuffer(GL_ARRAY_BUFFER, array->vertices_buffer);
    }

    if (buffer->target == GL_ELEMENT_ARRAY_BUFFER) {
        array->elements_buffer = buffer->id;

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, array->elements_buffer);
    }
}

extern void
vertex_array_format(VERTEX_ARRAY *array, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, size_t relativeoffset) {
#ifdef GL_VERSION_3_3 /* and not defined GL_VERSION_4_5 */
    glVertexAttribPointer(attribindex, size, type, normalized, array->vertices_stride, (const GLvoid*)(array->vertices_offset + relativeoffset));
    glEnableVertexAttribArray(attribindex); // should i do that?
#endif

#ifdef GL_ES_VERSION_2_0
    // save attribute params
    if (array->attributes_count > MAX_VERTEX_ARRAY_ATTRIBUTES)
        LOG_ERROR("%s\n", "attributes_count overflow");

    array->attributes[array->attributes_count] = (struct VertexArrayAttribute) {.attribindex = attribindex,
            .size = size,
            .type = type,
            .normalized = normalized,
            .relativeoffset = relativeoffset};
    array->attributes_count++;
#endif // GL_ES_VERSION_2_0
}
