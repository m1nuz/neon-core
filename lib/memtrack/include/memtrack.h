#pragma once

#define MEMTRACK
#define MEMTRACK_FREE
#define MEMTRACK_SHORTFILENAME

#ifdef MEMTRACK
#include <stddef.h>
#include <stdlib.h>

#ifdef MEMTRACK_SHORTFILENAME
#include <string.h>

#define __FILE_NANE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#else
#define __FILE_NANE __FILE__
#endif // MEMTRACK_SHORTFILENAME

#ifndef INTERNAL

#undef strdup

#define malloc(size) trackmalloc(size, #size, __FILE_NANE, __LINE__)
#define free(ptr) trackfree(ptr, #ptr, __FILE_NANE, __LINE__)
#define strdup(src) trackstrdup(src, #src,  __FILE_NANE, __LINE__)
#define realloc(block, size) trackrealloc(block, size, #size, __FILE_NANE, __LINE__)

#endif // INTERNAL

void *trackmalloc(size_t size, const char *expr, const char *file, int line);
char *trackstrdup(const char *src, const char *expr, const char *file, int line);
void *trackrealloc(void *ptr, size_t size, const char *expr, const char *file, int line);
void trackfree(void *ptr, const char *expr, const char *file, int line);
void tracklist_allocations(void);

void trackmem_init(void);

const char *trackmeminfo(void);

#define MEMTRACK_INIT() trackmem_init()

#else
#define MEMTRACK_INIT() (void)0
#endif

