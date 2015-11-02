#include "video/commandbuffer.h"
#include "video/stats.h"
#include "core/common.h"
#include <video/gl.h>
#include <assert.h>
#include <memtrack.h>

// maximum uniforms is 1024 or check max_uniform_components
#define DEFAULT_MEMORY_CACHE_SIZE (1024 * sizeof(float) * 4)

static size_t
uniform_size(unsigned int type) {
    size_t sz = 0;
    switch (type) {
    case UNIFORM_INT:
        sz = sizeof(int);
        break;
    case UNIFORM_FLOAT:
        sz = sizeof(float);
        break;
    case UNIFORM_FLOAT2:
        sz = sizeof(float) * 2;
        break;
    case UNIFORM_FLOAT3:
        sz = sizeof(float) * 3;
        break;
    case UNIFORM_FLOAT4:
        sz = sizeof(float) * 4;
        break;
    case UNIFORM_MATRIX3:
        sz = sizeof(float) * 9;
        break;
    case UNIFORM_MATRIX4:
        sz = sizeof(float) * 16;
        break;
    }

    return sz;
}

extern unsigned int
get_gl_uniform_type(unsigned int type) {
    unsigned int gl_type = GL_ZERO;
    switch (type) {
    case UNIFORM_INT:
        gl_type = GL_INT;
        break;
    case UNIFORM_FLOAT:
        gl_type = GL_FLOAT;
        break;
    case UNIFORM_FLOAT2:
        gl_type = GL_FLOAT_VEC2;
        break;
    case UNIFORM_FLOAT3:
        gl_type = GL_FLOAT_VEC3;
        break;
    case UNIFORM_FLOAT4:
        gl_type = GL_FLOAT_VEC4;
        break;
    case UNIFORM_MATRIX3:
        gl_type = GL_FLOAT_MAT3;
        break;
    case UNIFORM_MATRIX4:
        gl_type = GL_FLOAT_MAT4;
        break;
    }

    return gl_type;
}

static inline void
dispatch_uniform(const VIDEO_COMMAND_BUFFER *restrict buffer, uint32_t offset, int location, uint32_t type, uint32_t count) {
    switch (type) {
    case UNIFORM_INT:
        glUniform1iv(location, count, (const int*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_FLOAT:
        glUniform1fv(location, count, (const float*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_FLOAT2:
        glUniform2fv(location, count, (const float*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_FLOAT3:
        glUniform3fv(location, count, (const float*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_FLOAT4:
        glUniform4fv(location, count, (const float*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_MATRIX3:
        glUniformMatrix3fv(location, count, GL_FALSE, (const float*)((const char*)buffer->memory + offset));
        break;
    case UNIFORM_MATRIX4:
        glUniformMatrix4fv(location, count, GL_FALSE, (const float*)((const char*)buffer->memory + offset));
        break;
    }
}

static void
apply_default_states(const VIDEO_COMMAND_BUFFER *buffer) {
    if (buffer->flags & VCB_NO_STATES_BIT)
        return;

    if (buffer->texture_bindings) {
    }

    if (buffer->shader_bindings)
        glUseProgram(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

#ifndef GL_ES_VERSION_2_0
    glDisable(GL_DEPTH_CLAMP);
#endif

    if (buffer->depth.stencil_test) {
        glStencilOpSeparate(GL_FRONT_AND_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
        glDisable(GL_STENCIL_TEST);
    }

#ifndef GL_ES_VERSION_2_0
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_RASTERIZER_DISCARD);
#endif
    glDisable(GL_CULL_FACE);

    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // set default framebuffer
}

static void
apply_states(const VIDEO_COMMAND_BUFFER *buffer) {
    if (buffer->flags & VCB_NO_STATES_BIT)
        return;

    if (buffer->blend.enable) {
        glEnable(GL_BLEND);
        glBlendFunc(buffer->blend.sfactor, buffer->blend.dfactor); // TODO : more blend control
    }

    if (buffer->rasterizer.cull_mode != GL_NONE) {
        glEnable(GL_CULL_FACE);
        glCullFace(buffer->rasterizer.cull_mode);
    }

#ifndef GL_ES_VERSION_2_0
    if (buffer->rasterizer.fill_mode != GL_NONE) {
        glPolygonMode(GL_FRONT_AND_BACK, buffer->rasterizer.fill_mode);
    }

    if (buffer->rasterizer.discard)
        glEnable(GL_RASTERIZER_DISCARD);
#endif

    if (buffer->depth.depth_test) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(buffer->depth.depth_func);
    }

    if (buffer->depth.depth_write)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

#ifndef GL_ES_VERSION_2_0
    if (buffer->depth.depth_clamp) {
        glEnable(GL_DEPTH_CLAMP);
        //glDepthRange(1.f, 1000.f);
    }
    else
        glDisable(GL_DEPTH_CLAMP);
#endif // NO GL_ES_VERSION_2_0

    if (buffer->depth.stencil_test) {
        glEnable(GL_STENCIL_TEST);

        // initial func GL_ALWAYS
        glStencilFuncSeparate(GL_FRONT, buffer->depth.front.func, 0, buffer->depth.front.mask);
        glStencilFuncSeparate(GL_BACK, buffer->depth.back.func, 0, buffer->depth.back.mask);

        // initial value is GL_KEEP for all
        glStencilOpSeparate(GL_FRONT, buffer->depth.front.sfail, buffer->depth.front.dpfail, buffer->depth.front.dppass);
        glStencilOpSeparate(GL_BACK, buffer->depth.back.sfail, buffer->depth.back.dpfail, buffer->depth.back.dppass);
    }
}

extern VIDEO_COMMAND_BUFFER
new_command_buffer(const VIDEO_COMMAND_BUFFER_INFO *info) {
    static uint32_t cmd_buffer_id = 0;
    size_t sz = info->memory_cache_size;

    // TODO: whats wrong?
    //sif (sz == 0)
        sz = 65536;//DEFAULT_MEMORY_CACHE_SIZE;

    void *mem = malloc(sz);

    return (VIDEO_COMMAND_BUFFER){.id = cmd_buffer_id++, .flags = 0, .memory = mem, .memory_size = sz, .memory_offset = 0};
}

extern void
free_command_buffer(VIDEO_COMMAND_BUFFER *buffer) {
    free(buffer->memory);
    buffer->memory = NULL;
    buffer->flags = 0;
}

// NOTE: static dispatching is better?
/*static void
dispatch_clear_command(const VIDEO_COMMAND_BUFFER *restrict buffer, const VIDEO_COMMAND *restrict command) {
    UNUSED(buffer);
    glViewport(command->viewport.x, command->viewport.y, command->viewport.width, command->viewport.height);
}

typedef void (*COMMAND_DISPATCH_FN)(const VIDEO_COMMAND_BUFFER *restrict, const VIDEO_COMMAND *restrict);

static COMMAND_DISPATCH_FN dispatch_functions[] = {
    [VC_CLEAR_COMMAND] = dispatch_clear_command
};*/

extern void
clear_command_buffers(size_t count, VIDEO_COMMAND_BUFFER **buffers) {
    for (size_t i = 0; i < count; i++) {
        VIDEO_COMMAND_BUFFER *buffer = buffers[i];

        buffer->shader_bindings = 0;
        buffer->texture_bindings = 0;
        buffer->commands_count = 0;
        buffer->memory_offset = 0;
    }
}

extern int
submit_command_buffers(size_t count, VIDEO_COMMAND_BUFFER **buffers) {
    videostats_reset();

    for (size_t j = 0; j < count; j++) {
        VIDEO_COMMAND_BUFFER *buffer = buffers[j];

        if (!buffer)
            break;

        apply_states(buffer);

        for (int i = 0; i < buffer->commands_count; i++) {
            VIDEO_COMMAND *command = &buffer->commands[i];

            switch (buffer->commands[i].type) {
            case VC_VIEWPORT_COMMAND:
                glViewport(command->viewport.x, command->viewport.y, command->viewport.width, command->viewport.height);
                break;
            case VC_CLEAR_COMMAND:
                glClearColor(buffer->color[0], buffer->color[1], buffer->color[2], buffer->color[3]);
                glClear(buffer->mask);
                break;

            case VC_DRAW_ARRAY_COMMAND:
                glDrawArrays(command->draw_array.mode, command->draw_array.first, command->draw_array.count);
                break;
            case VC_DRAW_ELEMENTS_COMMAND:
                glDrawElementsBaseVertex(command->draw_elements.mode,
                                         command->draw_elements.count,
                                         command->draw_elements.type,
                                         (const void*)(uintptr_t)command->draw_elements.indices_offset,
                                         command->draw_elements.base_vertex);
                video_stats.dips++;
                break;
            case VC_DRAW_ELEMENTS_INSTANCED_COMMAND:
                glDrawElementsInstancedBaseVertex(command->draw_elements_instanced.mode,
                                                  command->draw_elements_instanced.count,
                                                  command->draw_elements_instanced.type,
                                                  (const void*)(uintptr_t)command->draw_elements_instanced.indices_offset,
                                                  command->draw_elements_instanced.instances,
                                                  command->draw_elements_instanced.base_vertex);
                break;

            case VC_TARGET_DRAW_BUFFER_COMMAND:
                glDrawBuffer(command->target_draw_buffer.buf);
                break;
            case VC_BIND_FRAMEBUFFER_COMMAND:
                glBindFramebuffer(GL_FRAMEBUFFER, command->bind_framebuffer.framebuffer);
                break;
            case VC_BIND_TARGET_COMMAND:
                glFramebufferTexture2D(GL_FRAMEBUFFER, command->bind_target.attachment, command->bind_target.target, command->bind_target.texture, 0);
                break;
            case VC_BIND_SHADER_COMMAND:
                glUseProgram(command->bind_shader.program);
                buffer->shader_bindings++;
                break;
            case VC_BIND_TEXTURE_COMMAND:
                glActiveTexture(GL_TEXTURE0 + command->bind_texture.unit);
                glBindTexture(command->bind_texture.target, command->bind_texture.texture);
                glBindSampler(command->bind_texture.unit, command->bind_texture.sampler);

                glUniform1i(command->bind_texture.location, command->bind_texture.unit);
                buffer->texture_bindings++;
                break;
            case VC_BIND_VERTEX_ARRAY_COMMAND:
                glBindVertexArray(command->bind_vertex_array.array);
                break;

            case VC_UNIFORM_COMMAND:
                dispatch_uniform(buffer, command->uniform.offset, command->uniform.location, command->uniform.type, command->uniform.count);
                break;
            }
        }

        apply_default_states(buffer);
    }

    return 0;
}

extern void
clear_command(VIDEO_COMMAND_BUFFER *cb) {
    assert(cb != NULL);

    cb->commands[cb->commands_count] = (VIDEO_COMMAND) {.type = VC_CLEAR_COMMAND};
    cb->commands_count++;
}

extern void
viewport_command(VIDEO_COMMAND_BUFFER *cb, int viewport[4]) {
    assert(cb != NULL);

    cb->commands[cb->commands_count] = (VIDEO_COMMAND) {.type = VC_VIEWPORT_COMMAND,
            .viewport = {viewport[0], viewport[1], viewport[2], viewport[3]}};
    cb->commands_count++;
}

extern void
draw_array_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t count) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_DRAW_ARRAY_COMMAND;
    cmd.draw_array.mode = mode;
    cmd.draw_array.first = 0;
    cmd.draw_array.count = count;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
draw_elements_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t type, uint32_t count, uint32_t offset, uint32_t base_vertex) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_DRAW_ELEMENTS_COMMAND;
    cmd.draw_elements.mode = mode;
    cmd.draw_elements.count = count;
    cmd.draw_elements.type = type;
    cmd.draw_elements.base_vertex = base_vertex;
    cmd.draw_elements.indices_offset = offset;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
draw_elements_instanced_command(VIDEO_COMMAND_BUFFER *cb, uint32_t mode, uint32_t type, uint32_t count, uint32_t offset, uint32_t base_vertex, uint32_t instances) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_DRAW_ELEMENTS_INSTANCED_COMMAND;
    cmd.draw_elements_instanced.mode = mode;
    cmd.draw_elements_instanced.type = type;
    cmd.draw_elements_instanced.count = count;
    cmd.draw_elements_instanced.base_vertex = base_vertex;
    cmd.draw_elements_instanced.indices_offset = offset;
    cmd.draw_elements_instanced.instances = instances;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
target_draw_buffer_command(VIDEO_COMMAND_BUFFER *cb, uint32_t target, uint32_t buf) {
    assert(cb != NULL);

    cb->commands[cb->commands_count] = (VIDEO_COMMAND){.type = VC_TARGET_DRAW_BUFFER_COMMAND,
            .target_draw_buffer = {.target = target, .buf = buf}};
    cb->commands_count++;
}

extern void
bind_framebuffer_command(VIDEO_COMMAND_BUFFER *cb, uint32_t franmebuffer) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_BIND_FRAMEBUFFER_COMMAND;
    cmd.bind_framebuffer.framebuffer = franmebuffer;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
bind_target_command(VIDEO_COMMAND_BUFFER *cb, uint32_t attachment, uint32_t target, uint32_t texture) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_BIND_TARGET_COMMAND;
    cmd.bind_target.attachment = attachment;
    cmd.bind_target.target = target;
    cmd.bind_target.texture = texture;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
bind_shader_command(VIDEO_COMMAND_BUFFER *cb, uint32_t program) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_BIND_SHADER_COMMAND;
    cmd.bind_shader.program = program;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
bind_texture_command(VIDEO_COMMAND_BUFFER *cb, uint32_t target, uint32_t unit, uint32_t texture, uint32_t sampler, int location) {
    assert(cb != NULL);

    VIDEO_COMMAND cmd;
    cmd.type = VC_BIND_TEXTURE_COMMAND;
    cmd.bind_texture.target = target;
    cmd.bind_texture.unit = unit;
    cmd.bind_texture.texture = texture;
    cmd.bind_texture.sampler = sampler;
    cmd.bind_texture.location = location;
    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
}

extern void
bind_vertex_array_command(VIDEO_COMMAND_BUFFER *cb, uint32_t va) {
    assert(cb != NULL);

    cb->commands[cb->commands_count] = (VIDEO_COMMAND) {.type = VC_BIND_VERTEX_ARRAY_COMMAND,
            .bind_vertex_array = {.array = va}};
    cb->commands_count++;
}

extern void
uniform_command(VIDEO_COMMAND_BUFFER *cb, int location, uint32_t type, const void *ptr, uint32_t count) {
    assert(cb != NULL);
    assert(ptr != NULL);
    assert(cb->memory_offset < cb->memory_size);

    if (location < 0)
        return;

    size_t sz = uniform_size(type) * count;
    memcpy((char*)cb->memory + cb->memory_offset, ptr, sz);

    VIDEO_COMMAND cmd;
    cmd.type = VC_UNIFORM_COMMAND;
    cmd.uniform.location = location;
    cmd.uniform.type = type;
    cmd.uniform.size = sz;
    cmd.uniform.count = count;
    cmd.uniform.offset = cb->memory_offset;

    cb->commands[cb->commands_count] = cmd;
    cb->commands_count++;
    cb->memory_offset += cmd.uniform.size;
}
