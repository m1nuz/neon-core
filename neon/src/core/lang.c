#include "base/vector.h"
#include "core/lang.h"
#include "core/logerr.h"
#include "core/rwtext.h"
#include "core/asset.h"
#include <xxhash.h>
#include <stdint.h>
#include <memtrack.h>
#include <base/str_replace.h>
#include <base/pjw.h>

#define MIN_LANG_ITEMS 20

struct LangItem {
    char        *key;
    char        *name;
    uint32_t    key_hash;
    size_t      key_len;
    size_t      name_len;
};

VECTOR          lang_dict;
RESOURCE_INFO   *lang_files;
size_t          lang_files_count;
char            **lang_names;

static void
free_lang_item(void *ptr) {
    struct LangItem *li = ptr;
    free(li->key);
    free(li->name);
}

static void
cleanup_lang_dict(void) {
    if (lang_dict)
        vector_free(lang_dict);
    lang_dict = NULL;
}

extern int
load_lang(SDL_RWops *rw) {
    if (!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return -1;
    }

    cleanup_lang_dict();
    lang_dict = vector_new(MIN_LANG_ITEMS, sizeof(struct LangItem), free_lang_item);

    char buffer[1024];

    while (!rweof(rw)) {
        memset(buffer, 0, sizeof(buffer));
        rwgets(buffer, sizeof(buffer), rw);

        if (buffer[0] == '#')
            continue;

        char *ptr;
        char *key = strtok_r(buffer, " \r\n", &ptr);

        if (key) {
            if (key[0] == '#')
                continue;

            char *value = strtok_r(NULL, "\"", &ptr);

            //LOG("%s /%s/\n", key, value);

            if (value) {
                struct LangItem li;
                li.key = strdup(key);
                li.key_len = strlen(key);
                li.key_hash = XXH32(li.key, li.key_len, 0);
                li.name = str_replace("\\n", "\n", value);
                li.name_len = strlen(value);

                vector_append(lang_dict, &li);
            }
        }
    }

    // for debug
    /*for (size_t i = 0; i < vector_size(lang_dict); i++) {
        struct LangItem *li = vector_at(lang_dict, i);

        LOG("%s %s\n", li->key, li->name);
    }*/

    SDL_RWclose(rw);

    return 0;
}

const char *
lang_get(const char *id) {
    uint32_t hash = XXH32(id, strlen(id), 0);

    for (size_t i = 0; i < vector_size(lang_dict); i++) {
        struct LangItem *li = vector_at(lang_dict, i);

        if (li->key_hash == hash)
            return li->name;
    }

    LOG_WARNING("Lang key %s not found.\n", id);

    return NULL;
}

extern const char *
lang_get_name(size_t lang) {
    if (lang > lang_files_count)
        return NULL;

    return lang_names[lang];
}

/*extern int
lang_select(size_t lang) {
    if (lang > lang_files_count)
        return -1;

    load_lang(asset_request_hash(lang_files[lang].name_hash));

    return 0;
}*/

extern size_t
lang_select(const char *name) {
    for (size_t i = 0; i < lang_files_count; i++)
        if (strcmp(lang_names[i], name) == 0) {
            load_lang(asset_request_hash(lang_files[i].name_hash));
            return i;
        }

    return 0;
}

extern size_t
lang_count(void) {
    return lang_files_count;
}

static char *
lang_read_name(SDL_RWops *rw) {
    if (!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return NULL;
    }

    char *name = NULL;
    char buffer[1024];

    while (!rweof(rw)) {
        memset(buffer, 0, sizeof(buffer));
        rwgets(buffer, sizeof(buffer), rw);

        if (buffer[0] == '#')
            continue;

        char *ptr;
        char *key = strtok_r(buffer, " \r\n", &ptr);

        if (key) {
            if (key[0] == '#')
                continue;

            char *value = strtok_r(NULL, "\"", &ptr);

            if (value) {
                if (strcmp(key, "lang_name") == 0)
                    name = strdup(value);
            }
        }
    }

    SDL_RWclose(rw);

    return name;
}

extern void
lang_init(void) {
    size_t count = asset_query_filelist(".lang", NULL);

    if (count > 0) {
        lang_files = malloc(sizeof (RESOURCE_INFO) * count);
        memset(lang_files, 0, sizeof (RESOURCE_INFO) * count);

        lang_names = malloc(sizeof (char*) * count);

        asset_query_filelist(".lang", lang_files);

        lang_files_count = count;

        for (size_t i = 0; i < count; i++) {
            LOG("LANG %#x %#x %s (%s)\n", lang_files[i].name_hash, lang_files[i].ext_hash, lang_files[i].name, lang_files[i].path);

            lang_names[i] = lang_read_name(asset_request_hash(lang_files[i].name_hash));
        }
    }
}

extern void
lang_cleanup(void) {
    cleanup_lang_dict();

    for (size_t i = 0; i < lang_files_count; i++)
        free(lang_names[i]);
    free(lang_names);

    free(lang_files);
    lang_files = NULL;
    lang_files_count = 0;
}
