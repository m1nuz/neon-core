#include <assert.h>
#include <core/logerr.h>
#include <video/vertex.h>
#include <video/resources.h>
#include <video/resources_detail.h>

VIDEO_RESOURCES all_resources;

size_t textures_count;
size_t samplers_count;
size_t renderbuffers_count;
size_t framebuffers_count;
size_t buffers_count;
size_t vertex_arrays_count;
size_t shaders_count;

extern void
resources_init(void) {
}

extern void
resources_cleanup(void) {
    for (int i = 0; i < MAX_TEXTURES; i++)
        if (all_resources.textures[i].id != 0)
            free_texture(&all_resources.textures[i]);

    for (int i = 0; i < MAX_SAMPLERS; i++)
        if (all_resources.samplers[i].id != 0)
            free_sampler(&all_resources.samplers[i]);

    for (int i = 0; i < MAX_RENDERBUFFERS; i++)
        if (all_resources.renderbuffers[i].id)
            free_renderbuffer(&all_resources.renderbuffers[i]);

    for (int i = 0; i < MAX_FRAMEBUFFERS; i++)
        if (all_resources.framebuffers[i].id)
            free_framebuffer(&all_resources.framebuffers[i]);

    for (int i = 0; i < MAX_BUFFERS; i++)
        if (all_resources.buffers[i].id)
            free_video_buffer(&all_resources.buffers[i]);

    for (int i = 0; i < MAX_VERTEX_ARRAYS; i++)
        if (all_resources.arrays[i].id)
            free_vertex_array(&all_resources.arrays[i]);

    for (int i = 0; i < MAX_SHADERS; i++)
        free_shader(&all_resources.shaders[i]);        
}

void
resources_process(void) {

}

static TEXTURE*
find_texture_by_hash(const TEXTURE *tex) {
    if (tex->hash == 0)
        return NULL;

    for (size_t i = 0; i < textures_count; i++)
        if (tex->hash == all_resources.textures[i].hash)
            return &all_resources.textures[i];

    return NULL;
}

extern TEXTURE*
store_texture(const TEXTURE texture) {
    if (texture.usage != 0)
        LOG_WARNING("Texture %u already used.\n", texture.id);

    TEXTURE *tex = find_texture_by_hash(&texture);
    if (tex) {
        tex->usage++;
        return tex;
    }

    all_resources.textures[textures_count] = texture;
    all_resources.textures[textures_count].usage = 1;

    textures_count++;

    return &all_resources.textures[textures_count - 1];
}

extern SAMPLER*
store_sampler(const SAMPLER sampler) {
    all_resources.samplers[samplers_count] = sampler;
    all_resources.samplers[samplers_count].usage = 1;

    samplers_count++;

    return &all_resources.samplers[samplers_count - 1];
}

extern RENDERBUFFER*
store_renderbuffer(const RENDERBUFFER renderbuffer) {
    all_resources.renderbuffers[renderbuffers_count] = renderbuffer;
    all_resources.renderbuffers[renderbuffers_count].usage = 1;

    renderbuffers_count++;

    return &all_resources.renderbuffers[renderbuffers_count - 1];
}

extern FRAMEBUFFER*
store_framebuffer(const FRAMEBUFFER framebuffer) {
    all_resources.framebuffers[framebuffers_count] = framebuffer;
    all_resources.framebuffers[framebuffers_count].usage = 1;

    framebuffers_count++;

    return &all_resources.framebuffers[framebuffers_count - 1];
}

extern VIDEO_BUFFER*
store_buffer(const VIDEO_BUFFER buffer) {
    all_resources.buffers[buffers_count] = buffer;
    all_resources.buffers[buffers_count].usage = 1;

    buffers_count++;

    return &all_resources.buffers[buffers_count - 1];
}

extern VERTEX_ARRAY*
store_vertex_array(const VERTEX_ARRAY array) {
    all_resources.arrays[vertex_arrays_count] = array;
    all_resources.arrays[vertex_arrays_count].usage = 1;

    vertex_arrays_count++;

    return &all_resources.arrays[vertex_arrays_count - 1];
}

extern SHADER*
store_shader(const SHADER shader) {
    all_resources.shaders[shaders_count] = shader;
    all_resources.shaders[shaders_count].usage = 1;

    shaders_count++;

    return &all_resources.shaders[shaders_count - 1];
}

extern VERTEX_ARRAY*
store_vertices_buffers(const VIDEO_VERTICES_INFO *info) {
    store_buffer(info->vertices);
    store_buffer(info->elements);

    return store_vertex_array(info->array);
}
