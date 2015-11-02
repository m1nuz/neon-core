#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "base/pjw.h"
#include "core/package.h"
#include <core/filesystem.h>

#include <minilzo.h>
#include <lz4.h>
#include <lz4hc.h>

#define HEAP_ALLOC(var, size)\
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

typedef struct FileInfo {
    char name[PACKED_FILENAME_SIZE];
    char path[PACKED_FILEPATH_SIZE];
    uint32_t name_hash;
    uint32_t ext_hash;
} FILE_INFO;

FILE_INFO       *files;
int             files_count;
int             files_size;

static int search_all_files(const char *path, int *count);

static int
on_list_file(const char *name, const char *path, bool isdir, void *user) {
    if(name[0] == '.')
        return 0;

    const char *ext = strrchr(name, '.');

    if (!isdir) {
        if (files) {
            strncpy(files[files_count].name, name, PACKED_FILENAME_SIZE);
            strncpy(files[files_count].path, path, PACKED_FILEPATH_SIZE);

            files[files_count].name_hash = pjw_hash(name);
            files[files_count].ext_hash = pjw_hash(ext);

            files_count++;
        }

        if (user)
            (*(int*)user)++;
    }
    else
        return search_all_files(path, user);

    return 0;
}

static int
search_all_files(const char *path, int *count) {
    if (!path)
        return -1;

    filesystem_list(path, on_list_file, count);

    return 0;
}

static void*
filedata_read(const char *filename, size_t *size) {
    if (!size)
        return NULL;

    FILE *fp = fopen(filename, "rb");

    if (!fp)
        return NULL;

    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    void* data = malloc(*size);
    fread(data, *size, 1, fp);

    fclose(fp);

    return data;
}


static int
package_write(const char *package_path, uint32_t flags) {
    FILE* fp = fopen(package_path, "wb");

    if (!fp)
        return -1;

    PACKAGE_HEADER header;
    memset(&header, 0, sizeof header);
    header.filenum = files_count;
    header.flags = flags;
    header.magic = PACKAGE_MAGIC;
    header.version = PACKAGE_VERSION;

    fwrite(&header, sizeof(header), 1, fp);

    PACKAGE_FILE *items = malloc(files_count * sizeof(PACKAGE_FILE));

    for (int i = 0; i < files_count; i++) {
        items[i].name_hash = files[i].name_hash;
        items[i].ext_hash = files[i].ext_hash;
        items[i].data_position = 0;
        items[i].data_size = 0;
        items[i].packed_data_size = 0;
        items[i].name_size = 0;
    }

    fwrite(items, sizeof(PACKAGE_FILE), files_count, fp);

    for (int i = 0; i < files_count; i++) {
        size_t sz = 0;
        void *data = filedata_read(files[i].path, &sz);

        if (data) {
            items[i].data_position = ftell(fp);
            items[i].data_size = sz;
            files_size += sz;

            if (flags & PACKAGE_FLAG_COMPRESS_LZO)
            {
                void *out_data = malloc(sz + sz / 16 + 64 + 3);

                lzo_uint out_sz = 0;

                lzo1x_1_compress(data, sz, out_data, &out_sz, wrkmem);

                items[i].packed_data_size = out_sz;

                fwrite(out_data, (size_t)out_sz, 1, fp);

                float compession = (float)(items[i].packed_data_size * 100) / items[i].data_size;

                printf("compress %s position %d size %d packed size %d %4.2f%%\n",
                       files[i].name, items[i].data_position, items[i].data_size, items[i].packed_data_size, compession);

                free(out_data);
            }
            else if (flags & PACKAGE_FLAG_COMPRESS_LZ4)
            {
                void *out_data = malloc(LZ4_COMPRESSBOUND(sz));

                int out_sz = LZ4_compress(data, out_data, sz);

                items[i].packed_data_size = out_sz;

                fwrite(out_data, (size_t)out_sz, 1, fp);

                float compession = (float)(items[i].packed_data_size * 100) / items[i].data_size;

                printf("compress %s position %d size %d packed size %d %4.2f%%\n",
                       files[i].name, items[i].data_position, items[i].data_size, items[i].packed_data_size, compession);

                free(out_data);
            }
            else if (flags & PACKAGE_FLAG_COMPRESS_LZ4_HC)
            {
                void *out_data = malloc(LZ4_COMPRESSBOUND(sz));

                int out_sz = LZ4_compressHC(data, out_data, sz);

                items[i].packed_data_size = out_sz;

                fwrite(out_data, (size_t)out_sz, 1, fp);

                float compession = (float)(items[i].packed_data_size * 100) / items[i].data_size;

                printf("compress %s position %d size %d packed size %d %4.2f%%\n",
                       files[i].name, items[i].data_position, items[i].data_size, items[i].packed_data_size, compession);

                free(out_data);
            }
            else
            {
                fwrite(data, sz, 1, fp);

                printf("write %s position %u size %u\n", files[i].name, items[i].data_position, items[i].data_size);
            }
        } else
            fprintf(stderr, "error: can't read file[%d] %s data\n", i, files[i].name);
    }

    int package_size = ftell(fp);

    fseek(fp, sizeof(header), SEEK_SET);
    fwrite(items, sizeof(PACKAGE_FILE), files_count, fp);

    fclose(fp);
    free(items);

    return package_size;
}

extern int
main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <dir> [flags]\n", "packer");
        exit(EXIT_SUCCESS);
    }

    uint32_t flags = 0;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            flags |= PACKAGE_FLAG_SAVE_NAMES;
            continue;
        }

        if (strcmp(argv[i], "-plzo") == 0) {
            flags |= PACKAGE_FLAG_COMPRESS_LZO;
            continue;
        }

        if (strcmp(argv[i], "-plz4") == 0) {
            flags |= PACKAGE_FLAG_COMPRESS_LZ4;
            continue;
        }

        if (strcmp(argv[i], "-plz4hc") == 0) {
            flags |= PACKAGE_FLAG_COMPRESS_LZ4_HC;
            continue;
        }
    }

    size_t path_size = strlen(argv[1]);
    char path[path_size];
    memset(path, 0, sizeof path);
    strcpy(path, argv[1]);

    const char *default_name = "resources.package";
    char fullpath[path_size + strlen(default_name) + 1];
    memset(fullpath, 0, sizeof fullpath);
    strcpy(fullpath, path);
    strcat(fullpath, default_name);

    // default
    if (flags == 0) {
        flags |= PACKAGE_FLAG_COMPRESS_LZ4;
    }

    int files_num = 0;
    search_all_files(argv[1], &files_num);

    files = malloc(files_num * sizeof(FILE_INFO));

    search_all_files(argv[1], &files_num);

    for (int i = 0; i < files_count; i++)
        printf("%#x %s\n", files[i].name_hash, files[i].path);

    int written = package_write(fullpath, flags);

    printf("%d files %s %d bytes %4.2f%%\n", files_num, flags ? "compressed" : "written", written, 100 - (float)(written * 100) / files_size);

    free(files);
    files = NULL;

    return 0;
}
