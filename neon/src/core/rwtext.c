#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "core/rwtext.h"

// feof equivalent for SDL_rwops
int rweof(SDL_RWops *ctx) {
    return SDL_RWsize(ctx) == SDL_RWtell(ctx);
}

// fgetc equivalent for SDL_rwops
int rwgetc(SDL_RWops *rw) {
    char c;

    return SDL_RWread(rw, &c, 1, 1) == 1 ? c : EOF;
}

// fgets equivalent for SDL_rwops
char *rwgets(char *buf, int count, SDL_RWops *rw) {
    Sint64 base = SDL_RWtell(rw);
    SDL_RWread(rw, buf, count, 1);

    char *end = strchr(buf, '\n');
    ptrdiff_t offs =  end - buf;

    /*if (offs != 0) {
        SDL_RWseek(rw, base + offs + 1, RW_SEEK_SET);
        *end = '\0';
    } else {
        SDL_RWseek(rw, base + 1, RW_SEEK_SET);
        *buf = '\0';
    }*/
    if (!end)
        end = buf;

    SDL_RWseek(rw, base + offs + 1, RW_SEEK_SET);
    *end = '\0';

    return buf;
}

/*char *rwgets(char *buf, int count, SDL_RWops *rw)
{
    int i;

    buf[count - 1] = '\0';

    for (i = 0; i < count - 1; i++)
    {
        if (SDL_RWread(rw, buf + i, 1, 1) != 1)
        {
            if (i == 0)
            {
                return NULL;
            }

            break;
        }

        if (buf[i] == '\n')
        {
            break;
        }
    }

    buf[i] = '\0';

    return buf;
}*/
