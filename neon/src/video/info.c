#include <stdio.h>
#include <string.h>

#include "video/info.h"

GLboolean
video_is_extension_supported(const char *extension) {
#ifdef GL_VERSION_3_0
    if (glGetStringi == NULL) {
        if (strstr((const char *)glGetString(GL_EXTENSIONS), extension) != NULL)
            return true;

        return false;
    }

    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    for (int i = 0; i < num_extensions; i++)
        if (strcmp((const char *)glGetStringi(GL_EXTENSIONS, i), extension) == 0)
            return true;
#else
    if (strstr((const char *)glGetString(GL_EXTENSIONS), extension) != NULL)
        return true;
#endif
    return false;
}

const char *
video_get_opengl_info(void) {
    static const GLubyte *vendor = NULL;
    static const GLubyte *renderer = NULL;
    static const GLubyte *version = NULL;
    static const GLubyte *shading_language_version = NULL;

    static char glinfo[200] = {0};
    static int glinfo_count = 0;

    if ((!vendor || !renderer || !version || !shading_language_version) && (glinfo_count == 0)) {
        vendor = glGetString(GL_VENDOR);
        renderer = glGetString(GL_RENDERER);
        version = glGetString(GL_VERSION);
        shading_language_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

        snprintf(glinfo, sizeof(glinfo), "Vendor: %s\nRenderer: %s\nVersion: %s\nShading language version: %s",
                 vendor, renderer, version, shading_language_version);

        glinfo_count++;
    }

    return glinfo;
}
