if(NOT CMAKE_BUILD_TYPE)
    message("default to Release build for GCC builds")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(CMAKE_BUILD_TARGET STREQUAL ndk)
    message("set ndk toolchain")
    set(ANDROID_NDK_HOME
         "/home/cc/mksdk/build/android-ndk-r21d"
         CACHE INTERNAL "ANDROID_NDK_HOME")
    set(ANDROID_NDK
         "${ANDROID_NDK_HOME}"
         CACHE INTERNAL "ANDROID_NDK")
    set(ANDROID_ABI
         "armeabi-v7a"
         CACHE INTERNAL "ANDROID_ABI")
    set(ANDROID_TOOLCHAIN
         "clang"
         CACHE INTERNAL "ANDROID_TOOLCHAIN")
    set(ANDROID_PLATFORM
         "android-27"
         CACHE INTERNAL "ANDROID_PLATFORM")
    set(ANDROID_STL
         "c++_shared"
         CACHE INTERNAL "ANDROID_STL")
    set(CMAKE_TOOLCHAIN_FILE 
         "${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake"
         CACHE INTERNAL "CMAKE_TOOLCHAIN_FILE")
    include(${ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
endif()

if(CMAKE_BUILD_TARGET STREQUAL buildroot)
    message("set buildroot toolchain")
    set(CMAKE_TOOLCHAIN_FILE 
        ${CMAKE_SOURCE_DIR}/buildroot.cmake)
    include(${CMAKE_SOURCE_DIR}/buildroot.cmake)
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
    external/libdrm
    external/libdrm/include/drm
    hardware/libhardware/include/hardware
    hardware/libhardware/modules/gralloc
    system/core/liblog/includeNDROID_NDK_REVISION_REGEX
    system/core/libion/kernel-headers
    )

set(IM2D_SRCS
    RockchipRga.cpp
    GraphicBuffer.cpp
    normal/NormalRga.cpp
    normal/NormalRgaApi.cpp
    im2d_api/im2d.cpp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUN_NEED_GL")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUN_NEED_GL -std=c++1y")
#set(CMAKE_CXX_FLAGS  "-D__ANDROID_VNDK__")
add_compile_options(-std=c++1y)

set(CMAKE_CXX_FLAGS_RELEASE "-Wno-error -DLOG_TAG=\"librga\" -DRK_DRM_GRALLOC=1 -DANDROID_8 -DANDROID_7_DRM")

add_library(rga SHARED ${IM2D_SRCS})

#find_library(native-log-lib
#    log)

#target_link_libraries(rga
#    ${native-log-lib})

set(IM2D_DEMO_SRCS
    demo/im2d_api_demo/RockchipFileOps.cpp
    demo/im2d_api_demo/rgaImDemo.cpp
    demo/im2d_api_demo/args.cpp)

add_executable(im2d ${IM2D_DEMO_SRCS})

target_link_libraries(im2d
    rga
	pthread)
