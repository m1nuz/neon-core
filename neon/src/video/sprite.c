#include <stdlib.h>
#include <assert.h>
#include <video/gl.h>
#include <memtrack.h>
#include "video/sprite.h"
#include "video/shader.h"
#include "core/video.h"
#include "video/buffer.h"

static void
sprite_batch_create_buffers(VIDEO_SPRITE_BATCH *batch) {
    batch->va = new_vertex_array();
    bind_vertex_arrays(&batch->va);
    batch->vb = new_vertex_buffer(NULL, batch->capacity * 4 * sizeof(v3t2c4_t), GL_DYNAMIC_DRAW);
    batch->ib = new_index_buffer(batch->indices, sizeof(uint16_t) * batch->capacity * 6, GL_STATIC_DRAW);

    vertex_array_buffer(&batch->va, &batch->vb, 0, sizeof(v3t2c4_t));
    vertex_array_format(&batch->va, POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3t2c4_t, position));
    vertex_array_format(&batch->va, TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, offsetof(v3t2c4_t, texcoord));
    vertex_array_format(&batch->va, COLOR_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, offsetof(v3t2c4_t, color));
    vertex_array_buffer(&batch->va, &batch->ib, 0, 0);
}

static void
sprite_batch_delete_buffers(VIDEO_SPRITE_BATCH *batch) {
    free_video_buffer(&batch->vb);
    free_video_buffer(&batch->ib);
    free_vertex_array(&batch->va);
}

extern VIDEO_SPRITE_BATCH
new_sprite_batch(const VIDEO_SPRITE_BATCH_INFO *info) {
    // BUG: memory corruption, wtf?
    /*VIDEO_SPRITE_BATCH batch = (VIDEO_SPRITE_BATCH){.texture = info->texture,
            .count = 0,
            .capacity = info->max_sprites,
            .vertices = malloc(sizeof(v3t2c4_t) * batch.capacity * 4),
            .indices = malloc(sizeof(uint16_t) * batch.capacity * 6),
            .vertices_count = 0,
            .indices_count = 0};*/

    VIDEO_SPRITE_BATCH batch;
    memset(&batch, 0, sizeof(batch));
    batch.texture = info->texture;
    batch.capacity = info->max_sprites;
    batch.vertices = malloc(sizeof(v3t2c4_t) * batch.capacity * 4);
    batch.indices = malloc(sizeof(uint16_t) * batch.capacity * 6);

    for (size_t i = 0; i < batch.capacity; i++) {
        batch.indices[i * 6 + 0] = 0 + (i * 4);
        batch.indices[i * 6 + 1] = 1 + (i * 4);
        batch.indices[i * 6 + 2] = 2 + (i * 4);

        batch.indices[i * 6 + 3] = 0 + (i * 4);
        batch.indices[i * 6 + 4] = 2 + (i * 4);
        batch.indices[i * 6 + 5] = 3 + (i * 4);
    }

    sprite_batch_create_buffers(&batch);

    return batch;
}

extern void
free_sprite_batch(VIDEO_SPRITE_BATCH *batch) {
    sprite_batch_delete_buffers(batch);

    free(batch->vertices);
    batch->vertices = NULL;

    free(batch->indices);
    batch->indices = NULL;
}

extern void
sprite_batch_submit_vertices(VIDEO_SPRITE_BATCH *batch, const v3t2c4_t vertices[4]) {
    assert(batch != NULL);
    assert(batch->count < UINT16_MAX);
    assert(batch->count < batch->capacity);
    assert(batch->vertices != NULL);

    memcpy(batch->vertices + batch->vertices_count, vertices, sizeof(v3t2c4_t) * 4);

    batch->vertices_count += 4;
    batch->indices_count += 6;
    batch->count++;
}

extern void
sprite_batch_submit_sprite(VIDEO_SPRITE_BATCH *batch, const float2 position, const float2 size, const float4 offset, const float4 color) {
    const float correction = screen.aspect;

    const v3t2c4_t vertices[4] = {
        {{position[0], position[1] + size[1] * correction, 0}, {offset[0], offset[1]}, {color[0], color[1], color[2], color[3]}},
        {{position[0] + size[0], position[1] + size[1] * correction, 0}, {offset[0] + offset[2], offset[1]}, {color[0], color[1], color[2], color[3]}},
        {{position[0] + size[0], position[1], 0}, {offset[0] + offset[2], offset[1] + offset[3]}, {color[0], color[1], color[2], color[3]}},
        {{position[0], position[1], 0}, {offset[0], offset[1] + offset[3]}, {color[0], color[1], color[2], color[3]}},
    };

    sprite_batch_submit_vertices(batch, vertices);
}

void
submit_batches(VIDEO_SPRITE_BATCH *batches, size_t batches_count, SHADER *shader, SAMPLER *sampler, COLOR_BLEND_STATE blend) {
    for (size_t i = 0; i < batches_count; i++) {
        if(batches[i].count < 1)
            continue;

        video_buffer_subdata(&batches[i].vb, 0, batches[i].vertices, batches[i].vertices_count * sizeof(v3t2c4_t));
    }
    unbind_video_buffer(GL_ARRAY_BUFFER);

    glUseProgram(shader->pid);

    if (blend.enable) {
        glEnable(GL_BLEND);
        glBlendFunc(blend.sfactor, blend.dfactor);
    }

    int loc_sprite = shader_get_uniform(shader, "sprite_map");

    for (size_t i = 0; i < batches_count; i++) {
        if(batches[i].count < 1)
            continue;

        glUniform1i(loc_sprite, 0);

        bind_texture(0, &batches[i].texture);
        bind_sampler(0, sampler);

        bind_vertex_arrays(&batches[i].va);
        glDrawElements(GL_TRIANGLES, batches[i].indices_count, GL_UNSIGNED_SHORT, 0);
        unbind_vertex_array(&batches[i].va);

        glBindTexture(GL_TEXTURE_2D, 0);

        batches[i].count = 0;
        batches[i].vertices_count = 0;
        batches[i].indices_count = 0;
    }

    glDisable(GL_BLEND);

    glUseProgram(0);
}
