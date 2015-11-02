#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <memtrack.h>

#include "core/common.h"
#include "core/configs.h"
#include "core/asset.h"
#include "core/rwtext.h"
#include "base/list.h"
#include "base/pjw.h"
#include "core/logerr.h"

#define MAX_VARIANT_STR_SIZE 255

enum VariantType {
    VARIANT_INT,
    VARIANT_FLOAT,
    VARIANT_STRING
};

typedef struct Variant {
    enum VariantType type;
    union {
        int     int_value;
        float   float_value;
        char    str_value[MAX_VARIANT_STR_SIZE];
    };
} VARIANT;

#define VARIANT_SET_INT(v, value) ((v).type = VARIANT_INT, (v).int_value = (value))
#define VARIANT_SET_FLOAT(v, value) ((v).type = VARIANT_FLOAT, (v).float_value = (value))
#define VARIANT_SET_STRING(v, value) ((v).type = VARIANT_STRING, strncpy((v).str_value, value, MAX_VARIANT_STR_SIZE))
#define VARIANT_GET_VALUE(v) ((v).type == VARIANT_INT ? (v).int_value : (v).type == VARIANT_FLOAT ? (v).float_value : (v).str_value)

#define MAX_CONFIGS_STR_KEY_SIZE 200

struct ConfigItem {
    char        key[MAX_CONFIGS_STR_KEY_SIZE];
    VARIANT     var;
    uint32_t    hash;
};

SLIST *configs;

static bool
isint(const char *str) {
    if (!str)
        return false;

    const char *s = str;

    int count = 0;

    while((*s != '\0') && (count < MAX_CONFIGS_STR_KEY_SIZE)) {
        if (!isdigit(*s) && (*s != '-'))
            return false;
        s++;
        count++;
    }

    return true;
}

static bool
isfloat(const char *str) {
    if (!str)
        return false;

    const char *s = str;

    int count = 0;

    while((*s != '\0') && (count < MAX_CONFIGS_STR_KEY_SIZE)) {
        if (!isdigit(*s) && (*s != '.') && (*s != '-'))
            return false;
        s++;
        count++;
    }

     return true;
}

static struct ConfigItem *
find_item(uint32_t hash) {
    SLIST_ITERATOR p = slist_head(configs, NULL);

    while(p != NULL) {
        struct ConfigItem *item = *(struct ConfigItem **)slist_get(p);

        if (item->hash == hash)
            return item;

        p = slist_next(p);
    }

    return NULL;
}

int
load_config(const char *filepath) {
    SDL_RWops *fp = SDL_RWFromFile(filepath, "r");

    if (!fp) {
        LOG_ERROR("Can't open file %s\n", filepath);
        return -1;
    }

    char buffer[256];

    while (!rweof(fp)) {
        memset(buffer, 0, sizeof(buffer));
        rwgets(buffer, sizeof(buffer), fp);

        if (buffer[0] == '#')
            continue;

        char *ptr;
        char *p = strtok_r(buffer, " \r\n", &ptr);

        if (p) {
            struct ConfigItem *item = malloc(sizeof(struct ConfigItem));

            strncpy(item->key, p, MAX_CONFIGS_STR_KEY_SIZE);
            item->hash = pjw_hash(p);

            p = strtok_r(NULL, " \r\n", &ptr);

            if (isint(p)) {
                VARIANT_SET_INT(item->var, atoi(p));
            } else if (isfloat(p)) {
                VARIANT_SET_FLOAT(item->var, atof(p));
            } else {
                VARIANT_SET_STRING(item->var, p);
            }

            // TODO: check dublicate
            slist_append(configs, &item);
        }
    }

    SDL_RWclose(fp);

    return 0;
}

void
configs_init(void) {
    configs = slist_alloc(sizeof(struct ConfigItem));
}

void
configs_cleanup(void) {
    if (!configs)
        return;

    SLIST_ITERATOR p = slist_head(configs, NULL);

    while(p != NULL) {
        struct ConfigItem *item = *(struct ConfigItem **)slist_get(p);

        free(item);

        p = slist_next(p);
    }

    slist_free(configs);
}

extern int
configs_int(const char *key) {
    uint32_t hash = pjw_hash(key);

    struct ConfigItem *item = find_item(hash);

    if (item)
        if (item->var.type == VARIANT_INT)
            return item->var.int_value;

    return 0;
}

extern float
configs_float(const char *key) {
    uint32_t hash = pjw_hash(key);

    struct ConfigItem *item = find_item(hash);

    if (item)
        if (item->var.type == VARIANT_FLOAT)
            return item->var.float_value;

    return 0.f;
}

extern const char*
configs_string(const char *key) {
    uint32_t hash = pjw_hash(key);

    struct ConfigItem *item = find_item(hash);

    if (item)
        if (item->var.type == VARIANT_STRING)
            return item->var.str_value;

    return NULL;
}
