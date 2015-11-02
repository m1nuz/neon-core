cmake_minimum_required(VERSION 2.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(GL_TARGET "GL330")

set(core_headers
    include/core/common.h
    include/core/logerr.h
    include/core/filesystem.h
    include/core/frame.h
    include/core/atlas.h
    include/core/glyphs.h
    include/core/image.h
    include/core/rwtext.h
    include/core/asset.h
    include/core/configs.h
    include/core/package.h
    include/core/wavefront.h
    include/core/text.h
    include/core/wave.h
    include/core/targa.h
    include/core/lang.h
    include/core/reader.h
    include/core/audio.h
    include/core/video.h
    include/core/application.h
    include/neon.h
)

set(core_sources
    src/core/log.c
    src/core/filesystem.c
    src/core/frame.c
    src/core/atlas.c
    src/core/glyphs.c
    src/core/image.c
    src/core/rwtext.c
    src/core/asset.c
    src/core/configs.c
    src/core/wavefront.c
    src/core/text.c
    src/core/wave.c
    src/core/targa.c
    src/core/lang.c
    src/core/reader.c
    src/core/audio.c
    src/core/video.c
    src/core/application.c
)

set(video_headers
    include/video/gl.h
    include/video/info.h
    include/video/meminfo.h
    include/video/vertex.h
    include/video/state.h
    include/video/framebuffer.h
    include/video/texture.h
    include/video/sampler.h
    include/video/shader.h
    include/video/vertices.h
    include/video/resources.h
    include/video/gfx.h
    include/video/buffer.h
    include/video/commandbuffer.h
    include/video/sprite.h
    include/video/text.h
    include/video/stats.h)
set(video_sources
    src/video/info.c
    src/video/meminfo.c
    src/video/state.c
    src/video/framebuffer.c
    src/video/texture.c
    src/video/sampler.c
    src/video/shader.c
    src/video/vertices.c
    src/video/resources.c
    src/video/buffer.c
    src/video/commandbuffer.c
    src/video/sprite.c
    src/video/text.c
    src/video/stats.c)

include_directories("./include")
include_directories("../argon/include")
include_directories("../lib/lz4/include")
include_directories("../lib/xxhash/include")
include_directories("../lib/memtrack/include")
include_directories("../lib/cutef8")

set(engine_libs
    -lm
    lz4
    lz4hc
    xxhash
    -lSDL2
    -lSDL2_ttf
    argon-base
    memtrack
    -lGL -lGLU -lX11
)

if(UNIX)
list(APPEND engine_libs openal)
endif(UNIX)

if(WIN32)
list(APPEND engine_libs -lopengl32)
include_directories("../lib/SDL2/include")
include_directories("../lib/SDL2_ttf/include")
include_directories("../lib/OpenAL/include")
link_directories(${LIB_PATH}/SDL2/lib/win64)
list(APPEND engine_libs ${LIB_PATH}/OpenAL/libs/Win64/OpenAL32.lib)
endif()

if(GL_TARGET STREQUAL "GL330")
add_definitions(-DOPENGL_MAJOR_VERSION=3 -DOPENGL_MINOR_VERSION=3 -DOPENGL_CONTEXT_PROFILE_CORE)

list(APPEND engine_libs GLcore330)
endif()

if(GL_TARGET STREQUAL "GLES2")
add_definitions(-DOPENGL_MAJOR_VERSION=2 -DOPENGL_MINOR_VERSION=0 -DOPENGL_CONTEXT_PROFILE_ES)
include_directories("../lib/GLES2/include")
list(APPEND engine_libs GLcoreES2)
list(APPEND engine_libs -lEGL -lGLESv2 -lX11)
endif()

include_directories("../lib/GL/include")
include_directories("../lib/GLcore/include")

add_definitions(-DOPENAL_BACKEND)
add_definitions(-DVIDEO_SRGB_CAPABLE)
add_definitions(-D_GNU_SOURCE)

add_library(neon-engine STATIC ${core_headers} ${core_sources} ${video_headers} ${video_sources})
target_link_libraries(neon-engine ${engine_libs})
set_target_properties(neon-engine PROPERTIES COMPILE_FLAGS "-std=c11 -pedantic -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes")
