if(NOT CMAKE_BUILD_TYPE)
    message("default to Release build for GCC builds")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

execute_process(COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/genversion.sh cmake)

if(CMAKE_BUILD_TARGET STREQUAL ndk)
    message("set ndk toolchain")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error -DLOG_TAG=librga")
    add_compile_options(-DLINUX)
    add_compile_options(-DANDROID_VNDK)
endif()

if(CMAKE_BUILD_TARGET STREQUAL buildroot)
    message("set buildroot toolchain")
    set(CMAKE_TOOLCHAIN_FILE 
        ${CMAKE_SOURCE_DIR}/buildroot.cmake)
    include(${CMAKE_SOURCE_DIR}/buildroot.cmake)
    add_compile_options(-DLINUX)
    set(CMAKE_CXX_FLAGS "-std=c++11 -O2 -pthread")
endif()

cmake_minimum_required(VERSION 2.8)

#project name
project(librga)

include_directories(
    ./
    ./include
    external/libdrm
    external/libdrm/include/drm
    hardware/libhardware/include/hardware
    hardware/libhardware/modules/gralloc
    system/core/liblog/includeNDROID_NDK_REVISION_REGEX
    system/core/libion/kernel-headers
    )

set(IM2D_SRCS
    core/RockchipRga.cpp
    core/GrallocOps.cpp
    core/NormalRga.cpp
    core/NormalRgaApi.cpp
    core/RgaUtils.cpp
    im2d_api/im2d.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUN_NEED_GL")

if(CMAKE_BUILD_TARGET STREQUAL ndk)
    string(REPLACE "-DANDROID" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

add_library(rga SHARED ${IM2D_SRCS})


#build im2d test
set(IM2D_DEMO_SRCS
    samples/im2d_api_demo/rgaImDemo.cpp
    samples/im2d_api_demo/args.cpp)

add_executable(im2d ${IM2D_DEMO_SRCS})

target_link_libraries(im2d
    rga)
