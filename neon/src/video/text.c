#include <assert.h>
#include "video/text.h"
#include "video/sprite.h"
#include "core/video.h"
#include "video/resources.h"
#include "video/gfx.h"
#include "core/glyphs.h"

extern void
get_text_vertices(VERTICES_INFO *info, int w, int h, const char *text, size_t text_size, const float2 position, const float4 color, int font_type) {
    float tw = w;
    float th = h;

    const char *s = text;

    float2 pos;
    copy_vector2(pos, position);

    const int adv_y = glyph_cache_get_font_lineskip(font_type);
    const int fh = glyph_cache_get_font_size(font_type);
    const float spt = 1.f / screen.width;

    size_t count = 0;

    while (*s) {
        if (count > text_size)
            break;

        if (*s == '\n') {
            pos[0] = position[0];
            pos[1] -= spt * adv_y * screen.aspect;
            s++;
            continue;
        }

        GLYPH *glyph = glyph_cache_find(*s, font_type);

        if (!glyph) {
            LOG_WARNING("%s\n", "Glyph not found");
            return;
        }
        //assert((glyph != NULL) && "Glyph not found");

        float2 size = {glyph->advance * spt, fh * spt};
        float4 offset;
        offset[0] = (float)glyph->rc.x / tw;
        offset[1] = (float)glyph->rc.y / th;
        offset[2] = (float)glyph->rc.w / tw;
        offset[3] = (float)glyph->rc.h / th;

        const float correction = screen.aspect;

        const v3t2c4_t vertices[6] = {
            {{pos[0], pos[1] + size[1] * correction, 0}, {offset[0], offset[1]}, {color[0], color[1], color[2], color[3]}},
            {{pos[0] + size[0], pos[1] + size[1] * correction, 0}, {offset[0] + offset[2], offset[1]}, {color[0], color[1], color[2], color[3]}},
            {{pos[0] + size[0], pos[1], 0}, {offset[0] + offset[2], offset[1] + offset[3]}, {color[0], color[1], color[2], color[3]}},
            {{pos[0] + size[0], pos[1], 0}, {offset[0] + offset[2], offset[1] + offset[3]}, {color[0], color[1], color[2], color[3]}},
            {{pos[0], pos[1], 0}, {offset[0], offset[1] + offset[3]}, {color[0], color[1], color[2], color[3]}},
            {{pos[0], pos[1] + size[1] * correction, 0}, {offset[0], offset[1]}, {color[0], color[1], color[2], color[3]}},
        };

        memcpy((char*)info->data.vertices + info->data.vertices_num * sizeof(vertices[0]), vertices, sizeof(vertices));
        info->data.vertices_num += 6;

        pos[0] += glyph->advance * spt;

        s++;
        count++;
    }
}

vec2
get_text_size(const char *text, size_t text_size, int font_type) {
    const char *s = text;

    if (!text)
        return (vec2){0, 0};

    float2 position = {0, 0};
    float2 pos;
    copy_vector2(pos, position);

    const int adv_y = glyph_cache_get_font_lineskip(font_type);
    //const int fh = glyph_cache_get_font_size(font_type);
    const float spt = 1.f / screen.width;

    size_t count = 0;

    while (*s) {
        if (count > text_size)
            break;

        if (*s == '\n') {
            pos[0] = position[0];
            pos[1] -= spt * adv_y * screen.aspect;
            s++;
            continue;
        }

        GLYPH *glyph = glyph_cache_find(*s, font_type);

        if (!glyph) {
            LOG_WARNING("%s\n", "Glyph not found");
            return (vec2){0, 0};
        }
        //assert((glyph != NULL) && "Glyph not found");

        pos[0] += glyph->advance * spt;

        s++;
        count++;
    }

    float msy = ABS(position[1] - pos[1]);
    return (vec2){ABS(position[0] - pos[0]), msy == 0 ? (float)adv_y / screen.height : msy};
}

extern void
submit_text(VIDEO_SPRITE_BATCH *batch, const char *text, const float2 position, const float4 color, int font_type) {
    float tw = batch->texture->width;
    float th = batch->texture->height;

    const char *s = text;

    float2 pos;
    copy_vector2(pos, position);

    const int adv_y = glyph_cache_get_font_lineskip(font_type);
    const int fh = glyph_cache_get_font_size(font_type);
    const float spt = 1.f / screen.width;

    while (*s) {
        if (*s == '\n') {
            pos[0] = position[0];
            pos[1] -= spt * adv_y * screen.aspect;
            s++;
            continue;
        }

        GLYPH *glyph = glyph_cache_find(*s, font_type);

        if (!glyph) {
            LOG_WARNING("%s\n", "Glyph not found");
            return;
        }
        //assert((glyph != NULL) && "Glyph not found");

        float2 sz = {glyph->advance * spt, fh * spt};
        float4 offset;
        offset[0] = (float)glyph->rc.x / tw;
        offset[1] = (float)glyph->rc.y / th;
        offset[2] = (float)glyph->rc.w / tw;
        offset[3] = (float)glyph->rc.h / th;

        sprite_batch_submit_sprite(batch, pos, sz, offset, color);

        pos[0] += glyph->advance * spt;

        s++;
    }
}

