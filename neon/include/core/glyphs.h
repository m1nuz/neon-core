#pragma once

#include <stddef.h>
#include <SDL2/SDL_rect.h>

#include "video/texture.h"
#include "core/atlas.h"

typedef struct Glyph {
    Uint16      ch;
    SDL_Rect    rc;
    int         advance;
    int         type;
} GLYPH;

typedef struct GlyphGroup {
    int         type;
    int         size;
    int         lineskip;
} GLYPH_GROUP;

/*typedef struct GlyphsCache {
    struct Glyph        *glyphs;
    struct GlyphGroup   *groups;
    size_t              size;
    ATLAS_SURFACE       *atlas;
    int                 glyph_index;
    int                 group_index;
} GLYPHS_CACHE;*/

typedef struct FontInfo {
    const char  *font_name;
    int         font_size;
    const char  *cache;
    //size_t      cache_size;
} FONT_INFO;

int glyph_cache_build(const FONT_INFO *infos, int w, int h);
void glyph_cache_cleanup(void);
GLYPH* glyph_cache_find(Uint16 ch, int type);
int glyph_cache_get_font_size(int type);
int glyph_cache_get_font_lineskip(int type);
int glyph_cache_get_char_width(uint16_t ch, int type);
ATLAS_SURFACE *glyph_cache_atlas(void);
IMAGE_DATA glyph_cache_image(void);
