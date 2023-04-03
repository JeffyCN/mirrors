if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DBUILD_TYPE_DEBUG)
endif()

cmake_minimum_required(VERSION 2.8)

PROJECT(rk_stream)

add_compile_options(-DUSE_RAWSTREAM_LIB)

include_directories(
    include
    ${AIQ_DIR}/xcore
    ${AIQ_DIR}/xcore/base
    ${AIQ_DIR}/aiq_core
    ${AIQ_DIR}/common
    ${AIQ_DIR}/common/linux
    ${AIQ_DIR}/include
    ${AIQ_DIR}/include/common
    ${AIQ_DIR}/include/common/mediactl
    ${AIQ_DIR}/include/iq_parser
    ${AIQ_DIR}/include/iq_parser_v2
    ${AIQ_DIR}/include/xcore
    ${AIQ_DIR}/include/xcore/base
    ${AIQ_DIR}/include/algos
    )

set(XCORE_SRC
    ${AIQ_DIR}/xcore/xcam_common.cpp
    ${AIQ_DIR}/xcore/xcam_log.cpp
    ${AIQ_DIR}/xcore/xcam_thread.cpp
    ${AIQ_DIR}/xcore/xcam_buffer.cpp
    ${AIQ_DIR}/xcore/video_buffer.cpp
    ${AIQ_DIR}/xcore/v4l2_buffer_proxy.cpp
    ${AIQ_DIR}/xcore/v4l2_device.cpp
    ${AIQ_DIR}/xcore/fake_v4l2_device.cpp
    ${AIQ_DIR}/xcore/buffer_pool.cpp
    ${AIQ_DIR}/xcore/poll_thread.cpp
    ${AIQ_DIR}/xcore/smart_buffer_priv.cpp
    )

set(STREAMCFG_SRC
    stream_cfg/rk_vi_user_api_stream_cfg.cpp
    stream_cfg/RawStreamCapUnit.cpp
    stream_cfg/RawStreamProcUnit.cpp
    stream_cfg/MediaInfo.cpp
    stream_cfg/Stream.cpp
    ${AIQ_DIR}/common/mediactl/mediactl.c
    )

set(RK_VI_STREAM_CFG_LIB rkrawstream)
set(RK_AIQ_LIB rkaiq)

add_library(${RK_VI_STREAM_CFG_LIB} SHARED ${STREAMCFG_SRC} ${XCORE_SRC} )
target_link_libraries(${RK_VI_STREAM_CFG_LIB} PRIVATE)
install(TARGETS ${RK_VI_STREAM_CFG_LIB} LIBRARY DESTINATION lib)


set (VI_DEMO_SRCS
    rkvi_demo/rkvi_demo.c
    )

cmake_policy(SET CMP0015 NEW)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/all_lib/${CMAKE_BUILD_TYPE})
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/exe/${CMAKE_BUILD_TYPE})
link_directories("${LIBRARY_OUTPUT_PATH}" "${AIQ_DIR}/build/linux/output/aarch64/all_lib/RelWithDebInfo")
add_executable(rkvi_demo ${VI_DEMO_SRCS})
target_link_libraries(rkvi_demo ${RK_VI_STREAM_CFG_LIB} ${RK_AIQ_LIB} pthread)
