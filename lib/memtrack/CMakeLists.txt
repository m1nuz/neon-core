cmake_minimum_required(VERSION 2.8)

include_directories("./include")
add_library(memtrack STATIC include/memtrack.h src/memtrack.c)
