cmake_minimum_required(VERSION 3.5)

if (EXISTS ${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH})
    message("load ${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH}")
    include(${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH})
endif()

project(rga_samples_utils)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--allow-shlib-undefined -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--allow-shlib-undefined -ldl")

# allocator demo utils_obj
add_library(utils_obj OBJECT utils.cpp)
