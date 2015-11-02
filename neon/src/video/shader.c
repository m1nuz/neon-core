#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <xxhash.h>
#include <memtrack.h>

#include <base/str_replace.h>
#include <base/str_insert.h>
#include <base/pjw.h>

#include "video/shader.h"

#include "core/common.h"
#include "core/logerr.h"
#include "core/text.h"
#include "core/asset.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

static size_t
print_shader_source(const char *text, size_t size) {
    FILE *fp = fmemopen((void*)text, size, "r");
    size_t num = 0;

    char line[256];
    while (!feof(fp)) {
        if (fgets(line, sizeof(line), fp) != NULL)
            LOG("%4zu %s", num + 1, line);

        num++;
    }
    LOG("%s\n", "END");

    fclose(fp);

    return num;
}

#pragma GCC diagnostic pop

static size_t
get_definitions_position(const char *text) {
    const char *p = strstr(text, "#version");

    if (!p)
        return 0;

    const char *end = strchr(p, '\n');

    if (!end)
        return 0;

    return (size_t)(end - text) + 1;
}

static void
compile_shader(SHADER *shader, int type, const char *definitions) {
    if (!shader->shaders[type])
        return;

    assert(shader->sources[type].source);

    if (definitions) {
        size_t pos = get_definitions_position(shader->sources[type].source);
        char *source = str_insert(shader->sources[type].source, definitions, pos);
        free(shader->sources[type].source);

        shader->sources[type].source = source;
        shader->sources[type].size = strlen(shader->sources[type].source);
    }

    const char *fullsource[] =  {shader->sources[type].source, 0};

    glShaderSource(shader->shaders[type], 1, fullsource, 0);
    glCompileShader(shader->shaders[type]);

    GLint status = 0;
    glGetShaderiv(shader->shaders[type], GL_COMPILE_STATUS, &status);

    if (!status) {
        GLint lenght = 0;
        glGetShaderiv(shader->shaders[type], GL_INFO_LOG_LENGTH, &lenght);

        char info[lenght];

        GLsizei written = 0;
        glGetShaderInfoLog(shader->shaders[type], sizeof(info), &written, info);

        LOG("%s\n", "SHADER:");
        print_shader_source(shader->sources[type].source, shader->sources[type].size);

        LOG_CRITICAL("%s", info);
        exit(EXIT_FAILURE);
    }
}

static void
link_shader(GLuint pid) {
    GLint linked = 0;

    glLinkProgram(pid);
    glGetProgramiv(pid, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint lenght = 0;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &lenght);

        char info[lenght];

        GLsizei written = 0;
        glGetProgramInfoLog(pid, sizeof(info), &written, info);

        LOG_CRITICAL("%s\n", info);
        exit(EXIT_FAILURE);
    }
}

static void
validate_shader(GLuint pid) {
    GLint validate = 0;

    glValidateProgram(pid);
    glGetProgramiv(pid, GL_VALIDATE_STATUS, &validate);

    if(validate != GL_TRUE) {
        GLint lenght = 0;
        glGetProgramiv(pid, GL_INFO_LOG_LENGTH, &lenght);

        char info[lenght];

        GLsizei written = 0;
        glGetProgramInfoLog(pid, sizeof(info), &written, info);

        LOG_WARNING("%s\n", info);
    }
}

/*static int
compare_uniform(const void *a, const void *b) {
    return ((const UNIFORM*)a)->name_hash - ((const UNIFORM*)b)->name_hash;
}*/

//  How to get all uniforms and attributes:
//  http://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade
//  ARB_program_interface_query

//  How get uniform blocks:
//  http://gamedev.stackexchange.com/questions/48926/opengl-fetching-the-names-of-all-uniform-blocks-in-your-program

//  https://www.opengl.org/wiki/Program_Introspection

static int
get_program_attributes(GLuint pid, ATTRIBUTE **attr) {
    if (!attr)
        return -1;

    ATTRIBUTE *attributes = NULL;

    int total = -1;
    glGetProgramiv(pid, GL_ACTIVE_ATTRIBUTES, &total);

    if (total > 0) {
        attributes = malloc(total * sizeof(ATTRIBUTE));

        for (int i = 0; i < total; i++) {
            int name_len = -1, num = -1;
            GLenum type = GL_ZERO;
            glGetActiveAttrib(pid, i, sizeof(attributes[i].name) - 1, &name_len, &num, &type, attributes[i].name);

            attributes[i].name[name_len] = 0;
            attributes[i].location = glGetAttribLocation(pid, attributes[i].name);
            attributes[i].num = num;
            attributes[i].type = type;

            //printf("\tAttribute: %s %d\n", attributes[i].name, attributes[i].location);

#ifdef ATTRIBUTES_HASHED_SEARCH
            attributes[i].name_hash = pjw_hash(attributes[i].name);
#else
            attributes[i].name_hash = 0;
#endif
        }
    }

    *attr = attributes;

    return total;
}

static int
get_program_uniforms(GLuint pid, UNIFORM **uni) {
    if (!uni)
        return -1;

    UNIFORM *uniforms = NULL;

    int total = -1;
    glGetProgramiv(pid, GL_ACTIVE_UNIFORMS, &total);

    if (total > 0) {
        uniforms = malloc(total * sizeof(UNIFORM));

        for (int i = 0; i < total; ++i) {
            int name_len = -1, num = -1;
            GLenum type = GL_ZERO;
            glGetActiveUniform(pid, i, sizeof(uniforms[i].name) - 1, &name_len, &num, &type, uniforms[i].name);
            uniforms[i].name[name_len] = 0;
            uniforms[i].location = glGetUniformLocation(pid, uniforms[i].name);
            uniforms[i].num = num;
            uniforms[i].type = type;

            // TODO: this is problem with arrays
            // can't get to uniform location by array index, like 'array[index]'
            if (num > 1) {
                char * p = strchr(uniforms[i].name, '[');
                *p = '\0';
            }

#ifdef UNIFORMS_HASHED_SEARCH
            uniforms[i].name_hash = pjw_hash(uniforms[i].name);
#else
            uniforms[i].name_hash = 0;
#endif
            //printf("    %s %d [%d]\n", uniforms[i].name, uniforms[i].location, num);
        }
    }
#ifdef UNIFORMS_HASHED_SEARCH
    /*if (total > 1)
        qsort(uniforms, total, sizeof(UNIFORM), compare_uniform);*/
#endif
    *uni = uniforms;

    return total;
}

extern SHADER
new_shader_ext(const SHADER_INFO *info, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings) {
    SHADER sh;
    memset(&sh, 0, sizeof(sh));

    rebuild_shader(&sh, info, definitions, attributes, varyings);

    return sh;
}

extern void
free_shader(SHADER *shader) {
    assert(shader != NULL);

    if (glIsProgram(shader->pid))
        glDeleteProgram(shader->pid);
    shader->pid = 0;

    for (int i = 0; i < MAX_SHADER_TYPES_SUPPORTED; i++) {
        if (glIsShader(shader->shaders[i]))
            glDeleteShader(shader->shaders[i]);
        shader->shaders[i] = 0;

        if (shader->sources[i].name) {
            free(shader->sources[i].name);
            shader->sources[i].name = NULL;
        }

        if (shader->sources[i].source) {
            free(shader->sources[i].source);
            shader->sources[i].source = NULL;
        }

        shader->sources[i].reload = false;
        shader->sources[i].size = 0;
    }

    free(shader->uniforms);
    shader->uniforms = NULL;
    shader->uniforms_count = 0;

    free(shader->attributes);
    shader->attributes = NULL;
    shader->attributes_count = 0;

    memset(shader->hash, 0, sizeof(shader->hash));
    shader->usage = 0;
}

static const GLenum gl_shader_types[MAX_SHADER_TYPES_SUPPORTED] = {
    [ST_VERTEX] = GL_VERTEX_SHADER,
    [ST_FRAGMENT] = GL_FRAGMENT_SHADER,
    #ifndef GL_ES_VERSION_2_0
    [ST_GEOMETRY] = GL_GEOMETRY_SHADER
    #endif
};

#include <core/reader.h>

static void
create_shader(SHADER *shader, int type, const SHADER_SOURCE_INFO *info, const char *definitions) {
    assert(shader != NULL);

    const char *name = info ? info->name : NULL;
    const char *text = info ? info->text : NULL;

    if ((shader->shaders[type] == 0) &&
        ((shader->sources[type].source != NULL) || (text != NULL) || (name != NULL)))
        shader->shaders[type] = glCreateShader(gl_shader_types[type]);

    if (info) {
        if (!shader->sources[type].name)
            shader->sources[type].name = info->name ? strdup(info->name) : NULL;

        if (!shader->sources[type].reload)
            shader->sources[type].reload = info->reload;
    }

    if (shader->shaders[type]) {
        if (text) {
            free(shader->sources[type].source);
            shader->sources[type].source = strdup(text);
            shader->sources[type].size = strlen(text);
        } else if (shader->sources[type].reload) {
            if (shader->sources[type].name) {
                free(shader->sources[type].source);

                TEXT_DATA textdata;
                if(read_text(shader->sources[type].name, &textdata) != 0) {
                    LOG_CRITICAL("%s\n", "Can\'t read shader.");
                    exit(EXIT_FAILURE);
                }

                shader->sources[type].source = textdata.text;
                shader->sources[type].size = textdata.size;
            } else if (!shader->sources[type].source) {
                LOG_CRITICAL("%s\n", "Can\'t create shader withot any thing.");
                exit(EXIT_FAILURE);
            }
        }
    }

    compile_shader(shader, type, definitions);
    shader->hash[type] = XXH64(shader->sources[type].source, shader->sources[type].size, 0);
}

extern void
rebuild_shader(SHADER *shader, const SHADER_INFO *info, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings) {
    assert(shader != NULL);

    if (shader->pid == 0)
        shader->pid = glCreateProgram();

    create_shader(shader, ST_VERTEX, info ? &info->vs : NULL, definitions);
    create_shader(shader, ST_FRAGMENT, info ? &info->fs : NULL, definitions);
#ifndef GL_ES_VERSION_2_0
    create_shader(shader, ST_GEOMETRY, info ? &info->gs : NULL, definitions);
#endif

    for (int i = 0; i < MAX_SHADER_TYPES_SUPPORTED; i++)
        if (shader->shaders[i])
            glAttachShader(shader->pid, shader->shaders[i]);

#ifndef GL_ES_VERSION_2_0
    if (varyings) {
        switch(varyings->buffer_mode) {
        case GL_INTERLEAVED_ATTRIBS:
            // check GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS
            break;
        case GL_SEPARATE_ATTRIBS:
            // check GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS
            break;
        }

        // In the vertex shader variables should be output
        // and initialize by values ​​in the main
        glTransformFeedbackVaryings(shader->pid, varyings->count, varyings->varyings, varyings->buffer_mode);
    }
#else
    if (varyings)
        LOG_WARNING("%s\n", "Trasform feedback not supported!");
#endif

    if (attributes)
        while (*attributes) {
            glBindAttribLocation(shader->pid, (*attributes)->location, (*attributes)->name);

            attributes++;
        }

    link_shader(shader->pid);
    validate_shader(shader->pid);

    free(shader->uniforms);
    shader->uniforms = NULL;

    shader->uniforms_count = get_program_uniforms(shader->pid, &shader->uniforms);

    free(shader->attributes);
    shader->attributes = NULL;

    shader->attributes_count = get_program_attributes(shader->pid, &shader->attributes);
}

static void
get_shader_names(const char *names, char *name[]) {
    char s[200] = {0};
    char *ptr;

    strncpy(s, names, sizeof(s));

    int count = 0;

    char *p = strtok_r(s, " ", &ptr);

    while(p != NULL) {
        name[count] = strdup(p);
        count++;

        if(count > MAX_SHADER_TYPES_SUPPORTED)
            break;

        p = strtok_r(NULL, " ", &ptr);
    }
}

extern char *
load_shader_text(SDL_RWops *rw, size_t *size) {
    const char include_directive[] = "#include";
    const size_t step = sizeof(include_directive) - 1;
    char *text = load_text(rw, size);

    char *p = text;

    while (p < text + *size) {

        char *sp;
        if ((sp = strstr(p, include_directive)) != NULL) {
            size_t pos = p - text;
            char *end = strchr(sp, '\n');

            if ((size_t)(end - sp) == step) {
                LOG_CRITICAL("%s\n", "#include expect \"FILENAME\"");
                exit(EXIT_FAILURE);
            }

            char *bracers_first = strchr(sp, '\"');
            if (bracers_first == NULL) {
                LOG_CRITICAL("%s\n", "#include expect \"FILENAME\"");
                exit(EXIT_FAILURE);
            }

            char *space = sp + sizeof(include_directive);
            while (space < bracers_first)
                if (*space == ' ')
                    space++;
                else {
                    LOG_CRITICAL("%s\n", "#include expect \"FILENAME\"");
                    exit(EXIT_FAILURE);
                }

            char *bracers_second = strchr(bracers_first + 1, '\"');
            if (bracers_second == NULL) {
                LOG_CRITICAL("%s\n", "#include expect \"FILENAME\"");
                exit(EXIT_FAILURE);
            }

            size_t name_size = bracers_second - bracers_first - 1;
            if (name_size == 0) {
                LOG_CRITICAL("%s\n", "empty filename in #include");
                exit(EXIT_FAILURE);
            }

            char name[name_size];
            memcpy(name, bracers_first + 1, name_size);
            name[name_size] = 0;

            SDL_RWops *include_rw = asset_request(name);

            //memset(sp, ' ', end - sp);

            char origin[end - sp];
            memcpy(origin, sp, sizeof(origin));
            origin[end - sp] = 0;

            size_t include_size;
            char *replace = load_text(include_rw, &include_size);

            //printf("ORIGIN |%s|\nREPLACE |%s|\n", origin, replace);

            char *old_text = text;
            text = str_replace_once(origin, replace, text);
            p = text + pos;
            free(old_text);
            //printf("NEW SHADER TEXT ||||||||||%s||||||||||\n", text);

            free(replace);
        }

        p += step;
    }

    return text;
}

extern SHADER
upload_shader(const char *names) {
    return upload_shader_ext(names, NULL, NULL, NULL);
}

extern SHADER
upload_shader_ext(const char *names, const char *definitions, const ATTRIBUTE_INFO **attributes, const FEEDBACK_VARYINGS *varyings) {
    assert(names != NULL);

    char* name[MAX_SHADER_TYPES_SUPPORTED] = {0};

    get_shader_names(names, name);

    const char *vs_name = NULL;
    const char *fs_name = NULL;
    const char *gs_name = NULL;

    for (int i = 0; i < MAX_SHADER_TYPES_SUPPORTED; i++) {
        if (name[i]) {
            const char *ext = strrchr(name[i], '.');

            LOG("LOAD %s\n", name[i]);

            if (strcmp(ext, ".vert") == 0) {
                vs_name = name[i];
                continue;
            }

            if (strcmp(ext, ".frag") == 0) {
                fs_name = name[i];
                continue;
            }

            if (strcmp(ext, ".geom") == 0) {
                gs_name = name[i];
                continue;
            }
        }
    }

    SHADER shader = new_shader_ext(&(SHADER_INFO){.vs = {.text = NULL, .name = vs_name, .reload = true},
                                                  .fs = {.text = NULL, .name = fs_name, .reload = true},
                                                  .gs = {.text = NULL, .name = gs_name, .reload = true}},
                                   definitions, attributes, varyings);

    for (int i = 0; i < MAX_SHADER_TYPES_SUPPORTED; i++)
        free(name[i]);

    return shader;
}

extern void
bind_shader(SHADER *shader) {
    glUseProgram(shader->pid);
}

extern void
unbind_shader(SHADER *shader) {
    UNUSED(shader);

    glUseProgram(0);
}

/*static int
compare_uniform(const void *a, const void *b) {
    return ((const UNIFORM*)a)->name_hash - ((const UNIFORM*)b)->name_hash;
}*/

extern int
shader_get_uniform(SHADER *shader, const char *name) {
    assert(name != NULL);

#ifdef UNIFORMS_HASHED_SEARCH
    unsigned hash = pjw_hash(name);

    for(int i = 0; i < shader->uniforms_count; i++)
        if(shader->uniforms[i].name_hash == hash)
            return shader->uniforms[i].location;

    /*UNIFORM key;
    memset(&key, 0, sizeof(key));
    key.name_hash = hash;

    UNIFORM *p = bsearch(&key, shader->uniforms, shader->uniforms_count, sizeof(UNIFORM), compare_uniform);
    if (p)
        return p->location;*/
#else
    for(int i = 0; i < shader->uniforms_count; i++)
        if(strncmp(shader->uniforms[i].name, name, MAX_UNIFORM_NAME_SIZE) == 0)
            return shader->uniforms[i].location;
#endif // UNIFORMS_HASHED_SEARCH
    return -1;
}

extern int
shader_get_attribute(SHADER *shader, const char *name) {
    assert(name != NULL);

#ifdef ATTRIBUTES_HASHED_SEARCH
    unsigned hash = pjw_hash(name);

    for (int i = 0; i < shader->attributes_count; i++)
        if (shader->attributes[i].name_hash == hash)
            return shader->attributes[i].location;
#else
    for (int i = 0; i < shader->attributes_count; i++)
        if (strncmp(shader->attributes[i].name, name, MAX_ATTRIBUTE_NAME_SIZE) == 0)
            return shader->attributes[i].location;
#endif // ATTRIBUTES_HASHED_SEARCH
    return -1;
}

extern unsigned int
shader_uniform_type(SHADER *shader, const char *name) {
#ifdef UNIFORMS_HASHED_SEARCH
    unsigned hash = pjw_hash(name);

    for(int i = 0; i < shader->uniforms_count; i++)
        if(shader->uniforms[i].name_hash == hash)
            return shader->uniforms[i].type;
#endif
    return 0;
}

