#pragma once

#include <stdint.h>

typedef struct VideoStats {
    uint32_t    dips;
} VIDEO_STATS;

extern VIDEO_STATS video_stats;

void videostats_reset(void);
