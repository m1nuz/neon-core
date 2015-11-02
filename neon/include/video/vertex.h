#pragma once

enum DefaultAttributes {
    POSITION_ATTRIBUTE  = 0,
    TEXCOORD_ATTRIBUTE  = 1,
    NORMAL_ATTRIBUTE    = 2,
    COLOR_ATTRIBUTE     = 4,
    TANGENT_ATTRIBUTE   = 5
};

enum VERTEX_FORMAT {
    VF_UNKNOWN  = -1,
    VF_V3       =  0,
    VF_V3N3,
    VF_V3T2,
    VF_V3T2N3,
    VF_V3T2C4,
    VF_V2T2C4
};

enum INDEX_FORMAT {
    IF_UNKNOWN,
    IF_UI16,
    IF_UI32
};

typedef struct v3t2n3_t {
    float position[3];
    float texcoord[2];
    float normal[3];
} v3t2n3_t; // 32b

typedef struct v3t2c4_t {
    float position[3];
    float texcoord[2];
    float color[4];
} v3t2c4_t; // 36b

typedef struct v3n3_t {
    float position[3];
    float normal[3];
} v3n3_t; // 24b

typedef struct v3t2_t {
    float position[3];
    float texcoord[2];
} v3t2_t; // 20b

typedef struct v3c4_t {
    float position[3];
    float color[4];
} v3c4_t; // 28

typedef struct v3_t {
    float position[3];
} v3_t; // 16b

// TODO: use it for sprites
typedef struct v2t2c4 {
    float position[2];
    float texcoord[2];
    float color[4];
} v2t2c4; // 32b

typedef struct v3t2n3t3_t {
    float position[3];
    float texcoord[2];
    float normal[3];
    float tangent[3];
} v3t2n3t3_t; // 44b
