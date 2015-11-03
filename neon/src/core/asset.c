#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <memtrack.h>

#include "core/asset.h"
#include <core/filesystem.h>
#include "base/pjw.h"
#include "core/logerr.h"
#include "core/package.h"
#include "core/common.h"

typedef struct Package {
    char            filepath[MAX_PACKAGE_FILEPATH];
    uint32_t        flags;
    uint32_t        version;
} PACKAGE;

typedef struct Resource {
    bool            packed;
    char            name[MAX_RESOURCE_NAME];
    char            path[MAX_RESOURCE_PATH];
    uint32_t        name_hash;
    uint32_t        ext_hash;
    // TODO: search data, check data copy
    //uint64_t        data_hash;
    uint32_t        size;
    uint32_t        position;
    uint32_t        packed_size;
    uint32_t        packed_name_size;
    void            *buffer;

    const PACKAGE   *package;
} RESOURCE;

const char *resource_exts[] = {
    RESOURCE_EXTS_LIST
    0
};

PACKAGE     packages[MAX_PACKAGES];
int         packages_count;

RESOURCE    *files;
int         files_num;
int         files_count;

static void
archive_resize(int num) {
    if (files_num <= num) {
        files_num += FILES_RESERV_NUM;
        files = realloc(files, files_num * sizeof(RESOURCE));
    }
}

static const PACKAGE*
add_package(const char *filepath, uint32_t flags, uint32_t version) {
    if (packages_count > MAX_PACKAGES) {
        LOG_CRITICAL("Can't add package %s. Overflow packages.\n", filepath);
        exit(EXIT_FAILURE);
    }

    PACKAGE *p = &packages[packages_count];

    strncpy(p->filepath, filepath, MAX_PACKAGE_FILEPATH);
    p->flags = flags;
    p->version = version;

    packages_count++;

    return p;
}

static int
add_resource(const char *name, const char *path, const char *ext) {
    assert(files_count < files_num);
    RESOURCE *res = &files[files_count];

    memset(res, 0, sizeof(RESOURCE));

    strncpy(res->name, name, MAX_RESOURCE_NAME);
    strncpy(res->path, path, MAX_RESOURCE_PATH);
    res->name_hash = pjw_hash(name);
    res->ext_hash = pjw_hash(ext);
    res->packed = false;    

    return files_count++;
}

static int
add_packed_resource(const PACKAGE *package, const PACKAGE_FILE *file) {
   RESOURCE *res = &files[files_count];

   memset(res, 0, sizeof(RESOURCE));

   res->name_hash = file->name_hash;
   res->ext_hash = file->ext_hash;
   res->packed = true;
   res->position = file->data_position;
   res->size = file->data_size;
   res->packed_size = file->packed_data_size;
   res->packed_name_size = file->name_size;
   res->package = package;

   return files_count++;
}

static int
package_open(const char *filepath) {
    SDL_RWops *rw = SDL_RWFromFile(filepath, "rb");

    checkif_return(!rw, -1, "Can't open package %s\n", filepath);

    LOG("Open package %s\n", filepath);

    PACKAGE_HEADER header = {0};

    SDL_RWread(rw, &header, sizeof(header), 1);

    /*if (header.magic != PACKAGE_MAGIC) {
        LOG_ERROR("Unknown pakage type %u\n", header.magic);
        SDL_RWclose(rw);
        return -1;
    }*/
    checkif_do(header.magic != PACKAGE_MAGIC, {
                   LOG_WARNING("Unknown pakage type %u\n", header.magic);
                   SDL_RWclose(rw);
                   return -1;
               });

    if (header.version < PACKAGE_VERSION) {
        LOG_ERROR("Unsupported pakage version %x\n", header.version);
        SDL_RWclose(rw);
        return -1;
    }

    archive_resize(header.filenum);

    const PACKAGE *pkg = add_package(filepath, header.flags, header.version);

    PACKAGE_FILE *items = malloc(sizeof(PACKAGE_FILE) * header.filenum);

    SDL_RWread(rw, items, sizeof(PACKAGE_FILE), header.filenum);

    for (unsigned i = 0; i < header.filenum; i++) {
        printf("%#x\n", items[i].name_hash);

        add_packed_resource(pkg, &items[i]);
    }

    free(items);

    SDL_RWclose(rw);

    return 0;
}

#include "lz4.h"
#include "lz4hc.h"

static void *
get_data(RESOURCE *res) {
    SDL_RWops *rw = SDL_RWFromFile(res->package->filepath, "rb");

    if (!rw) {
        LOG_ERROR("Can't open package %s\n", res->package->filepath);
        return NULL;
    }

    SDL_RWseek(rw, res->position, RW_SEEK_SET);

    // TODO: read name
    void *mem = NULL;

    if ((res->package->flags & PACKAGE_FLAG_COMPRESS_LZ4) ||
            (res->package->flags & PACKAGE_FLAG_COMPRESS_LZ4_HC)) {
        void *buffer = malloc(res->packed_size);
        mem = malloc(res->size);
        SDL_RWread(rw, buffer, res->packed_size, 1);

        LZ4_decompress_fast(buffer, mem, res->size);

        free(buffer);
    }

    SDL_RWclose(rw);

    return mem;
}

static bool
is_valid_resource_ext(const char *ext) {
    const char **p = &resource_exts[0];

    while (*p) {
        if (strcasecmp(ext, *p) == 0)
            return true;

        p++;
    }

    return false;
}

static int directory_search_all_files(const char *path);

static int
on_list_file(const char *name, const char *path, bool isdir, void *user) {
    UNUSED(user);

    if(name[0] == '.')
        return 0;

    const char *ext = strrchr(name, '.');

    if (!isdir) {
        if (!is_valid_resource_ext(ext))
            return 0;

        // TODO: search duplicate files
        // TODO: realloc files array

        add_resource(name, path, ext);
    }
    else
        return directory_search_all_files(path);

    return 0;
}

static int
directory_search_all_files(const char *path) {
    if (!path)
        return -1;

    archive_resize(FILES_RESERV_NUM - 1);

    filesystem_list(path, on_list_file, NULL);

    return 0;
}

static int
compare_hash (const void * a, const void * b) {
    return (int)((const RESOURCE*)a)->name_hash - (int)((const RESOURCE*)b)->name_hash;
}

extern int
asset_open(const char *path) {
    if (!path)
        return -1;

    int ret = -1;

    if (filesystem_is_directory(path))
        ret = directory_search_all_files(path);
    else {
        ret = package_open(path);
    }

#ifdef OPTIMIZE_FILES_SEARCH
    if (ret >= 0)
        qsort(files, files_count, sizeof(RESOURCE), compare_hash);
#endif

    for (int i = 0; i < files_count; i++)
        LOG("RESOURCE %#x %s (%s)\n", files[i].name_hash, files[i].name, files[i].path);
    LOG("RESOURCES founded %d\n", files_count);

    return ret;
}

extern void
asset_close(void) {
    free(files);
    files = NULL;
}

extern void
asset_process(void) {
    for (int i = 0; i < (files_count - 1); i++)
        if(files[i].buffer) {
            LOG("file %u free buffer %p\n", files[i].name_hash, files[i].buffer);
            free(files[i].buffer);
            files[i].buffer = NULL;
        }
}

/*const char *
get_filepath(const char *name) {
    // TODO: replace pjw hash
    uint32_t hash = pjw_hash(name);

    return get_filepath_from_hash(hash);
}*/

static const char *
get_filepath_from_hash(uint32_t hash) {
    assert(hash != 0);

#ifdef OPTIMIZE_FILES_SEARCH
    RESOURCE key;
    memset(&key, 0, sizeof(key));
    key.name_hash = hash;
    RESOURCE *p = bsearch(&key, files, files_count, sizeof(RESOURCE), compare_hash);

    if (p)
        return p->path;

    return NULL;
#else
    for (int i = 0; i < files_count; i++) {
        if (files[i].hash == hash)
            return files[i].path;
    }

    return NULL;
#endif
}

extern size_t
asset_query_filelist(const char *ext, RESOURCE_INFO *info) {
    uint32_t hash = pjw_hash(ext);

    size_t n = 0;
    for (int i = 0; i < files_count; i++)
        if (hash == files[i].ext_hash) {
            if (info) {
                strncpy(info[n].name, files[i].name, MAX_RESOURCE_NAME);
                strncpy(info[n].path, files[i].path, MAX_RESOURCE_PATH);
                info[n].name_hash = files[i].name_hash;
                info[n].ext_hash = files[i].ext_hash;
                info[n].packed = files[i].packed;
            }

            n++;
        }

    return n;
}

extern SDL_RWops*
asset_request_hash(uint32_t hash) {
    RESOURCE *p;
#ifdef OPTIMIZE_FILES_SEARCH
    RESOURCE key;
    memset(&key, 0, sizeof(key));
    key.name_hash = hash;
    p = bsearch(&key, files, files_count, sizeof(RESOURCE), compare_hash);
#else
    for (int i = 0; i < files_count; i++) {
        if (files[i].hash == hash) {
            p = &files[i];
            break;
        }
    }
#endif

    if (!p) {
        LOG_CRITICAL("Unknown file %#x\n", hash);
        exit(EXIT_FAILURE);
    }

    if (!p->packed)
        return SDL_RWFromFile(get_filepath_from_hash(hash), "rb");

    if (p->buffer)
        return SDL_RWFromMem(p->buffer, p->size);

    p->buffer = get_data(p);
    return SDL_RWFromMem(p->buffer, p->size);
}

extern SDL_RWops*
asset_request(const char *name) {
    assert(name != NULL);
    uint32_t hash = pjw_hash(name);

    return asset_request_hash(hash);
}
