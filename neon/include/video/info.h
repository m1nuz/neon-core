#pragma once

#include <stdbool.h>
#include <video/gl.h>

GLboolean video_is_extension_supported(const char *extension);
const char * video_get_opengl_info(void);
