#include <SDL2/SDL_timer.h>

#include "core/frame.h"

static Uint64   frame_start_time;
static Uint64   frame_end_time;
static double   frame_time_value;
static double   frame_accumulator;
static int      frame_counter;
static int      frame_rate;
static int      max_frame_rate;
static int      min_frame_rate;
static char     frame_info_str[80];

extern void
frame_flush(void) {
    frame_accumulator = 0;
    frame_counter = 0;
    frame_rate = 0;
    max_frame_rate = 0;
    min_frame_rate = 0;
    frame_time_value = 0;
}

extern void
frame_begin(void) {
    frame_start_time = SDL_GetPerformanceCounter();
}

extern void
frame_end(void) {
    frame_end_time = SDL_GetPerformanceCounter();

    frame_time_value = (double)(frame_end_time - frame_start_time) / SDL_GetPerformanceFrequency();
    frame_accumulator += frame_time_value;
    frame_counter++;

    if (frame_accumulator > 1.0) {
        frame_rate = round((double)frame_counter / frame_accumulator);

        if (min_frame_rate == 0)
            min_frame_rate = frame_rate;

        max_frame_rate = (frame_rate > max_frame_rate) ? frame_rate : max_frame_rate;
        min_frame_rate = (frame_rate < min_frame_rate) ? frame_rate : min_frame_rate;

        frame_accumulator = 0;
        frame_counter = 0;

        snprintf(frame_info_str, sizeof(frame_info_str), "fps %d\nmin %d\nmax %d\navg %d\ntm %lfms", frame_rate, min_frame_rate,
                max_frame_rate, (min_frame_rate + max_frame_rate) / 2, frame_time_value * 1000.0);
    }
}

extern double
frame_time(void) {
    return frame_time_value;
}

extern const char *
frame_info(void) {
    return frame_info_str;
}
