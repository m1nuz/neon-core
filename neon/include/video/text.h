#pragma once

#include "video/sprite.h"
#include "video/vertices.h"

void submit_text(VIDEO_SPRITE_BATCH *batch, const char *text, const float2 position, const float4 color, int font_type);
void get_text_vertices(VERTICES_INFO *info, int w, int h, const char *text, size_t text_size, const float2 position, const float4 color, int font_type);
vec2 get_text_size(const char *text, size_t text_size, int font_type);
