cmake_minimum_required(VERSION 2.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(sources
    src/main.c
    ../neon/src/core/filesystem.c
)

include_directories("../neon/include")
include_directories("../argon/include")
include_directories("../lib/lz4/include")
include_directories("../lib/minilzo/include")

add_executable(filespacker WIN32 ${sources})
target_link_libraries(filespacker minilzo lz4 lz4hc argon-base)
set_target_properties(filespacker PROPERTIES COMPILE_FLAGS "-std=c11 -pedantic -Wall -Wextra")
