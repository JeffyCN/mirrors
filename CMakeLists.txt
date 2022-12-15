cmake_minimum_required(VERSION 3.5)

include(${CMAKE_SOURCE_DIR}/cmake/utils.cmake)

if (EXISTS ${BUILD_TOOLCHAINS_PATH})
    message("load ${BUILD_TOOLCHAINS_PATH}")
    include(${BUILD_TOOLCHAINS_PATH})
endif()

project(rga)

if(NOT CMAKE_BUILD_TYPE)
    message("default to Release build for GCC builds")
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(CMAKE_BUILD_TARGET STREQUAL android_ndk)
    message("cmake build android ndk")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-error -DLOG_TAG=librga")
    add_compile_options(-DLINUX)
    add_compile_options(-DANDROID_VNDK)
    string(REPLACE "-DANDROID" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TARGET STREQUAL buildroot)
    message("cmake build buildroot")
    set(CMAKE_CXX_FLAGS "-std=c++11 -O2 -pthread")
    add_compile_options(-DLINUX)
endif()

if(CMAKE_BUILD_TARGET STREQUAL cmake_linux)
    message("RGA build with cmake")
    add_compile_options(-DLINUX)
    # TO support pass external CMAKE_CXX_FLAGS
    set(CMAKE_CXX_FLAGS_EXT "-std=c++11 -O2 -pthread -fdata-sections -ffunction-sections -fno-exceptions -fno-rtti")

    if (NOT DEFINED CMAKE_C_COMPILER)
        message(FATAL_ERROR "RGA: CMAKE_C_COMPILER not define")
    endif()

    if (NOT DEFINED CMAKE_CXX_COMPILER)
        message(FATAL_ERROR "RGA: CMAKE_CXX_COMPILER not define")
    endif()

endif()

if (BUILD_WITH_LIBDRM STREQUAL true)
    message("RGA build with libdrm")
    add_compile_options(-DLIBDRM=1)
    include_directories(
        ${LIBDRM_FILE_INCLUDE}/
        ${LIBDRM_FILE_INCLUDE}/libdrm
    )
endif()

if (BUILD_WITH_VERSION STREQUAL true)
    im2d_api_extract_version()
endif()

set(IM2D_SRCS
    core/utils/android_utils/src/android_utils.cpp
    core/utils/drm_utils/src/drm_utils.cpp
    core/utils/utils.cpp
    core/RockchipRga.cpp
    core/GrallocOps.cpp
    core/NormalRga.cpp
    core/NormalRgaApi.cpp
    core/RgaUtils.cpp
    core/RgaApi.cpp
    core/rga_sync.cpp
    im2d_api/src/im2d_log.cpp
    im2d_api/src/im2d_impl.cpp
    im2d_api/src/im2d.cpp)

set(IM2D_INCLUDE
    ./
    ./im2d_api
    ./include
    ./core
    ./core/hardware
    ./core/utils
    ./core/3rdparty/libdrm/include/drm
    hardware/libhardware/include/hardware
    hardware/libhardware/modules/gralloc
    system/core/liblog/includeNDROID_NDK_REVISION_REGEX
    system/core/libion/kernel-headers)

set(IM2D_PUBLIC_HEADER
    im2d_api/im2d_version.h
    im2d_api/im2d_type.h
    im2d_api/im2d_buffer.h
    im2d_api/im2d_common.h
    im2d_api/im2d_single.h
    im2d_api/im2d_task.h
    im2d_api/im2d_mpi.h
    im2d_api/im2d_expand.h
    im2d_api/im2d.h
    im2d_api/im2d.hpp
    include/drmrga.h
    include/rga.h
    include/RgaApi.h
    include/RockchipRga.h
    include/RgaUtils.h
    include/RgaSingleton.h
    include/RgaMutex.h
    include/GrallocOps.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_EXT} -DUN_NEED_GL")

#build object
set(OBJECT_NAME ${PROJECT_NAME}-object)

add_library(${OBJECT_NAME} OBJECT ${IM2D_SRCS})

target_include_directories(${OBJECT_NAME} PUBLIC ${IM2D_INCLUDE})
# Some older compilers need to explicitly set this property
# After setting this property, it can be used to generate a shared library
set_target_properties(${OBJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)

# build shared lib
set(SHARED_LIB_NAME ${PROJECT_NAME})

add_library(${SHARED_LIB_NAME} SHARED $<TARGET_OBJECTS:${OBJECT_NAME}>)

target_include_directories(${SHARED_LIB_NAME} PUBLIC ${IM2D_INCLUDE})

if(NOT DEFINED IM2D_API_VERSION)
    set_target_properties(${SHARED_LIB_NAME} PROPERTIES
        PUBLIC_HEADER "${IM2D_PUBLIC_HEADER}")
else ()
    message("Generated im2d api version: v${IM2D_API_VERSION}")
    set_target_properties(${SHARED_LIB_NAME} PROPERTIES
        PUBLIC_HEADER "${IM2D_PUBLIC_HEADER}"
        VERSION ${IM2D_API_VERSION} SOVERSION ${IM2D_API_VERSION_MAJOR})
endif()

# build static lib
set(STATIC_LIB_NAME ${PROJECT_NAME}-static)

add_library(${STATIC_LIB_NAME} STATIC $<TARGET_OBJECTS:${OBJECT_NAME}>)

target_include_directories(${STATIC_LIB_NAME} PUBLIC ${IM2D_INCLUDE})
set_target_properties(${STATIC_LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if ((BUILD_WITH_LIBDRM STREQUAL true) AND (EXISTS ${LIBDRM_FILE_LIB}/libdrm.so))
    target_link_libraries(${SHARED_LIB_NAME} ${LIBDRM_FILE_LIB}/libdrm.so)
    target_link_libraries(${STATIC_LIB_NAME} ${LIBDRM_FILE_LIB}/libdrm.so)
elseif (BUILD_WITH_LIBDRM STREQUAL true)
    target_link_libraries(${SHARED_LIB_NAME} drm)
    target_link_libraries(${STATIC_LIB_NAME} drm)
endif()

# If cmake version is 3.5.1, FIXIT
if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR lib)
endif()

if (NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
    set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

install(TARGETS ${SHARED_LIB_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS  ${STATIC_LIB_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# build im2d api demo
add_subdirectory(samples/im2d_api_demo)
#add_subdirectory(samples)
