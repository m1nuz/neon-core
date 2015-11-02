#include <assert.h>
#include <memtrack.h>

#include "core/text.h"
#include "core/logerr.h"

extern char *
load_text(SDL_RWops *rw, size_t *size) {
    assert(rw != NULL);

    if (!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return NULL;
    }

    long int lenght = SDL_RWsize(rw);

    char *text = NULL;

    if(size && (lenght > 0)) {
        *size = lenght - 1;

        text = malloc(lenght);
        text[lenght - 1] = 0;

        SDL_RWread(rw, text, lenght - 1, 1);
        SDL_RWclose(rw);
    }

    return text;
}
