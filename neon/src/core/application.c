#include <assert.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <video/gl.h>
#include <xxhash.h>
#include <memtrack.h>

#include "base/stack.h"
#include "core/common.h"
#include "base/math_ext.h"
#include "core/asset.h"
#include "core/configs.h"
#include "core/frame.h"

#include "core/logerr.h"
#include <core/application.h>
#include "core/video.h"
#include "video/resources_detail.h"
#include <core/audio.h>

static int              running = 1;
static STACK            *states_stack;
static APP_STATE       *allstates;
static size_t           states_num;

NEON_API void
application_next_state(unsigned int state) {
    if (state > states_num) {
        LOG_ERROR("State(%d) out of range", state);
        exit(EXIT_FAILURE);
    }

    push_stack(states_stack, &allstates[state]);

    ((APP_STATE*)top_stack(states_stack))->on_init();

    frame_flush();
}

NEON_API void
application_back_state(void) {
    ((APP_STATE*)pop_stack(states_stack))->on_cleanup();
    frame_flush();
}

static void
application_cleanup(void) {
    configs_cleanup();
    asset_close();

    audio_cleanup();
    video_cleanup();

    while (!is_stack_empty(states_stack))
        application_back_state();

    delete_stack(states_stack);
}

#ifdef OPENAL_BACKEND
#define SDL_INIT_FLAGS (SDL_INIT_VIDEO | SDL_INIT_TIMER)
#else
#define SDL_INIT_FLAGS (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)
#endif

NEON_API int
application_exec(const char *title, APP_STATE *states, size_t states_n) {
    allstates = states;
    states_num = states_n;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        LOG_ERROR("%s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    atexit(SDL_Quit);

    if (TTF_Init() < 0) {
        LOG_ERROR("%s\n", TTF_GetError());
        return EXIT_FAILURE;
    }

    atexit(TTF_Quit);

    if ((states_stack = new_stack(sizeof(APP_STATE), states_n + 1)) == NULL) {
        LOG_ERROR("%s\n", "Can\'t create game states stack");
        return EXIT_FAILURE;
    }

    LOG("%s launched...\n", title);
    LOG("Platform: %s\n", SDL_GetPlatform());

    video_init(title);
    audio_init();

    atexit(application_cleanup);

    application_next_state(0);

    if (is_stack_empty(states_stack)) {
        LOG_CRITICAL("%s\n", "No game states");
        exit(EXIT_FAILURE);
    }

    SDL_Event event;

    Uint64 current = 0;
    Uint64 last = 0;

    float accumulator = 0.0f;

    while(running) {
        frame_begin();

        while(SDL_PollEvent(&event)) {
            ((APP_STATE*)top_stack(states_stack))->on_event(&event);
        }

        asset_process();
        resources_process();

        last = current;
        current = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();

        float delta = (double)(current - last) / (double)freq;

        accumulator += CLAMP(delta, 0.f, 0.2f);

        while(accumulator >= TIMESTEP) {
            accumulator -= TIMESTEP;
            ((APP_STATE*)top_stack(states_stack))->on_update(TIMESTEP);
        }

        ((APP_STATE*)top_stack(states_stack))->on_present(screen.width, screen.height, accumulator / TIMESTEP);
        video_swap_buffers();

        frame_end();

        SDL_Delay(1);
    }

    return EXIT_SUCCESS;
}

NEON_API void
application_quit(void) {
    running = 0;
}
