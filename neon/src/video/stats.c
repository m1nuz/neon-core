#include <string.h>

#include "video/stats.h"

VIDEO_STATS video_stats;

void videostats_reset(void) {
    memset(&video_stats, 0, sizeof(video_stats));
}
