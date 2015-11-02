#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <video/gl.h>

#include "base/math_ext.h"
#include "base/intersection.h"
#include "video/buffer.h"

typedef struct VerticesData {
    void            *vertices;
    void            *indices;
    unsigned int    vertices_num;
    unsigned int    indices_num;
} VERTICES_DATA;

typedef struct VerticesDesc {
    uint32_t        primitive, vf, ef;
} VERTICES_DESC;

typedef struct VerticesInfo {
    VERTICES_DATA data;
    VERTICES_DESC desc;
} VERTICES_INFO;

typedef struct VideoVerticesInfo {
    VERTEX_ARRAY    array;
    VIDEO_BUFFER    vertices;
    VIDEO_BUFFER    elements;
    VERTICES_DESC   desc;

    struct VerticesObjectInfo {
        uint32_t    vb_offset;
        uint32_t    ib_offset;
        uint32_t    count;
        uint32_t    base_vertex;
        uint32_t    base_index;
        AABB        bound;
    }               *objects;
    uint32_t        objects_count;
} VIDEO_VERTICES_INFO;

VIDEO_VERTICES_INFO new_vertices_buffers(VERTICES_DATA *data, size_t count, const VERTICES_DESC *desc);
void cleanup_vertices_info(VIDEO_VERTICES_INFO *info);
void free_vertices_info(VIDEO_VERTICES_INFO *info);
VIDEO_VERTICES_INFO upload_vertices(const char *name);

typedef struct GenSphereInfo {
    int         rings;
    int         sectors;
    float       radius;
} GEN_SHPERE_INFO;

typedef struct GenTorusInfo {
    float innerRadius;
    float outerRadius;
    uint32_t numberSlices;
    uint32_t numberStacks;
} GEN_TORUS_INFO;

typedef struct GenRibbonInfo {
    int segments;
    float width;
} GEN_RIBBON_INFO;

typedef struct GenRectangularGridPlane {
    float horizontalExtend;
    float verticalExtend;
    uint32_t rows;
    uint32_t columns;
    bool triangleStrip;
} GEN_RECTANGULAR_GRID_PLANE;

VERTICES_INFO vertgen_cube(matrix4 transform);
VERTICES_INFO vertgen_sphere(const GEN_SHPERE_INFO *info, matrix4 transform);
VERTICES_INFO vertgen_quad_plane(matrix4 transform);
VERTICES_INFO vertgen_torus(const GEN_TORUS_INFO *info, matrix4 transform);
VERTICES_INFO vertgen_ribbon(const GEN_RIBBON_INFO *info, matrix4 transform);
VERTICES_INFO vertgen_rectangular_grid_plane(const GEN_RECTANGULAR_GRID_PLANE *info, matrix4 transform);
