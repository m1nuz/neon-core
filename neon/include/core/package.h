#pragma once

#include <stdint.h>

#define PACKAGE_MAGIC (('A' << 24) + ('G' << 16) + ('K' << 8) + 'P')
#define PACKAGE_VERSION 0x0102

#define PACKAGE_FLAG_SAVE_NAMES         0x0001
#define PACKAGE_FLAG_COMPRESS_LZO       0x0002
#define PACKAGE_FLAG_COMPRESS_LZ4       0x0004
#define PACKAGE_FLAG_COMPRESS_LZ4_HC    0x0080
#define PACKAGE_FLAG_HASH_PJW           0x0100
#define PACKAGE_FLAG_HASH_XXHASH        0x0200

#define PACKED_FILENAME_SIZE            260
#define PACKED_FILEPATH_SIZE            260

#pragma pack(push, package_header_align)
#pragma pack(1)

typedef struct PackageHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t filenum;
    uint32_t flags;
} PACKAGE_HEADER;

typedef struct PackageFile {
    uint32_t name_hash;
    uint32_t ext_hash;
    uint32_t data_size;
    uint32_t data_position;
    uint32_t packed_data_size;
    uint32_t name_size;
} PACKAGE_FILE;

#pragma pack(pop, package_header_align)
