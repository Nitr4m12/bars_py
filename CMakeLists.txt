# Conf based on https://github.com/pabloariasal/modern-cmake-sample

cmake_minimum_required(VERSION 3.10)
project(libbars VERSION 1.0.0 LANGUAGES CXX)


set(ABSL_PROPAGATE_CXX_STD ON)

include(FetchContent)
FetchContent_Declare(
    oead
    GIT_REPOSITORY https://github.com/zeldamods/oead.git
)
FetchContent_MakeAvailable(oead)

add_library(bars
    src/bars.cpp
    src/amta.cpp
)
target_include_directories(bars
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(bars oead)
target_compile_features(bars PUBLIC cxx_std_17)

enable_testing()
add_subdirectory(tests)