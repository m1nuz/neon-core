#include <SDL2/SDL_ttf.h>
#include <locale.h>
#include <memtrack.h>

#include "core/common.h"
#include "core/glyphs.h"
#include "core/atlas.h"
#include "core/logerr.h"
#include "core/asset.h"

#include <uchar.h>

#define USE_STD_QSORTBSEARCH

GLYPH               *glyphs;
GLYPH_GROUP         *glyph_group;
size_t              glyph_cache_size;

ATLAS_SURFACE       *glyphs_atlas;

static int          glyph_index;
static int          group_index;

static int
compare_glyph(const void * restrict a, const void * restrict b) {
    if (((const GLYPH*)a)->ch == ((const GLYPH*)b)->ch)
        return ((const GLYPH*)a)->type - ((const GLYPH*)b)->type;

    return ((const GLYPH*)a)->ch - ((const GLYPH*)b)->ch;
}

//mbstate_t state;

static void
append_glyphs(const FONT_INFO *info) {
    SDL_Color White = {255, 255, 255, 255};
    //SDL_Color Black = {0, 0, 0, 255};

    TTF_Font *font = TTF_OpenFontRW(asset_request(info->font_name), SDL_TRUE, info->font_size);

    if (!font) {
        LOG_CRITICAL("%s %s\n", "Can't open font", info->font_name);
        exit(EXIT_FAILURE);
    }

    size_t sz = utf8_strlen(info->cache);
    wchar_t str[sz];

    size_t count = utf8_towcs(info->cache, str, sizeof(str));

    for (size_t i = 0; i < count; i++) {
        wchar_t ch[2] = {str[i], 0};

        int minx, maxx, miny, maxy, advance;

        if (TTF_GlyphMetrics(font, ch[0], &minx, &maxx, &miny, &maxy, &advance) != -1) {
            SDL_Surface *glyph = TTF_RenderUNICODE_Blended(font, (Uint16*)&ch[0], White);
            SDL_Surface *rgba_glyph = SDL_ConvertSurfaceFormat(glyph, SDL_PIXELFORMAT_RGBA8888, 0);

            glyphs[glyph_index].rc = atlas_add(glyphs_atlas, rgba_glyph);
            glyphs[glyph_index].ch = ch[0];
            glyphs[glyph_index].advance = advance;
            glyphs[glyph_index].type = group_index;
            glyph_index++;

            SDL_FreeSurface(glyph);
            SDL_FreeSurface(rgba_glyph);
        } else
            LOG_WARNING("Symbol %h not found.\n", ch[0]);
    }

    /*size_t count = strlen(info->cache);

    // big white
    for (size_t i = 0; i < count; i++) {
        int minx, maxx, miny, maxy, advance;

        TTF_GlyphMetrics(font, info->cache[i], &minx, &maxx, &miny, &maxy, &advance);
        SDL_Surface *glyph = TTF_RenderUNICODE_Blended(font, info->cache[i], White);
        SDL_Surface *rgba_glyph = SDL_ConvertSurfaceFormat(glyph, SDL_PIXELFORMAT_RGBA8888, 0);

        glyphs[glyph_index].rc = atlas_add(glyphs_atlas, rgba_glyph);
        glyphs[glyph_index].ch = info->cache[i];
        glyphs[glyph_index].advance = advance;
        glyphs[glyph_index].type = group_index;
        glyph_index++;

        SDL_FreeSurface(glyph);
        SDL_FreeSurface(rgba_glyph);
    }*/

    glyph_group[group_index].type = group_index;
    glyph_group[group_index].size = TTF_FontHeight(font);
    glyph_group[group_index].lineskip = TTF_FontLineSkip(font);

    //printf("FONT SIZE %d\n", TTF_FontHeight(font));

    group_index++;

    TTF_CloseFont(font);
}

#include <uchar.h>

extern int
glyph_cache_build(const FONT_INFO *infos, int w, int h) {
    glyphs_atlas = atlas_alloc(w, h, 1);

    if (!glyphs_atlas) {
        LOG_CRITICAL("%s\n", "Can't create font atlas");
        exit(EXIT_FAILURE);
    }

    glyph_index = 0;
    group_index = 0;

    if (infos == NULL) {
        LOG_CRITICAL("%s\n", "No font infos");
        exit(EXIT_FAILURE);
    }

    const FONT_INFO *p = infos;

    int group_count = 0;

    while (p->cache) {
        glyph_cache_size += utf8_strlen(p->cache);//strlen(p->cache);
        group_count++;
        p++;
    }

    glyphs = malloc(sizeof(GLYPH) * glyph_cache_size);
    if (!glyphs) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    glyph_group = malloc(sizeof(GLYPH_GROUP) * group_count);
    if (!glyph_group) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    p = infos;
    while (p->cache) {
        append_glyphs(p);
        p++;
    }    

#ifdef USE_STD_QSORTBSEARCH
    qsort(glyphs, glyph_cache_size, sizeof(GLYPH), compare_glyph);
#endif

    return 0;
}

extern void
glyph_cache_cleanup(void) {
    free(glyphs);
    glyphs = NULL;

    free(glyph_group);
    glyph_group = NULL;

    atlas_free(glyphs_atlas);
    glyphs_atlas = NULL;
}

extern GLYPH*
glyph_cache_find(Uint16 ch, int type) {
#ifdef USE_STD_QSORTBSEARCH
    GLYPH glyph = {.ch = ch, .type = type};

    return bsearch(&glyph, glyphs, glyph_cache_size, sizeof(GLYPH), compare_glyph);
#else
    for (unsigned i = 0; i < glyph_cache_size; i++) {
        if((ch == glyph_cache[i].ch) && (type == glyph_cache[i].type))
            return &glyph_cache[i];
    }

    return NULL;
#endif
}

extern int
glyph_cache_get_font_size(int type) {
    for(int i = 0; i < group_index; i++)
        if (type == glyph_group[i].type)
            return glyph_group[i].size;

    return 0;
}

extern int
glyph_cache_get_font_lineskip(int type) {
    for(int i = 0; i < group_index; i++)
        if (type == glyph_group[i].type)
            return glyph_group[i].lineskip;

    return 0;
}

extern int
glyph_cache_get_char_width(uint16_t ch, int type) {
    GLYPH *g = glyph_cache_find(ch, type);
    if (g)
        return g->advance;

    return 0;
}

extern ATLAS_SURFACE *
glyph_cache_atlas(void) {
    return glyphs_atlas;
}

extern IMAGE_DATA
glyph_cache_image(void) {
    return atlas_get_image(glyphs_atlas);
}
