#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INTERNAL

#include "memtrack.h"

#ifdef MEMTRACK

struct memblock
{
    long           magic;
    struct memblock *next;
    struct memblock *prev;
    size_t         size;
    const char    *file;
    const char    *expr;
    int            line;
    unsigned int   index;   // Make it an even 16 byte length (total size=32 bytes in 32-bit mode).
};

#define MAGIC1 0x12345678
#define MAGIC2 0x87654321

struct memblock *memblockList = NULL;

static char     meminfo[100];

size_t          total_memory_used;
int             total_memory;
size_t          malloc_calls;
size_t          free_calls;
int             block_index;

FILE            *alloc_log;

//#define ALLOCATION_LOGGING

#ifdef ALLOCATION_LOGGING
//#define ALLOC_LOG stdout
#define ALLOC_LOG alloc_log
#endif

static void
trackmem_cleanup(void) {
#ifdef ALLOCATION_LOGGING
    if (alloc_log)
        fclose(alloc_log);
#endif
}

void
trackmem_init(void) {
#ifdef ALLOCATION_LOGGING
    alloc_log = fopen("allocations.log", "w");
    if (alloc_log == NULL) {
        perror("Init allocations log");
    }
#endif
    atexit(tracklist_allocations);
    atexit(trackmem_cleanup);
}

static void
trackdetails(struct memblock *mb) {
    printf("> %d bytes allocated with \"%s\" at %s:%d\n", (int)mb->size, mb->expr, mb->file, mb->line);
}

extern void *
trackmalloc(size_t size, const char *expr, const char *file, int line) {
    struct memblock *mb = malloc(size + sizeof(struct memblock));

    if (!mb) {
        // NOTE: May want to output some error message here!
        return NULL;
    }

    mb->magic = MAGIC1;
    mb->file = file;
    mb->line = line;
    mb->expr = expr;
    mb->size = size;
    mb->prev = NULL;
    mb->next = memblockList;
    mb->index = block_index++;

    if (memblockList)
        memblockList->prev = mb;

    memblockList = mb;
    total_memory_used += size;
    total_memory += size;
    malloc_calls++;

    void *p = (char*)mb + sizeof(struct memblock);

#ifdef ALLOCATION_LOGGING
    fprintf(ALLOC_LOG, "Allocated block[%u] %p size=%zu\n", mb->index, mb, mb->size);
#endif

    return p;
}

extern char *
trackstrdup(const char *src, const char *expr, const char *file, int line) {
    /*size_t sz = strlen(src) + 1;
    char *p = trackmalloc(sz, expr, file, line);
    memset(p, 0, sz);
    strcpy(p, src);

    return p;*/

    size_t len = strlen(src) + 1;
    void *copy = trackmalloc(len, expr, file, line);

    if (copy == NULL)
        return NULL;

    return (char *) memcpy(copy, src, len);
}

extern void *
trackrealloc(void *ptr, size_t size, const char *expr, const char *file, int line) {
    if (ptr) {
        struct memblock *mb  = (void*)((ptrdiff_t)ptr - sizeof(struct memblock));
        void *p = trackmalloc(size, expr, file, line);
        memcpy(p, ptr, mb->size);
        trackfree(ptr, expr, file, line);
        return ptr = p;
    }

    return trackmalloc(size, expr, file, line);
}

extern void
trackfree(void *ptr, const char *expr, const char *file, int line) {
    if (!ptr)
        return;
    else {
        struct memblock *mb  = (void*)((char*)ptr - sizeof(struct memblock));
        if (mb->magic != MAGIC1) {
            if (mb->magic == MAGIC2) {
                fprintf(stderr, "Attempt to free already freed memory:\n");
                trackdetails(mb);
            } else
                fprintf(stderr, "Invalid free of ptr (%p) with expression(\"%s\") at %s:%d\n", (void *)ptr, expr, file, line);

            return;
        }
        mb->magic = MAGIC2;
        if (mb ==  memblockList)
            memblockList = mb->next;

        // Unlink it.
        if (mb->next)
            mb->next->prev = mb->prev;

        if (mb->prev)
            mb->prev->next = mb->next;

        free_calls++;
        total_memory -= mb->size;

#ifdef MEMTRACK_FREE
        memset(ptr, 0xc0, mb->size);
#endif
#ifdef ALLOCATION_LOGGING
        fprintf(ALLOC_LOG, "Free block[%u] %p size=%zu\n", mb->index, mb, mb->size);
#endif
        free(mb);
    }
}


extern
void tracklist_allocations(void) {
    struct memblock *mb;

    puts("--- Allocations ---");
    if (!memblockList)
        puts(">>> Empty <<<");
    else
        for(mb = memblockList; mb; mb = mb->next)
            trackdetails(mb);
    puts("--- End ---");

    printf("Total memory used %zu B\n", total_memory_used);
    printf("Total memory left %d B\n", total_memory);
    printf("malloc calls %zu\n", malloc_calls);
    printf("free calls %zu\n", free_calls);
}

extern const char *
trackmeminfo(void) {
    snprintf(meminfo, sizeof(meminfo), "Total memory %d kB allocated\n", total_memory / 1024);

    return meminfo;
}

#endif // MEMTRACK
