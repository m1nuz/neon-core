#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <video/gl.h>
#include <memtrack.h>

#ifndef GL_ES_VERSION_2_0
#include <GL/ext_texture_filter_anisotropic.h>
#endif

#include "core/video.h"
#include "core/common.h"
#include "core/logerr.h"
#include "video/meminfo.h"
#include "core/glyphs.h"
#include "video/resources.h"
#include <video/resources_detail.h>
#include "video/info.h"

SCREEN          screen = {.srgb_capable = false};
VIDEO_INFO      video = {.debug = true};

static bool
display_mode_chech(const VIDEO_MODE *modes, int n, const VIDEO_MODE *m) {
    for (int i = 0; i < n; i++)
        if (memcmp(&modes[i], m, sizeof(VIDEO_MODE)) == 0)
            return true;

    return false;
}

#ifndef GL_ES_VERSION_2_0
static void APIENTRY
video_debug_output(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) {
    UNUSED(source);
    UNUSED(length);
    UNUSED(userParam);

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        fputs("ERROR", stderr);
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        fputs("DEPRECATED_BEHAVIOR", stderr);
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        fputs("UNDEFINED_BEHAVIOR", stderr);
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        fputs("PORTABILITY", stderr);
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        fputs("PERFORMANCE", stderr);
        break;
    case GL_DEBUG_TYPE_OTHER:
        fputs("OTHER", stderr);
        break;
    }
    fputs("(", stderr);
    switch (severity){
    case GL_DEBUG_SEVERITY_LOW:
        fputs("LOW", stderr);
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        fputs("MEDIUM", stderr);
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        fputs("HIGH", stderr);
        break;
    }
    fprintf(stderr, "): [%d] %s\n", id, message);
}
#endif // NO GL_ES_VERSION_2_0

static void
get_display_modes(void) {
    int n = SDL_GetNumDisplayModes(0);

    if (n < 0) {
        LOG_WARNING("%s\n", "Can\'t enumarate display modes");
        return;
    }

    VIDEO_MODE modes[n];

    for (int i = 0; i < n; i++) {
        SDL_DisplayMode dm = {0};

        if (SDL_GetDisplayMode(0, i, &dm) < 0)
            LOG_ERROR("%s\n", SDL_GetError());

        modes[i].width = dm.w;
        modes[i].height = dm.h;
        modes[i].refresh_rate = dm.refresh_rate;

        LOG("video %dx%d %2.2f @%d\n", dm.w, dm.h, (float)dm.w / dm.h, dm.refresh_rate);
    }

    video.modes = malloc(sizeof(VIDEO_MODE) * n);

    // Remove same modes
    for(int i = 0; i < n; i++) {
        if (display_mode_chech(video.modes, video.modes_count, &modes[i]))
            continue;

        video.modes[video.modes_count] = modes[i];
        video.modes_count++;
    }
}

extern int
find_display_mode(int width, int height) {
    for(int i = 0; i < video.modes_count; i++) {
        if (video.modes[i].width == width &&
                video.modes[i].height == height)
            return i;
    }

    LOG_WARNING("%s\n", "Unsupported display mode.");

    return 0;
}

static void
video_init_formats(void) {
#ifdef GL_ES_VERSION_2_0
    video.i_rgb8 = GL_RGB;
    video.i_rgba8 = GL_RGBA;
    video.i_depth = GL_DEPTH_COMPONENT16;
    video.f_rgb = GL_RGB;
    video.f_rgba = GL_RGBA;
#else
    video.i_depth = GL_DEPTH_COMPONENT;

    if (screen.srgb_capable) {
        video.i_rgb8 = GL_SRGB8;
        video.i_rgba8 = GL_SRGB8_ALPHA8;
        //video.f_rgb = GL_SRGB;
        //video.f_rgba = GL_SRGB_ALPHA;
    } else {
        video.i_rgb8 = GL_RGB8;
        video.i_rgba8 = GL_RGBA8;
    }

    video.f_rgb = GL_RGB;
    video.f_rgba = GL_RGBA;
#endif // NO GL_ES_VERSION_2_0
}

static SDL_Window *window;
static SDL_GLContext context;

typedef void (*VOIDFUNC)(void);
static inline VOIDFUNC
fn_cast(void *ptr) {
    union {
        void *p;
        VOIDFUNC f;
    } p;

    p.p = ptr;

    return p.f;
}

extern void
video_init(const char *title) {
    get_display_modes();
    video_init_formats();

    Uint32 flags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL;

    if(screen.fullscreen != 0)
        flags |= SDL_WINDOW_FULLSCREEN;

    if((screen.width == -1) || (screen.height == -1))
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    /*if (screen.width == 0)
        screen.width = 640;
    if (screen.height == 0)
        screen.height = 480;*/

#ifdef OPENGL_CONTEXT_PROFILE_CORE
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (screen.msaa) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, screen.msaa);
    }

#if defined(VIDEO_SRGB_CAPABLE)
    if (screen.srgb_capable)
        SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
#endif
    const int major_version = OPENGL_MAJOR_VERSION;
    const int minor_version = OPENGL_MINOR_VERSION;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major_version);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor_version);

#ifdef OPENGL_CONTEXT_PROFILE_ES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

#ifdef OPENGL_CONTEXT_PROFILE_CORE
    int context_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
    if (video.debug == true)
        context_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, context_flags);
#endif

    if ((window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen.width, screen.height, flags)) == NULL) {
        LOG_ERROR("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if ((context = SDL_GL_CreateContext(window)) == NULL) {
        LOG_ERROR("%s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    screen.aspect = (float)screen.width / screen.height;

    if(screen.vsync)
        SDL_GL_SetSwapInterval(1);

    // BUG: if you dont call any function, lib not load
#ifdef GL_ES_VERSION_2_0
#undef glGetString
    GL_APICALL const GLubyte *GL_APIENTRY glGetString (GLenum name);
    printf("VERSION %s\n", (const char *)glGetString(GL_VERSION));
#define glGetString _glGetString
#endif // GL_ES_VERSION_2_0

    glLoadFunctions();
    gpu_memory_info_init(window);
    LOG("%s\n", video_get_opengl_info());

#ifndef GL_ES_VERSION_2_0
    glGetIntegerv(GL_MAX_SAMPLES, &video.max_supported_samples);
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &video.max_texture_anisotropy);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &video.max_transform_feedback_interleaved_components);
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, &video.max_transform_feedback_separate_components);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &video.max_combined_texture_image_units); // -1
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &video.max_uniform_block_size);
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &video.max_texture_buffer_size);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &video.max_color_attachments);
#endif // NO GL_ES_VERSION_2_0
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &video.max_uniform_components);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &video.max_vertex_attribs);
    video.max_vertex_attribs--;

    //int max_draw_buffers;
    //glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

    //bool ARB_internalformat_query = glIsExtensionSupported("GL_ARB_internalformat_query");

    video.max_uniform_components /= 4; // the actual number of component

#ifndef GL_ES_VERSION_2_0
    if (screen.msaa)
        glEnable(GL_MULTISAMPLE);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // TODO: SDL_SetWindowBrightness
#ifdef VIDEO_SRGB_CAPABLE
    /*if (screen.srgb_capable)
        glEnable(GL_FRAMEBUFFER_SRGB);*/
#endif
#endif // NO GL_ES_VERSION_2_0

#ifndef GL_ES_VERSION_2_0
    if (video.debug) {
        PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = NULL;

        bool ARB_debug_output = video_is_extension_supported("GL_ARB_debug_output");
        bool KHR_debug = video_is_extension_supported("GL_KHR_debug");
        GLenum debug_output_synchronous = 0;

        if (ARB_debug_output) {
            glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)fn_cast(nativeGetProcAddress("glDebugMessageCallbackARB"));
            debug_output_synchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB;
        } else if (KHR_debug) {
            glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)fn_cast(nativeGetProcAddress("glDebugMessageCallback"));
            debug_output_synchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS;
        }

        if (glDebugMessageCallback) {
            glDebugMessageCallback(video_debug_output, NULL);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(debug_output_synchronous);
        }

        //TODO: GL_AMD_debug_output
    }
#endif // NO GL_ES_VERSION_2_0

    video_warp_mouse(screen.width / 2, screen.height / 2);    

    resources_init();
}

extern void
video_cleanup(void) {
    glyph_cache_cleanup();
    resources_cleanup();

    free(video.modes);
    video.modes = NULL;

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
}

extern void
video_swap_buffers(void) {
    SDL_GL_SwapWindow(window);
}

extern void
video_warp_mouse(int x, int y) {
    SDL_WarpMouseInWindow(window, x, y);
}

extern void
video_show_cursor(bool show) {
    SDL_ShowCursor(show);
}

extern void
video_apply_changes(void) {
    SDL_SetWindowSize(window, screen.width, screen.height);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_GL_SetSwapInterval(screen.vsync);
    screen.aspect = (float)screen.width / (float)screen.height;

    if (screen.fullscreen)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    /*if (screen.srgb_capable)
        glEnable(GL_FRAMEBUFFER_SRGB);
    else
        glDisable(GL_FRAMEBUFFER_SRGB);*/

    SDL_ShowWindow(window);
}

void
video_set_title(const char *title) {
    SDL_SetWindowTitle(window, title);
}
