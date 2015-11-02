#include <assert.h>
#include <memtrack.h>

#include "core/logerr.h"
#include "video/vertex.h"
#include "video/buffer.h"
#include "video/vertices.h"

extern VIDEO_VERTICES_INFO
new_vertices_buffers(VERTICES_DATA *data, size_t count, const VERTICES_DESC *desc) {
    assert(count != 0);

    VIDEO_VERTICES_INFO info;
    memset(&info, 0, sizeof(info));

    info.array = new_vertex_array();
    bind_vertex_arrays(&info.array);

    if (!data) {
        LOG("NEW VIDEO VERICES %d [%d %d]\n", info.array, info.vertices, info.elements);
        return info;
    }

    info.objects = malloc(sizeof(struct VerticesObjectInfo) * count);
    info.objects_count = count;
    info.desc = *desc;

    uint32_t vertices_data_size = 0;
    uint32_t indices_data_size = 0;

    struct {
        size_t vb_size;
        size_t ib_size;
    } buffers_info[count];
    uint32_t base_vertex = 0;
    uint32_t base_index = 0;

    // count vertex buffers size and other draw stuff
    for (size_t i = 0; i < count; i++) {
        const VERTICES_DATA *vd = &data[i];

        info.objects[i].count = vd->indices_num != 0 ? vd->indices_num : vd->vertices_num;

        switch (desc->vf) {
        case VF_V3:
            vertices_data_size += vd->vertices_num * sizeof(float3);
            buffers_info[i].vb_size = vd->vertices_num * sizeof(float3);

            make_aabb_from_vertices((const v3_t*)vd->vertices, vd->vertices_num, info.objects[i].bound);
            break;
        case VF_V3N3:
            vertices_data_size += vd->vertices_num * sizeof(v3n3_t);
            buffers_info[i].vb_size = vd->vertices_num * sizeof(v3n3_t);

            make_aabb_from_vertices((const v3n3_t*)vd->vertices, vd->vertices_num, info.objects[i].bound);
            break;
        case VF_V3T2:
            vertices_data_size += vd->vertices_num * sizeof(v3t2_t);
            buffers_info[i].vb_size = vd->vertices_num * sizeof(v3t2_t);

            make_aabb_from_vertices((const v3t2_t*)vd->vertices, vd->vertices_num, info.objects[i].bound);
            break;
        case VF_V3T2N3:
            vertices_data_size += vd->vertices_num * sizeof(v3t2n3_t);
            buffers_info[i].vb_size = vd->vertices_num * sizeof(v3t2n3_t);

            make_aabb_from_vertices((const v3t2n3_t *)vd->vertices, vd->vertices_num, info.objects[i].bound);
            break;
        default:
            LOG_ERROR("%s\n", "Unknown vertex format");
        }

        info.objects[i].base_vertex = base_vertex;
        base_vertex += vd->vertices_num;
    }

    // count index buffers size
    for (size_t i = 0; i < count; i++) {
        const VERTICES_DATA *vd = &data[i];

        switch (desc->ef) {
        case IF_UI16:
            indices_data_size += vd->indices_num * sizeof(uint16_t);
            buffers_info[i].ib_size = vd->indices_num * sizeof(uint16_t);
            break;
        case IF_UI32:
            indices_data_size += vd->indices_num * sizeof(uint32_t);
            buffers_info[i].ib_size = vd->indices_num * sizeof(uint32_t);
            break;
        default:
            LOG_ERROR("%s\n", "Unknown index format");
        }

        info.objects[i].base_index = base_index;
        base_index += vd->indices_num;
    }

    // alloc buffers
    void *vertex_data = malloc(vertices_data_size);
    void *index_data = malloc(indices_data_size);
    ptrdiff_t vb_offset = 0;
    ptrdiff_t ib_offset = 0;

    // copy buffers to one buffer
    for (size_t i = 0; i < count; i++) {
        info.objects[i].vb_offset = vb_offset;
        memcpy((char*)vertex_data + vb_offset, data[i].vertices, buffers_info[i].vb_size);
        vb_offset += buffers_info[i].vb_size;

        info.objects[i].ib_offset = ib_offset;
        memcpy((char*)index_data + ib_offset, data[i].indices, buffers_info[i].ib_size);
        ib_offset += buffers_info[i].ib_size;
    }

    info.vertices = new_vertex_buffer(vertex_data, vertices_data_size, GL_STATIC_DRAW);

    // transfer to video memory
    switch (desc->vf) {
    case VF_V3:
        vertex_array_buffer(&info.array, &info.vertices, 0, 0);
        vertex_array_format(&info.array, POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 0);
        break;
    case VF_V3N3:
        vertex_array_buffer(&info.array, &info.vertices, 0, sizeof(v3n3_t));
        vertex_array_format(&info.array, POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3n3_t, position));
        vertex_array_format(&info.array, NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3n3_t, normal));
        break;
    case VF_V3T2:
        vertex_array_buffer(&info.array, &info.vertices, 0, sizeof(v3t2_t));
        vertex_array_format(&info.array, POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3t2_t, position));
        vertex_array_format(&info.array, TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, offsetof(v3t2_t, texcoord));
        break;
    case VF_V3T2N3:
        vertex_array_buffer(&info.array, &info.vertices, 0, sizeof(v3t2n3_t));
        vertex_array_format(&info.array, POSITION_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3t2n3_t, position));
        vertex_array_format(&info.array, TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, offsetof(v3t2n3_t, texcoord));
        vertex_array_format(&info.array, NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, offsetof(v3t2n3_t, normal));
        break;
    }

    info.elements = new_index_buffer(index_data, indices_data_size, GL_STATIC_DRAW);

    unbind_vertex_array(&info.array);

    free(vertex_data);
    free(index_data);

    // NOTE: alloc and free in different places
    for (size_t i = 0; i < count; i++) {
        free(data[i].vertices);
        data[i].vertices = NULL;
        free(data[i].indices);
        data[i].indices = NULL;
    }

    LOG("NEW VIDEO VERICES %d [%d %d]\n", info.array, info.vertices, info.elements);

    return info;
}

extern void
cleanup_vertices_info(VIDEO_VERTICES_INFO *info) {
    free(info->objects);
    info->objects = NULL;
}

extern void
free_vertices_info(VIDEO_VERTICES_INFO *info) {
    cleanup_vertices_info(info);

    free_video_buffer(&info->vertices);
    free_video_buffer(&info->elements);

    free_vertex_array(&info->array);
}

/*
 * vertices generation
 */

#define CUBE_VERTICES_NUM       24
#define CUBE_INDICES_NUM        36
#define QUAD_VERTICES_NUM       4
#define QUAD_INDICES_NUM        6

v3t2n3_t cube_vertices_v3t2n3[CUBE_VERTICES_NUM] = {
    // front
    {{-1.f, 1.f, 1.f}, {0.f, 1.f}, { 0.f, 0.f, 1.f}},
    {{ 1.f, 1.f, 1.f}, {1.f, 1.f}, { 0.f, 0.f, 1.f}},
    {{ 1.f,-1.f, 1.f}, {1.f, 0.f}, { 0.f, 0.f, 1.f}},
    {{-1.f,-1.f, 1.f}, {0.f, 0.f}, { 0.f, 0.f, 1.f}},
    // back
    {{ 1.f, 1.f,-1.f}, {0.f, 1.f}, { 0.f, 0.f,-1.f}},
    {{-1.f, 1.f,-1.f}, {1.f, 1.f}, { 0.f, 0.f,-1.f}},
    {{-1.f,-1.f,-1.f}, {1.f, 0.f}, { 0.f, 0.f,-1.f}},
    {{ 1.f,-1.f,-1.f}, {0.f, 0.f}, { 0.f, 0.f,-1.f}},
    // top
    {{-1.f, 1.f,-1.f}, {0.f, 1.f}, { 0.f, 1.f, 0.f}},
    {{ 1.f, 1.f,-1.f}, {1.f, 1.f}, { 0.f, 1.f, 0.f}},
    {{ 1.f, 1.f, 1.f}, {1.f, 0.f}, { 0.f, 1.f, 0.f}},
    {{-1.f, 1.f, 1.f}, {0.f, 0.f}, { 0.f, 1.f, 0.f}},
    // bottom
    {{ 1.f,-1.f,-1.f}, {0.f, 1.f}, { 0.f,-1.f, 0.f}},
    {{-1.f,-1.f,-1.f}, {1.f, 1.f}, { 0.f,-1.f, 0.f}},
    {{-1.f,-1.f, 1.f}, {1.f, 0.f}, { 0.f,-1.f, 0.f}},
    {{ 1.f,-1.f, 1.f}, {0.f, 0.f}, { 0.f,-1.f, 0.f}},
    // left
    {{-1.f, 1.f,-1.f}, {0.f, 1.f}, {-1.f, 0.f, 0.f}},
    {{-1.f, 1.f, 1.f}, {1.f, 1.f}, {-1.f, 0.f, 0.f}},
    {{-1.f,-1.f, 1.f}, {1.f, 0.f}, {-1.f, 0.f, 0.f}},
    {{-1.f,-1.f,-1.f}, {0.f, 0.f}, {-1.f, 0.f, 0.f}},
    // right
    {{ 1.f, 1.f, 1.f}, {0.f, 1.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f, 1.f,-1.f}, {1.f, 1.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f,-1.f,-1.f}, {1.f, 0.f}, { 1.f, 0.f, 0.f}},
    {{ 1.f,-1.f, 1.f}, {0.f, 0.f}, { 1.f, 0.f, 0.f}}
};

unsigned short cube_indices_v3t2n3[CUBE_INDICES_NUM] = {
    0, 3, 1,  1, 3, 2,  // front
    4, 7, 5,  5, 7, 6,  // back
    8,11, 9,  9,11,10,  // top
    12,15,13, 13,15,14, // bottom
    16,19,17, 17,19,18, // left
    20,23,21, 21,23,22  // right
};

float cube_points[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
};

v3t2n3_t quad_vertices[4] = {
    {{-1.f,  1.f, 0.f}, {0.f, 1.f}, {0.f, 0.f, 1.f}},
    {{ 1.f,  1.f, 0.f}, {1.f, 1.f}, {0.f, 0.f, 1.f}},
    {{ 1.f, -1.f, 0.f}, {1.f, 0.f}, {0.f, 0.f, 1.f}},
    {{-1.f, -1.f, 0.f}, {0.f, 0.f}, {0.f, 0.f, 1.f}},
};

uint16_t quad_indices[6] = {
    0, 3, 1,  1, 3, 2
};

extern VERTICES_INFO
vertgen_cube(matrix4 transform) {
    v3t2n3_t *vs = malloc(sizeof(cube_vertices_v3t2n3));
    uint16_t *es = malloc(sizeof(cube_vertices_v3t2n3));

    memcpy(vs, cube_vertices_v3t2n3, sizeof(cube_vertices_v3t2n3));
    memcpy(es, cube_indices_v3t2n3, sizeof(cube_indices_v3t2n3));

    for (size_t i = 0; i < CUBE_VERTICES_NUM; i++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, cube_vertices_v3t2n3[i].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vs[i].position, v_out);

        //printf("v_in  %f %f %f %f\n", v_in[0], v_in[1], v_in[2], v_in[3]);
        //printf("v_out %f %f %f %f\n", v_out[0], v_out[1], v_out[2], v_out[3]);
    }

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vs,
                .indices = es,
                .vertices_num = CUBE_VERTICES_NUM,
                .indices_num = CUBE_INDICES_NUM},
                .desc = (VERTICES_DESC){.primitive = GL_TRIANGLES,
                .vf = VF_V3T2N3,
                .ef = IF_UI16}};
}

extern VERTICES_INFO
vertgen_sphere(const GEN_SHPERE_INFO *info, matrix4 transform) {
    assert(info->rings > 0);
    assert(info->sectors > 0);

    //float radius = 1.f;

    const float R = 1. / (float)(info->rings - 1);
    const float S = 1. / (float)(info->sectors - 1);
    int r, s;

    v3t2n3_t *vertices = (v3t2n3_t*)malloc(info->rings * info->sectors * sizeof(v3t2n3_t));

    if (!vertices) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    v3t2n3_t *v = vertices;

    for (r = 0; r < info->rings; r++) {
        for (s = 0; s < info->sectors; s++) {
            const float y = sin(-M_PI_2 + M_PI * r * R );
            const float x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
            const float z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

            (*v).texcoord[0] = s * S;
            (*v).texcoord[1] = r * R;

            (*v).position[0] = x * info->radius;
            (*v).position[1] = y * info->radius;
            (*v).position[2] = z * info->radius;

            (*v).normal[0] = x;
            (*v).normal[1] = y;
            (*v).normal[2] = z;

            v++;
        }
    }

    uint16_t *indices = (uint16_t*)malloc(info->rings * info->sectors * 6 * sizeof(uint16_t));

    if (!indices) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    uint16_t *i = indices;

    for (r = 0; r < info->rings; r++) {
        for (s = 0; s < info->sectors; s++) {
            *i++ = r * info->sectors + s; // 0
            *i++ = r * info->sectors + (s + 1); // 1
            *i++ = (r + 1) * info->sectors + s; // 3
            *i++ = r * info->sectors + (s + 1); // 1
            *i++ = (r + 1) * info->sectors + (s + 1); // 2
            *i++ = (r + 1) * info->sectors + s; // 3
        }
    }

    for (int j = 0; j < info->rings * info->sectors; j++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, vertices[j].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vertices[j].position, v_out);
    }

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vertices,
                .indices = indices,
                .vertices_num = info->rings * info->sectors,
                .indices_num = info->rings * (info->sectors - 1) * 6},
                .desc = (VERTICES_DESC){.primitive = GL_TRIANGLES,
                .vf = VF_V3T2N3}};
}

extern VERTICES_INFO
vertgen_quad_plane(matrix4 transform) {
    v3t2n3_t *vs = malloc(sizeof(quad_vertices));
    uint16_t *es = malloc(sizeof(quad_indices));

    memcpy(vs, quad_vertices, sizeof(quad_vertices));
    memcpy(es, quad_indices, sizeof(quad_indices));

    for (size_t i = 0; i < QUAD_VERTICES_NUM; i++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, quad_vertices[i].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vs[i].position, v_out);

        //printf("v_in  %f %f %f %f\n", v_in[0], v_in[1], v_in[2], v_in[3]);
        //printf("v_out %f %f %f %f\n", v_out[0], v_out[1], v_out[2], v_out[3]);
    }

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vs,
                .indices = es,
                .vertices_num = QUAD_VERTICES_NUM,
                .indices_num = QUAD_INDICES_NUM},
                .desc = (VERTICES_DESC){.primitive = GL_TRIANGLES,
                .vf = VF_V3T2N3,
                .ef = IF_UI16}};
}

extern VERTICES_INFO
vertgen_torus(const GEN_TORUS_INFO *info, matrix4 transform) {
    float s = 0;
    float t = 0;

    float torusRadius = (info->outerRadius - info->innerRadius) / 2.0f;
    float centerRadius = info->outerRadius - torusRadius;

    uint32_t indexVertices, indexIndices, indexNormals, indexTexCoords;
    uint32_t sideCount, faceCount;
    uint32_t v0, v1, v2, v3;

    uint32_t numberVertices = (info->numberStacks + 1) * (info->numberSlices + 1);
    uint32_t numberIndices = info->numberStacks * info->numberSlices * 2 * 3; // 2 triangles per face * 3 indices per triangle

    float *vertices = malloc(4 * numberVertices * sizeof(float));
    float *normals = malloc(3 * numberVertices * sizeof(float));
    float *texCoords = malloc(2 * numberVertices * sizeof(float));
    uint16_t *indices = malloc(numberIndices * sizeof(uint16_t));

    float sIncr = 1.0f / (float) info->numberSlices;
    float tIncr = 1.0f / (float) info->numberStacks;

    // generate vertices and its attributes
    for (sideCount = 0; sideCount <= info->numberSlices; ++sideCount, s += sIncr) {
        float cos2PIs = (float) cosf(2.0f * M_PI * s);
        float sin2PIs = (float) sinf(2.0f * M_PI * s);

        t = 0.0f;
        for (faceCount = 0; faceCount <= info->numberStacks; ++faceCount, t += tIncr) {
            float cos2PIt = (float) cosf(2.0f * M_PI * t);
            float sin2PIt = (float) sinf(2.0f * M_PI * t);

            // generate vertex and stores it in the right position
            indexVertices = ((sideCount * (info->numberStacks + 1)) + faceCount) * 4;
            vertices[indexVertices + 0] = (centerRadius + torusRadius * cos2PIt) * cos2PIs;
            vertices[indexVertices + 1] = (centerRadius + torusRadius * cos2PIt) * sin2PIs;
            vertices[indexVertices + 2] = torusRadius * sin2PIt;
            vertices[indexVertices + 3] = 1.0f;

            // generate normal and stores it in the right position
            // NOTE: cos (2PIx) = cos (x) and sin (2PIx) = sin (x) so, we can use this formula
            //       normal = {cos(2PIs)cos(2PIt) , sin(2PIs)cos(2PIt) ,sin(2PIt)}
            indexNormals = ((sideCount * (info->numberStacks + 1)) + faceCount) * 3;
            normals[indexNormals + 0] = cos2PIs * cos2PIt;
            normals[indexNormals + 1] = sin2PIs * cos2PIt;
            normals[indexNormals + 2] = sin2PIt;

            // generate texture coordinates and stores it in the right position
            indexTexCoords = ((sideCount * (info->numberStacks + 1)) + faceCount) * 2;
            texCoords[indexTexCoords + 0] = s;
            texCoords[indexTexCoords + 1] = t;
        }
    }

    // generate indices
    indexIndices = 0;
    for (sideCount = 0; sideCount < info->numberSlices; ++sideCount) {
        for (faceCount = 0; faceCount < info->numberStacks; ++faceCount) {
            // get the number of the vertices for a face of the torus. They must be < numVertices
            v0 = ((sideCount * (info->numberStacks + 1)) + faceCount);
            v1 = (((sideCount + 1) * (info->numberStacks + 1)) + faceCount);
            v2 = (((sideCount + 1) * (info->numberStacks + 1)) + (faceCount + 1));
            v3 = ((sideCount * (info->numberStacks + 1)) + (faceCount + 1));

            // first triangle of the face, counter clock wise winding
            indices[indexIndices++] = v0;
            indices[indexIndices++] = v1;
            indices[indexIndices++] = v2;

            // second triangle of the face, counter clock wise winding
            indices[indexIndices++] = v0;
            indices[indexIndices++] = v2;
            indices[indexIndices++] = v3;
        }
    }

    v3t2n3_t *vs = malloc(numberVertices * sizeof(v3t2n3_t));

    for (uint32_t i = 0; i < numberVertices; i++) {
        vs[i].position[0] = vertices[i * 4 + 0];
        vs[i].position[1] = vertices[i * 4 + 1];
        vs[i].position[2] = vertices[i * 4 + 2];

        vs[i].normal[0] = normals[i * 3 + 0];
        vs[i].normal[1] = normals[i * 3 + 1];
        vs[i].normal[2] = normals[i * 3 + 2];

        vs[i].texcoord[0] = texCoords[i * 2 + 0];
        vs[i].texcoord[1] = texCoords[i * 2 + 1];
    }

    //free(vs);
    free(vertices);
    free(normals);
    free(texCoords);
    //free(indices);

    for (size_t i = 0; i < numberVertices; i++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, vs[i].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vs[i].position, v_out);
    }

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vs,
                .indices = indices,
                .vertices_num = numberVertices,
                .indices_num = numberIndices},
                .desc = (VERTICES_DESC){.primitive = GL_TRIANGLES,
                .vf = VF_V3T2N3,
                .ef = IF_UI16}};
}

extern VERTICES_INFO
vertgen_ribbon(const GEN_RIBBON_INFO *info, matrix4 transform) {
    assert(info->segments > 0);

    v3t2n3_t *vertices = malloc(info->segments * 4 * sizeof(v3t2n3_t));
    uint16_t *indices = malloc(info->segments * 6 * sizeof(uint16_t));

    if (!vertices) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    if (!indices) {
        LOG_CRITICAL("%s\n", "Can't alloc memory");
        exit(EXIT_FAILURE);
    }

    float offset = 0.f;
    float width = info->width;

    size_t vertices_count = 0;
    size_t indices_count = 0;

    for(int i = 0; i < info->segments; i++) {
        vertices_count += 4;

        make_vector3(vertices[i * 4 + 0].position, -1.f * width + offset,  1.f * width, 0.f);
        make_vector2(vertices[i * 4 + 0].texcoord, 0.f, 1.f);
        make_vector3(vertices[i * 4 + 0].normal, 0.f, 0.f, 1.f);

        make_vector3(vertices[i * 4 + 1].position, 1.f * width + offset,  1.f * width, 0.f);
        make_vector2(vertices[i * 4 + 1].texcoord, 1.f, 1.f);
        make_vector3(vertices[i * 4 + 1].normal, 0.f, 0.f, 1.f);

        make_vector3(vertices[i * 4 + 2].position, 1.f * width + offset, -1.f * width, 0.f);
        make_vector2(vertices[i * 4 + 2].texcoord, 1.f, 0.f);
        make_vector3(vertices[i * 4 + 2].normal, 0.f, 0.f, 1.f);

        make_vector3(vertices[i * 4 + 3].position, -1.f * width + offset, -1.f * width, 0.f);
        make_vector2(vertices[i * 4 + 3].texcoord, 0.f, 0.f);
        make_vector3(vertices[i * 4 + 3].normal, 0.f, 0.f, 1.f);

        int base_index = vertices_count - 4;

        indices[0 + indices_count] = base_index + 0;
        indices[1 + indices_count] = base_index + 3;
        indices[2 + indices_count] = base_index + 1;

        indices[3 + indices_count] = base_index + 1;
        indices[4 + indices_count] = base_index + 3;
        indices[5 + indices_count] = base_index + 2;

        indices_count += 6;

        offset += 2.0f;
    }

    for (size_t i = 0; i < vertices_count; i++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, vertices[i].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vertices[i].position, v_out);
    }

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vertices,
                .indices = indices,
                .vertices_num = info->segments * 4,
                .indices_num = info->segments * 6},
                .desc = (VERTICES_DESC){.primitive = GL_TRIANGLES,
                .vf = VF_V3T2N3,
                .ef = IF_UI16}};
}

extern VERTICES_INFO
vertgen_rectangular_grid_plane(const GEN_RECTANGULAR_GRID_PLANE *info, matrix4 transform) {
    uint32_t numberVertices = (info->rows + 1) * (info->columns + 1);
    uint32_t numberIndices;
    uint32_t mode;

    if (info->triangleStrip) {
        numberIndices = info->rows * 2 * (info->columns + 1);
        mode = GL_TRIANGLE_STRIP;
    } else {
        numberIndices = info->rows * 6 * info->columns;
        mode = GL_TRIANGLES;
    }

    float *vertices = malloc(4 * numberVertices * sizeof(float));
    float *normals = malloc(3 * numberVertices * sizeof(float));
    float *texCoords = malloc(2 * numberVertices * sizeof(float));
    ushort *indices = malloc(numberIndices * sizeof(ushort));

    for (uint32_t i = 0; i < numberVertices; i++) {
        float x = (float) (i % (info->columns + 1)) / (float) info->columns;
        float y = 1.0f - (float) (i / (info->columns + 1)) / (float) info->rows;
        float s = x * info->columns;
        float t = y * info->rows;

        vertices[i * 4 + 0] = info->horizontalExtend * (x - 0.5f);
        vertices[i * 4 + 1] = info->verticalExtend * (y - 0.5f);
        vertices[i * 4 + 2] = 0.0f;
        vertices[i * 4 + 3] = 1.0f;

        normals[i * 3 + 0] = 0.0f;
        normals[i * 3 + 1] = 0.0f;
        normals[i * 3 + 2] = 1.0f;

        texCoords[i * 2 + 0] = s;
        texCoords[i * 2 + 1] = t;
    }

    if (info->triangleStrip)
        for (uint32_t i = 0; i < info->rows * (info->columns + 1); i++) {
            uint32_t currentColumn = i % (info->columns + 1);
            uint32_t currentRow = i / (info->columns + 1);

            if (currentRow == 0) {
                // Left to right, top to bottom
                indices[i * 2] = currentColumn + currentRow * (info->columns + 1);
                indices[i * 2 + 1] = currentColumn + (currentRow + 1) * (info->columns + 1);
            } else {
                // Right to left, bottom to up
                indices[i * 2] = (info->columns - currentColumn) + (currentRow + 1) * (info->columns + 1);
                indices[i * 2 + 1] = (info->columns - currentColumn) + currentRow * (info->columns + 1);
            }
        }
    else
        for (uint32_t i = 0; i < info->rows * info->columns; i++) {
            uint32_t currentColumn = i % info->columns;
            uint32_t currentRow = i / info->columns;

            indices[i * 6 + 0] = currentColumn + currentRow * (info->columns + 1);
            indices[i * 6 + 1] = currentColumn + (currentRow + 1) * (info->columns + 1);
            indices[i * 6 + 2] = (currentColumn + 1) + (currentRow + 1) * (info->columns + 1);

            indices[i * 6 + 3] = (currentColumn + 1) + (currentRow + 1) * (info->columns + 1);
            indices[i * 6 + 4] = (currentColumn + 1) + currentRow * (info->columns + 1);
            indices[i * 6 + 5] = currentColumn + currentRow * (info->columns + 1);
        }

    v3t2n3_t *vs = malloc(numberVertices * sizeof(v3t2n3_t));

    for (uint32_t i = 0; i < numberVertices; i++) {
        vs[i].position[0] = vertices[i * 4 + 0];
        vs[i].position[1] = vertices[i * 4 + 1];
        vs[i].position[2] = vertices[i * 4 + 2];

        vs[i].normal[0] = normals[i * 3 + 0];
        vs[i].normal[1] = normals[i * 3 + 1];
        vs[i].normal[2] = normals[i * 3 + 2];

        vs[i].texcoord[0] = texCoords[i * 2 + 0];
        vs[i].texcoord[1] = texCoords[i * 2 + 1];
    }

    for (size_t i = 0; i < numberVertices; i++) {
        float4 v_in;
        float4 v_out;
        copy_vector3(v_in, vs[i].position);
        v_in[3] = 1;

        left_product_matrix4_vector4(v_out, v_in, transform);
        copy_vector3(vs[i].position, v_out);
    }

    free(vertices);
    free(normals);
    free(texCoords);    

    return (VERTICES_INFO){.data = (VERTICES_DATA){.vertices = vs,
                .indices = indices,
                .vertices_num = numberVertices,
                .indices_num = numberIndices},
                .desc = (VERTICES_DESC){.primitive = mode,
                .vf = VF_V3T2N3,
                .ef = IF_UI16}};
}
