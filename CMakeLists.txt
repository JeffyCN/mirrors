#
# Removes the specified compile flag from the specified target.
#   _target     - The target to remove the compile flag from
#   _flag       - The compile flag to remove
#
# Pre: apply_global_cxx_flags_to_all_targets() must be invoked.
#
macro(remove_flag_from_target _target _flag)
    get_target_property(_target_cxx_flags ${_target} COMPILE_OPTIONS)
    if(_target_cxx_flags)
        list(REMOVE_ITEM _target_cxx_flags ${_flag})
        set_target_properties(${_target} PROPERTIES COMPILE_OPTIONS "${_target_cxx_flags}")
    endif()
endmacro()

if(NOT CMAKE_BUILD_TYPE)
    message("default to Release build for GCC builds")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(CMAKE_BUILD_TARGET STREQUAL ndk)
    message("set ndk toolchain")
    set(CMAKE_CXX_FLAGS_RELEASE "-Wno-error -DLOG_TAG=\"librga\" -DRK_DRM_GRALLOC=1 -DANDROID_8 -DANDROID_7_DRM")
    add_compile_options(-DLINUX)
    add_compile_options(-DANDROID_VNDK)
endif()

if(CMAKE_BUILD_TARGET STREQUAL buildroot)
    message("set buildroot toolchain")
    set(CMAKE_TOOLCHAIN_FILE 
        ${CMAKE_SOURCE_DIR}/buildroot.cmake)
    include(${CMAKE_SOURCE_DIR}/buildroot.cmake)
    add_compile_options(-DLINUX)
endif()

set(CC "${CMAKE_C_COMPILER}")
set(CXX "${CMAKE_CXX_COMPILER}")
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("CMAKE_FIND_ROOT_PATH: ${CMAKE_FIND_ROOT_PATH}")
message("CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message("ANDROID_NDK_REVISION_REGEX: ${ANDROID_NDK_REVISION_REGEX}")

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUN_NEED_GL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUN_NEED_GL")

add_library(rga SHARED ${IM2D_SRCS})

if(CMAKE_BUILD_TARGET STREQUAL ndk)
    remove_flag_from_target(rga -DANDROID)
endif()

set(IM2D_DEMO_SRCS
    samples/im2d_api_demo/rgaImDemo.cpp
    samples/im2d_api_demo/args.cpp)

add_executable(im2d ${IM2D_DEMO_SRCS})

target_link_libraries(im2d
    rga
	pthread)
