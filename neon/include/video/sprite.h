#pragma once

#include <stddef.h>
#include <stdint.h>
#include "base/math_ext.h"
#include "video/vertex.h"
#include "video/state.h"
#include "video/buffer.h"
#include "video/texture.h"

typedef struct VideoSpriteBatchInfo {
    TEXTURE     texture;
    size_t      max_sprites;
} VIDEO_SPRITE_BATCH_INFO;

typedef struct VideoSpriteBatch {
    TEXTURE     texture;

    v3t2c4_t    *vertices;
    uint16_t    *indices;
    size_t      indices_count;
    size_t      vertices_count;

    size_t      capacity;
    size_t      count;

    VERTEX_ARRAY va;
    VIDEO_BUFFER vb;
    VIDEO_BUFFER ib;
} VIDEO_SPRITE_BATCH;

VIDEO_SPRITE_BATCH new_sprite_batch(const VIDEO_SPRITE_BATCH_INFO *info);
void free_sprite_batch(VIDEO_SPRITE_BATCH *batch);

void sprite_batch_submit_vertices(VIDEO_SPRITE_BATCH *batch, const v3t2c4_t vertices[4]);
void sprite_batch_submit_sprite(VIDEO_SPRITE_BATCH *batch, const float2 position, const float2 size, const float4 offset, const float4 color);

void submit_batches(VIDEO_SPRITE_BATCH *batches, size_t batches_count, SHADER *shader, SAMPLER *sampler, COLOR_BLEND_STATE blend);
