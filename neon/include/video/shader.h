#pragma once

#include <video/gl.h>
#include <stdbool.h>

#define UNIFORMS_HASHED_SEARCH
#define ATTRIBUTES_HASHED_SEARCH
#define MAX_SHADER_UNIFORMS         1000
#define MAX_UNIFORM_NAME_SIZE       80
#define MAX_ATTRIBUTE_NAME_SIZE     80

#ifdef GL_ES_VERSION_2_0
#define MAX_SHADER_TYPES_SUPPORTED  2
#else
#define MAX_SHADER_TYPES_SUPPORTED  3
#endif // NO GL_ES_VERSION_2_0

typedef struct Uniform {
    char            name[MAX_UNIFORM_NAME_SIZE];
    unsigned        name_hash;
    int             location;
    int             num;
    unsigned int    type;
} UNIFORM;

typedef struct Attribute {
    char            name[MAX_ATTRIBUTE_NAME_SIZE];
    unsigned        name_hash;
    int             location;
    int             num;
    unsigned int    type;
} ATTRIBUTE;

typedef struct AttributeInfo {
    const char      *name;
    int             location;
} ATTRIBUTE_INFO; // Binding attributes

typedef struct ShaderSource {
    char            *name;
    char            *source;
    size_t          size;
    bool            reload;
} SHADER_SOURCE;

enum ShaderType {
    ST_VERTEX,
    ST_FRAGMENT,
    ST_GEOMETRY
};

typedef struct Shader {
    GLuint          pid;

    UNIFORM         *uniforms;
    int             uniforms_count;

    ATTRIBUTE       *attributes;
    int             attributes_count;

    // TODO: save feedback varying

    uint32_t        shaders[MAX_SHADER_TYPES_SUPPORTED];
    SHADER_SOURCE   sources[MAX_SHADER_TYPES_SUPPORTED];

    uint64_t        hash[MAX_SHADER_TYPES_SUPPORTED];
    uint32_t        usage;
} SHADER;

typedef struct FeedbackVaryings {
    const char  **varyings;
    int         count;
    GLenum      buffer_mode;
} FEEDBACK_VARYINGS;

typedef struct ShaderSourceInfo {
    const char  *text;
    const char  *name;
    bool        reload;
} SHADER_SOURCE_INFO;

typedef struct ShaderInfo {
    SHADER_SOURCE_INFO vs;
    SHADER_SOURCE_INFO gs;
    SHADER_SOURCE_INFO fs;
} SHADER_INFO;

SHADER new_shader_ext(const SHADER_INFO *info, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings);
void free_shader(SHADER *shader);
void rebuild_shader(SHADER *shader, const SHADER_INFO *info, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings);

SHADER upload_shader(const char *names);
SHADER upload_shader_ext(const char *names, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings);

void bind_shader(SHADER *shader);
void unbind_shader(SHADER *shader);

int shader_get_uniform(SHADER *shader, const char *name);
int shader_get_attribute(SHADER *shader, const char *name);
unsigned int shader_uniform_type(SHADER *shader, const char *name);
