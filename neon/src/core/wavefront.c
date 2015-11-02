#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <locale.h>
#include <memtrack.h>

#include "core/common.h"
#include "core/wavefront.h"
#include "core/logerr.h"
#include "core/rwtext.h"

//#define WAVEFRONT_DEBUG
#ifdef WAVEFRONT_DEBUG
#define wavefront_print(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define wavefront_print(fmt, ...)
#endif

typedef float   float2[2];
typedef float   float3[3];
typedef float   float4[4];
typedef int     int2[2];
typedef int     int3[3];
typedef int     int4[4];

struct Face
{
    int3 positions;
    int3 normals;
    int3 uvs;
};

struct Wavefront
{
    float3 *positions;
    float3 *normals;
    float2 *uvs;

    struct Face *faces;

    size_t num_positions;
    size_t num_normals;
    size_t num_uvs;

    size_t num_faces;

    int vf;
};

static void
update_wavefront_info(struct Wavefront *wavefront, SDL_RWops *fp) {
    assert(wavefront != NULL);
    assert(wavefront != NULL);

    wavefront->vf = -1;

    char buf[WAVEFRONT_READ_BUFFER_SIZE];

    while (!rweof(fp)) {
        memset(buf, 0, sizeof(buf));
        rwgets(buf, sizeof(buf), fp);

        char *ptr;
        char *p = NULL;
        int v[3] = {0}, n[3] = {0}, t[3] = {0};

        switch (buf[0]) {
        case 'v':

            switch (buf[1]) {
            case ' ':
                wavefront->num_positions++;
                break;
            case 'n':
                wavefront->num_normals++;
                break;
            case 't':
                wavefront->num_uvs++;
                break;
            }

            break;
        case 'f':
            p = strtok_r(buf + 1, " \n", &ptr);

            if (sscanf(p, "%d/%d/%d", &v[0], &t[0], &n[0]) == 3) {
                wavefront->vf = VF_V3T2N3;
            } else if (strstr(p, "//")) {
                wavefront->vf = VF_V3N3;
            } else if (sscanf(p, "%d/%d", &v[0], &t[0]) == 2) {
                wavefront->vf = VF_V3T2;
            } else {
                wavefront->vf = VF_V3;
            }

            wavefront->num_faces++;
            break;
        }
    }

    SDL_RWseek(fp, 0, RW_SEEK_SET);

#ifdef WAVEFRONT_DEBUG
    switch (wavefront->vf) {
    case VF_V3:
        puts("format V3");
        break;
    case VF_V3N3:
        puts("format V3N3");
        break;
    case VF_V3T2:
        puts("format V3T2");
        break;
    case VF_V3T2N3:
        puts("format V3T2N3");
        break;
    default:
        fprintf(stderr, "ERROR: unknown vertex format\n");
        exit(1);
        break;
    }

    printf("positions %d\n", wavefront->num_positions);
    printf("uv        %d\n", wavefront->num_uvs);
    printf("normals   %d\n", wavefront->num_normals);
    printf("faces     %d\n", wavefront->num_faces);
#else // NO WAVEFRONT_DEBUG
    if (wavefront->vf == -1) {
        LOG_CRITICAL("unknown vertex format %d\n", wavefront->vf);
        exit(EXIT_FAILURE);
    }
#endif // NO WAVEFRONT_DEBUG
}

static void
alloc_wavefront(struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    wavefront->positions = malloc(sizeof(float3) * wavefront->num_positions);
    wavefront->uvs = malloc(sizeof(float2) * wavefront->num_uvs);
    wavefront->normals = malloc(sizeof(float3) * wavefront->num_normals);
    wavefront->faces = malloc(sizeof(struct Face) * wavefront->num_faces);
}

static void
free_wavefront(struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    free(wavefront->positions);
    wavefront->positions = NULL;

    free(wavefront->uvs);
    wavefront->uvs = NULL;

    free(wavefront->normals);
    wavefront->normals = NULL;

    free(wavefront->faces);
    wavefront->faces = NULL;

    wavefront->num_positions = 0;
    wavefront->num_uvs = 0;
    wavefront->num_normals = 0;
    wavefront->num_faces = 0;
}

static v3_t *
build_v3(const struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    int count = (wavefront->num_faces - 1);
    v3_t *vertices = malloc(sizeof(v3_t) * count * 3);

    memset(vertices, 0, sizeof(v3_t) * count * 3);

    for (int i = 0; i < count; i++) {
        memcpy(vertices[i * 3 + 0].position, wavefront->positions[wavefront->faces[i].positions[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].position, wavefront->positions[wavefront->faces[i].positions[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].position, wavefront->positions[wavefront->faces[i].positions[2] - 1], sizeof(float3));
    }

    return vertices;
}


static v3n3_t *
build_v3n3(const struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    int count = (wavefront->num_faces - 1);
    v3n3_t *vertices = malloc(sizeof(v3n3_t) * count * 3);

    memset(vertices, 0, sizeof(v3n3_t) * count * 3);

    for (int i = 0; i < count; i++) {
        memcpy(vertices[i * 3 + 0].position, wavefront->positions[wavefront->faces[i].positions[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].position, wavefront->positions[wavefront->faces[i].positions[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].position, wavefront->positions[wavefront->faces[i].positions[2] - 1], sizeof(float3));

        memcpy(vertices[i * 3 + 0].normal, wavefront->positions[wavefront->faces[i].normals[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].normal, wavefront->positions[wavefront->faces[i].normals[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].normal, wavefront->positions[wavefront->faces[i].normals[2] - 1], sizeof(float3));
    }

    return vertices;
}

static v3t2_t *
build_v3t2(const struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    int count = (wavefront->num_faces - 1);
    v3t2_t *vertices = malloc(sizeof(v3t2_t) * count * 3);

    memset(vertices, 0, sizeof(v3t2_t) * count * 3);

    for (int i = 0; i < count; i++) {
        memcpy(vertices[i * 3 + 0].position, wavefront->positions[wavefront->faces[i].positions[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].position, wavefront->positions[wavefront->faces[i].positions[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].position, wavefront->positions[wavefront->faces[i].positions[2] - 1], sizeof(float3));

        memcpy(vertices[i * 3 + 0].texcoord, wavefront->positions[wavefront->faces[i].uvs[0] - 1], sizeof(float2));
        memcpy(vertices[i * 3 + 1].texcoord, wavefront->positions[wavefront->faces[i].uvs[1] - 1], sizeof(float2));
        memcpy(vertices[i * 3 + 2].texcoord, wavefront->positions[wavefront->faces[i].uvs[2] - 1], sizeof(float2));
    }

    return vertices;
}

static v3t2n3_t *
build_v3t2n3(const struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    int count = (wavefront->num_faces - 1);
    v3t2n3_t *vertices = malloc(sizeof(v3t2n3_t) * count * 3);

    memset(vertices, 0, sizeof(v3t2n3_t) * count * 3);

    for (int i = 0; i < count; i++) {
        memcpy(vertices[i * 3 + 0].position, wavefront->positions[wavefront->faces[i].positions[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].position, wavefront->positions[wavefront->faces[i].positions[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].position, wavefront->positions[wavefront->faces[i].positions[2] - 1], sizeof(float3));

        memcpy(vertices[i * 3 + 0].texcoord, wavefront->positions[wavefront->faces[i].uvs[0] - 1], sizeof(float2));
        memcpy(vertices[i * 3 + 1].texcoord, wavefront->positions[wavefront->faces[i].uvs[1] - 1], sizeof(float2));
        memcpy(vertices[i * 3 + 2].texcoord, wavefront->positions[wavefront->faces[i].uvs[2] - 1], sizeof(float2));

        memcpy(vertices[i * 3 + 0].normal, wavefront->positions[wavefront->faces[i].normals[0] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 1].normal, wavefront->positions[wavefront->faces[i].normals[1] - 1], sizeof(float3));
        memcpy(vertices[i * 3 + 2].normal, wavefront->positions[wavefront->faces[i].normals[2] - 1], sizeof(float3));
    }

    return vertices;
}

static unsigned short *
build_indices(struct Wavefront *wavefront) {
    assert(wavefront != NULL);

    int count = (wavefront->num_faces - 1);
    unsigned short *indices = malloc(sizeof(unsigned short) * count * 3);

    for (int i = 0; i < count * 3; i++) {
        indices[i] = i;
    }

    return indices;
}

extern int
load_wavefront(SDL_RWops *rw, void **_vertices, int *_vertices_num, void **_indices, int *_indices_num) {
    assert(rw != NULL);
    assert(_vertices != NULL);
    assert(_vertices_num != NULL);
    assert(_indices != NULL);
    assert(_indices_num != NULL);

    if(!rw) {
        LOG_ERROR("Can't open file %p\n", (void*)rw);
        return -1;
    }

    setlocale(LC_ALL, "C");

    struct Wavefront wavefront = {0};

    update_wavefront_info(&wavefront, rw);
    alloc_wavefront(&wavefront);

    char buf[WAVEFRONT_READ_BUFFER_SIZE];
    size_t positions_count = 0;
    size_t normals_count = 0;
    size_t uvs_count = 0;
    size_t faces_count = 0;

    while(!rweof(rw)) {
        memset(buf, 0, sizeof(buf));
        rwgets(buf, sizeof(buf), rw);

        char *ptr;
        char *p = NULL;
        int v[3] = {0}, n[3] = {0}, t[3] = {0};
        struct Face f = {{0}, {0}, {0}};

        switch (buf[0]) {
        case 'v':

            switch (buf[1]) {
            case ' ':
                p = strtok_r(buf, " \n", &ptr);

                p = strtok_r(0, " \n", &ptr);
                wavefront.positions[positions_count][0] = atof(p);

                p = strtok_r(0, " \n", &ptr);
                wavefront.positions[positions_count][1] = atof(p);

                p = strtok_r(0, " \n", &ptr);
                wavefront.positions[positions_count][2] = atof(p);

                wavefront_print("v %f %f %f\n", wavefront.positions[positions_count][0],
                        wavefront.positions[positions_count][1],
                        wavefront.positions[positions_count][2]);

                positions_count++;
                break;
            case 'n':
                p = strtok_r(buf, " \n", &ptr);

                p = strtok_r(0, " \n", &ptr);
                wavefront.normals[normals_count][0] = atof(p);

                p = strtok_r(0, " \n", &ptr);
                wavefront.normals[normals_count][1] = atof(p);

                p = strtok_r(0, " \n", &ptr);
                wavefront.normals[normals_count][2] = atof(p);

                wavefront_print("vn %f %f %f\n", wavefront.normals[normals_count][0],
                        wavefront.normals[normals_count][1],
                        wavefront.normals[normals_count][2]);

                normals_count++;
                break;
            case 't':
                p = strtok_r(buf, " \n", &ptr);

                p = strtok_r(0, " \n", &ptr);
                wavefront.uvs[uvs_count][0] = atof(p);

                p = strtok_r(0, " \n", &ptr);
                wavefront.uvs[uvs_count][1] = atof(p);

                wavefront_print("vt %f %f\n", wavefront.uvs[uvs_count][0],
                        wavefront.uvs[uvs_count][1]);

                uvs_count++;
                break;
            }

            break;
        case 'f':
            p = strtok_r(buf + 1, " \n", &ptr);

            if (sscanf(p, "%d/%d/%d", &v[0], &t[0], &n[0]) == 3) {
                wavefront_print("%d/%d/%d ", v[0], t[0], n[0]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d/%d/%d", &v[1], &t[1], &n[1]);
                wavefront_print("%d/%d/%d ", v[1], t[1], n[1]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d/%d/%d", &v[2], &t[2], &n[2]);
                wavefront_print("%d/%d/%d\n", v[2], t[2], n[2]);
            } else if (strstr(p, "//")) {
                sscanf(p, "%d//%d", &v[0], &n[0]);
                wavefront_print("%d//%d ", v[0], n[0]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d//%d", &v[1], &n[1]);
                wavefront_print("%d//%d ", v[1], n[1]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d//%d", &v[2], &n[2]);
                wavefront_print("%d//%d\n", v[2], n[2]);
            } else if (sscanf(p, "%d/%d", &v[0], &t[0]) == 2) {
                sscanf(p, "%d/%d", &v[0], &t[0]);
                wavefront_print("%d/%d ", v[0], t[0]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d/%d", &v[1], &t[1]);
                wavefront_print("%d/%d ", v[1], t[1]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d/%d", &v[2], &t[2]);
                wavefront_print("%d/%d\n", v[2], t[2]);
            } else {
                sscanf(p, "%d", &v[0]);
                wavefront_print("%d ", v[0]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d", &v[1]);
                wavefront_print("%d ", v[1]);

                p = strtok_r(0, " \n", &ptr);
                sscanf(p, "%d", &v[2]);
                wavefront_print("%d\n", v[2]);
            }

            memcpy(f.positions, v, sizeof(v));
            memcpy(f.uvs, t, sizeof(t));
            memcpy(f.normals, n, sizeof(n));

            memcpy(&wavefront.faces[faces_count], &f, sizeof(f));

            faces_count++;
            break;
        }
    }

    faces_count--; // we count +1

    switch (wavefront.vf) {
    case VF_V3:
        *_vertices = build_v3(&wavefront);
        *_indices = build_indices(&wavefront);
        *_vertices_num = faces_count * 3;
        *_indices_num = faces_count * 3;
        break;
    case VF_V3N3:
        *_vertices = build_v3n3(&wavefront);
        *_indices = build_indices(&wavefront);
        *_vertices_num = faces_count * 3;
        *_indices_num = faces_count * 3;
        break;
    case VF_V3T2:
        *_vertices = build_v3t2(&wavefront);
        *_indices = build_indices(&wavefront);
        *_vertices_num = faces_count * 3;
        *_indices_num = faces_count * 3;
        break;
    case VF_V3T2N3:
        *_vertices = build_v3t2n3(&wavefront);
        *_indices = build_indices(&wavefront);
        *_vertices_num = faces_count * 3;
        *_indices_num = faces_count * 3;
        break;
    default:
        LOG_ERROR("%s\n", "Unknown vertex format");
    }

    free_wavefront(&wavefront);

    SDL_RWclose(rw);

    setlocale(LC_ALL, "");

    return wavefront.vf;
}
