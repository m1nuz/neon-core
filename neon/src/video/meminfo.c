#include <stdbool.h>
#include <string.h>
#include <video/gl.h>

#if defined(GL_ATI_meminfo) || defined(GL_NVX_gpu_memory_info)

#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>

#include <GL/ati_meminfo.h>
#include <GL/nvx_gpu_memory_info.h>

#include "core/gpu_meminfo.h"
#include "core/logerr.h"

#ifdef GPU_MEMINFO

//#define USE_AMD_GPU_ASSOCIATION

static char gpu_meminfo[200] = {};
static bool ati_meminfo = false;
static bool nvx_gpu_memory_info = false;
static bool amd_gpu_association = false;
static bool meminfo_check = false;

static GLint vbo_mem[4] = {};
static GLint tex_mem[4] = {};
static GLint rtt_mem[4] = {};

static Uint64 meminfo_last = 0;
static double meminfo_accumulator = 0.0;

#ifdef USE_AMD_GPU_ASSOCIATION
#if (SDL_VIDEO_OPENGL_GLX == 1)
#include <GL/glx.h>

//
// https://www.opengl.org/registry/specs/AMD/glx_gpu_association.txt
//
#define GLX_GPU_VENDOR_AMD                 0x1F00
#define GLX_GPU_RENDERER_STRING_AMD        0x1F01
#define GLX_GPU_OPENGL_VERSION_STRING_AMD  0x1F02
#define GLX_GPU_FASTEST_TARGET_GPUS_AMD    0x21A2
#define GLX_GPU_RAM_AMD                    0x21A3
#define GLX_GPU_CLOCK_AMD                  0x21A4
#define GLX_GPU_NUM_PIPES_AMD              0x21A5
#define GLX_GPU_NUM_SIMD_AMD               0x21A6
#define GLX_GPU_NUM_RB_AMD                 0x21A7
#define GLX_GPU_NUM_SPI_AMD                0x21A8

unsigned int (*glXGetGPUIDsAMD)(unsigned int maxCount, unsigned int *ids);
int (*glXGetGPUInfoAMD)(unsigned int id, int property, GLenum dataType, unsigned int size, void *data);

static bool
amd_gpu_association_init(SDL_Window * window) {
    SDL_SysWMinfo syswm_info;
    SDL_VERSION(&syswm_info.version);

    if(!SDL_GetWindowWMInfo(window, &syswm_info))
        return false;

    Display *display = syswm_info.info.x11.display;

    if (display) {
        int screen = DefaultScreen(display);

        const char *extensions = glXQueryExtensionsString(display, screen);

        if (strstr(extensions, "GLX_AMD_gpu_association") != NULL) {
            glXGetGPUIDsAMD = glXGetProcAddress("glXGetGPUIDsAMD");
            glXGetGPUInfoAMD = glXGetProcAddress("glXGetGPUInfoAMD");

            return true;
        }

        return false;
    }

    return false;
}

GLuint
getTotalMemoryAMD() {
    if (glXGetGPUIDsAMD && glXGetGPUInfoAMD) {
        unsigned n = glXGetGPUIDsAMD(0, 0);
        int ids[n];

        size_t total_mem_mb = 0;

        glXGetGPUIDsAMD(n, ids);
        glXGetGPUInfoAMD(ids[0], GLX_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &total_mem_mb);

        return total_mem_mb;
    }

    return 0;
}

#endif

#if (SDL_VIDEO_OPENGL_WGL == 1)
#include <Wingdi.h>
//
// https://www.opengl.org/registry/specs/AMD/wgl_gpu_association.txt
//
#define WGL_GPU_VENDOR_AMD                 0x1F00
#define WGL_GPU_RENDERER_STRING_AMD        0x1F01
#define WGL_GPU_OPENGL_VERSION_STRING_AMD  0x1F02
#define WGL_GPU_FASTEST_TARGET_GPUS_AMD    0x21A2
#define WGL_GPU_RAM_AMD                    0x21A3
#define WGL_GPU_CLOCK_AMD                  0x21A4
#define WGL_GPU_NUM_PIPES_AMD              0x21A5
#define WGL_GPU_NUM_SIMD_AMD               0x21A6
#define WGL_GPU_NUM_RB_AMD                 0x21A7
#define WGL_GPU_NUM_SPI_AMD                0x21A8

UINT (*wglGetGPUIDsAMD)(UINT maxCount, UINT *ids);
INT (*wglGetGPUInfoAMD)(UINT id, INT property, GLenum dataType, UINT size, void *data);
const char *(*wglGetExtensionsStringARB)(HDC hdc);

static bool
amd_gpu_association_init(SDL_Window * window) {
    SDL_SysWMinfo syswm_info;
    SDL_VERSION(&syswm_info.version);

    HDC hdc = GetDC(syswm_info.info.win.window);

    wglGetExtensionsStringARB = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtensionsStringARB) {
        const char *extensions = wglGetExtensionsStringARB(hdc);

        if (strstr(extensions, "WGL_AMD_gpu_association") != NULL) {
            wglGetGPUIDsAMD = wglGetProcAddress("wglGetGPUIDsAMD");
            wglGetGPUInfoAMD = wglGetProcAddress("wglGetGPUInfoAMD");

            return true;
        }

        return false;
    }

    return false;
}

GLuint
getTotalMemoryAMD() {
    if (wglGetGPUIDsAMD && wglGetGPUInfoAMD) {
        UINT n = wglGetGPUIDsAMD(0, 0);
        INT ids[n];

        size_t total_mem_mb = 0;

        wglGetGPUIDsAMD(n, ids);
        wglGetGPUInfoAMD(ids[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &total_mem_mb);

        return total_mem_mb;
    }

    return 0;
}
#endif
#else
#endif // USE_AMD_GPU_ASSOCIATION

extern void
gpu_memory_info_init(SDL_Window * window) {
    if (!(ati_meminfo || nvx_gpu_memory_info)) {
        if (glIsExtensionSupported("GL_ATI_meminfo"))
            ati_meminfo = true;

        if (glIsExtensionSupported("GL_NVX_gpu_memory_info"))
            nvx_gpu_memory_info = true;
    }

    if (ati_meminfo) {
        glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, &vbo_mem[0]);
        glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &tex_mem[0]);
        glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, &rtt_mem[0]);
    }

#ifdef USE_AMD_GPU_ASSOCIATION
    amd_gpu_association = amd_gpu_association_init(window);
#endif // USE_AMD_GPU_ASSOCIATION

    if(!ati_meminfo && !amd_gpu_association && !nvx_gpu_memory_info)
        LOG_WARNING("AMD_gpu_association, GL_ATI_meminfo, GL_NVX_gpu_memory_info not avariable on this platform (%s).", SDL_GetPlatform());
}

extern const char *
gpu_memory_info() {
    if (meminfo_last > 0) {
        meminfo_accumulator += (double)(SDL_GetPerformanceCounter() - meminfo_last) / SDL_GetPerformanceFrequency();

        if (meminfo_accumulator > GPU_MEMINFO_UPDATE_TIME) {
            meminfo_check = true;
            meminfo_accumulator = 0.0;
        }
    }

    meminfo_last = SDL_GetPerformanceCounter();

    if (meminfo_check) {
        if (ati_meminfo) {
            GLint free_vbo_mem[4] = {};
            GLint free_tex_mem[4] = {};
            GLint free_rtt_mem[4] = {};

            glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, &free_vbo_mem[0]);
            glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &free_tex_mem[0]);
            glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, &free_rtt_mem[0]);

#ifdef USE_AMD_GPU_ASSOCIATION
            if (amd_gpu_association) {
                unsigned total_memory = getTotalMemoryAMD();
                snprintf(gpu_meminfo, sizeof(gpu_meminfo), "Total available GPU memory %u MB\nCurrent available GPU memory %d MB",
                         total_memory, free_tex_mem[0] / 1024);
            }
#else
            snprintf(gpu_meminfo, sizeof(gpu_meminfo), "VBOs memory %d MB available %d MB used\nTextures memory %d MB available"
                                                       " %d MB used\nRenderBuffer memory %d MB available  %d MB used",
                     free_vbo_mem[0] / 1024, (vbo_mem[0] - free_vbo_mem[0]) / 1024,
                    free_tex_mem[0] / 1024, (tex_mem[0] - free_tex_mem[0]) / 1024,
                    free_rtt_mem[0] / 1024, (rtt_mem[0] - free_rtt_mem[0]) / 1024);
#endif // USE_AMD_GPU_ASSOCIATION
        }
#ifdef USE_AMD_GPU_ASSOCIATION
        if (amd_gpu_association && !ati_meminfo) {
            unsigned total_memory = getTotalMemoryAMD();
            snprintf(gpu_meminfo, sizeof(gpu_meminfo), "Total available GPU memory %u MB\nCurrent available GPU memory %d MB",
                     total_memory, 0);
        }
#endif // USE_AMD_GPU_ASSOCIATION
        if (nvx_gpu_memory_info) {
            GLint nTotalMemoryInKB = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &nTotalMemoryInKB);

            GLint nCurAvailMemoryInKB = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &nCurAvailMemoryInKB);

            snprintf(gpu_meminfo, sizeof(gpu_meminfo), "Total available GPU memory %d MB\nCurrent available GPU memory %d MB",
                     nTotalMemoryInKB / 1024, nCurAvailMemoryInKB / 1024);
        }
    }

    return gpu_meminfo;
}

#endif // GPU_MEMINFO
#endif // defined(GL_ATI_meminfo) || defined(GL_NVX_gpu_memory_info)
