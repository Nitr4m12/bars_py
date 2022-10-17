cmake_minimum_required(VERSION 3.10)
project(libbars VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(bars
    src/bars.cpp
    src/util.h)

target_include_directories(bars PUBLIC include/ src/)

enable_testing()
add_subdirectory(tests/)