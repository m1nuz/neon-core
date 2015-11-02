cmake_minimum_required(VERSION 2.8)

include(${MODULES_PATH}/GenerateMain.cmake)

# Game project
set(game_name "test-neon-cube")
set(game_states gameplay)
set(GL_TARGET "GL330")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project("Test")

set(game_headers
)

set(game_sources
    main.c
    src/cube.c
)

include_directories("./include")
include_directories("../argon/include")
include_directories("../neon/include")
include_directories("../lib/memtrack/include")
include_directories("../lib/GL/include")
include_directories("../lib/GLcore/include")

set(game_libs
    argon-base
    neon-engine
)

if(WIN32)
include_directories("../lib/SDL2/include")
include_directories("../lib/SDL2_ttf/include")
include_directories("../lib/OpenAL/include")
link_directories(${LIB_PATH}/SDL2/lib/win64)
endif()

if(GL_TARGET STREQUAL "GL330")
include_directories("../lib/GL330/include")
endif()

if(GL_TARGET STREQUAL "GLES2")
include_directories("../lib/GLES2/include")
endif()

add_definitions(-DOPENAL_BACKEND)
add_definitions(-D_GNU_SOURCE)

add_executable(${game_name} WIN32 ${game_headers} ${game_sources})
generate_main(${game_name} "${game_states}" game_startup)
target_link_libraries(${game_name} ${game_libs})
set_target_properties(${game_name} PROPERTIES COMPILE_FLAGS "-std=c11 -pedantic -Wall")
