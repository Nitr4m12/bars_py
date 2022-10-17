# Conf based on https://github.com/pabloariasal/modern-cmake-sample

cmake_minimum_required(VERSION 3.10)
project(libbars VERSION 1.0.0 LANGUAGES CXX)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

find_package(blobify QUIET)

if(NOT blobify_FOUND)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/blobify-CMakeLists.txt.in
        ${CMAKE_BINARY_DIR}/blobify-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/blobify-download)
    if(result)
        message(FATAL_ERROR "CMake step for blobify failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/blobify-download)
    if(result)
        message(FATAL_ERROR "Build step for blobify failed: ${result}")
    endif()

    add_subdirectory(${CMAKE_BINARY_DIR}/blobify-src
        ${CMAKE_BINARY_DIR}/blobify-build
        EXCLUDE_FROM_ALL)

endif()

add_library(bars
    src/bars.cpp
)
target_include_directories(bars
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(bars 
    PUBLIC
        blobify
)

enable_testing()
add_subdirectory(tests)