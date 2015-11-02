#pragma once

#include <stdbool.h>
#include <SDL2/SDL_events.h>
#include <core/common.h>

#ifndef C_LINKAGE
#ifdef __cplusplus
#define C_LINKAGE   "C"
#else
#define C_LINKAGE
#endif
#endif // no C_LINKAGE

#define NEON_API extern

#define TIMESTEP                0.005f

typedef struct AppState {
    int (*on_init)(void);
    void (*on_update)(float);
    void (*on_present)(int, int, float);
    void (*on_event)(const SDL_Event *event);
    void (*on_cleanup)(void);
} APP_STATE;

NEON_API void application_quit(void);
NEON_API void application_next_state(unsigned int state);
NEON_API void application_back_state(void);
NEON_API int application_exec(const char *title, APP_STATE *states, size_t states_n);
