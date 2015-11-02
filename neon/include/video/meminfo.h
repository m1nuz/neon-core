#pragma once

//#define GPU_MEMINFO

#ifdef GPU_MEMINFO

// in seconds
#define GPU_MEMINFO_UPDATE_TIME 10.0

void gpu_memory_info_init(SDL_Window *window);
const char * gpu_memory_info();

#else
#define gpu_memory_info_init(window) (void)0
#define gpu_memory_info() (void)0
#endif // GPU_MEMINFO
