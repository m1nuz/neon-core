cmake_minimum_required(VERSION 2.8)

include_directories("./include")
add_library(lz4 STATIC include/lz4.h src/lz4.c)
add_library(lz4hc STATIC include/lz4hc.h src/lz4hc.c)
