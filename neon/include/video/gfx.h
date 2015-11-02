#pragma once

#include <core/logerr.h>

/*#define gfx_texture(tex) (all_resources.textures[tex])
#define gfx_sampler(sam) (all_resources.samplers[sam])
#define gfx_renderbuffer(rb) (all_resources.renderbuffers[rb])
#define gfx_framebuffer(fb) (all_resources.framebuffers[fb])
#define gfx_uniformbuffer(ub) (all_resources.uniformbuffers[ub])
#define gfx_shader(shd) (all_resources.shaders[shd])*/

#define gfx_uniform1f(shader, name, x) glUniform1f(shader_get_uniform(shader, name), x)
#define gfx_uniform1fv(shader, name, ptr, count) glUniform1fv(shader_get_uniform(shader, name), count, ptr)
#define gfx_uniform2f(shader, name, x, y) glUniform2f(shader_get_uniform(shader, name), x, y)
#define gfx_uniform3f(shader, name, x, y, z) glUniform3f(shader_get_uniform(shader, name), x, y, z)
#define gfx_uniform4f(shader, name, x, y, z, w) glUniform4f(shader_get_uniform(shader, name), x, y, z, w)
#define gfx_uniform4fv(shader, name, ptr, count) glUniform4fv(shader_get_uniform(shader, name), count, ptr)
#define gfx_uniform1i(shader, name, x) glUniform1i(shader_get_uniform(shader, name), x)
#define gfx_uniform_matrix4f(shader, name, m) glUniformMatrix4fv(shader_get_uniform(shader, name), 1, GL_FALSE, m)
#define gfx_uniform_location(shader, name) shader_get_uniform(shader, name)
#define gfx_uniform_buffer_connect(point, shader, buffer, name) uniform_buffer_connect(point, shader, buffer, name)
#define gfx_uniform_buffer_map(buffer, access) uniform_buffer_map(buffer, access)
#define gfx_use_shader(shader) glUseProgram((shader)->pid)
#define gfx_framebuffer_width(fb) ((fb)->width)
#define gfx_framebuffer_height(fb) ((fb)->height)
#define gfx_use_framebuffer(framebuffer) do {\
    glBindFramebuffer(GL_FRAMEBUFFER, (framebuffer)->id); \
    glViewport(0, 0, (framebuffer)->width, (framebuffer)->height); \
    if (screen.srgb_capable) (framebuffer)->id == 0 ? glDisable(GL_FRAMEBUFFER_SRGB) : glEnable(GL_FRAMEBUFFER_SRGB); \
    } while(0)

#define gfx_draw_elements(va, primitive, count) do { \
    glBindVertexArray(va); \
    glDrawElements(primitive, count, GL_UNSIGNED_SHORT, 0); \
    glBindVertexArray(0); \
    } while(0)
#define gfx_draw_arrays(va, primitive, count) do { \
    glBindVertexArray(va); \
    glDrawArrays(primitive, 0, count); \
    glBindVertexArray(0); \
    } while(0)

#define gfx_bind_texture(unit, texture) bind_texture(unit, texture)
#define gfx_bind_sampler(unit, sampler) bind_sampler(unit, sampler)

/*#define gfx_bind_texture(unit, texture, sampler) do { \
    glActiveTexture(GL_TEXTURE0 + (unit)); \
    glBindTexture(gfx_texture(texture).target, gfx_texture(texture).id); \
    glBindSampler(unit, (gfx_sampler(sampler)).id); \
    if((gfx_texture(texture).flags & TEXTURE_MIPMAPS_BIT) && (gfx_texture(texture).id != 0)) \
    glGenerateMipmap(gfx_texture(texture).target); \
    } while(0)*/

#define gfx_unbind_texture(unit, target) unbind_texture(unit, target)
/*#define gfx_unbind_texture(unit, target) do { \
    glActiveTexture(GL_TEXTURE0 + (unit)); \
    glBindSampler(unit, 0); \
    glBindTexture(target, 0); \
    } while(0)*/

#define gfx_clear(target, r, g, b, a) do { \
    glClearColor(r, g, b, a); \
    glClear((target)->mask); \
    } while(0)

// command buffer
#define gfx_clear_command(cb) clear_command(&(cb))
#define gfx_target_draw_buffer_command(cb, target, mode) target_draw_buffer_command(&(cb), gfx_framebuffer(target).id, mode)
#define gfx_viewport_target_command(cb, tar) viewport_command(&(cb), gfx_framebuffer(tar).viewport)
#define gfx_bind_texture_command(cb, unit, tex_name, shd, tex, smp) \
    bind_texture_command(&(cb), gfx_texture(tex).target, unit, gfx_texture(tex).id, gfx_sampler(smp).id, \
                         gfx_uniform_location(shd, tex_name))
#define gfx_bind_shader_command(cb, p) bind_shader_command(&(cb), gfx_shader(p).pid)
#define gfx_bind_target_command(cb, t) bind_framebuffer_command(&(cb), gfx_framebuffer(t).id)
#define gfx_uniform_command(cb, shd, name, type, value, count) do {\
    if (shader_uniform_type(&gfx_shader(shd), name) != get_gl_uniform_type(type)) {\
    LOG_CRITICAL("Invalid uniform type %s (location %d)\n", name, gfx_uniform_location(shd, name)); \
    exit(EXIT_FAILURE);\
    }\
    uniform_command(&(cb), gfx_uniform_location(shd, name), type, &(value), count); \
    } while(0)

#define gfx_bind_vertex_array_command(cb, va) \
    bind_vertex_array_command(&(cb), all_resources.arrays[va].id)

#define gfx_draw_elements_command(cb, va, count, offset, base_vertex) \
    draw_elements_command(&(cb), all_resources.arrays[va].mode, all_resources.arrays[va].type, count, offset, base_vertex);

#define gfx_draw_command(cb, msh) \
    bind_vertex_array(&(cb), gfx_mesh(msh).vertex_array); \
    draw_elements_command(&(cb), gfx_mesh(msh).primitive, GL_UNSIGNED_SHORT, gfx_mesh(msh).count, 0, 0)
