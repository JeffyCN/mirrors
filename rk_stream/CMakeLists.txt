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
    ${AIQ_DIR}/algos
    ${AIQ_DIR}/include/common
    ${AIQ_DIR}/include/common/mediactl
    ${AIQ_DIR}/include/iq_parser
    ${AIQ_DIR}/include/iq_parser_v2
    ${AIQ_DIR}/include/xcore
    ${AIQ_DIR}/include/xcore/base
    ${AIQ_DIR}/include/algos
    ${AIQ_DIR}/include/algo_types
    ${AIQ_DIR}/include/isp
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
    stream_cfg/IspStreamCapUnit.cpp
    stream_cfg/RawStreamProcUnit.cpp
    stream_cfg/MediaInfo.cpp
    stream_cfg/Stream.cpp
    ${AIQ_DIR}/common/mediactl/mediactl.c
    )

set(RK_VI_STREAM_CFG_LIB rkrawstream)
add_library(${RK_VI_STREAM_CFG_LIB} SHARED ${STREAMCFG_SRC} ${XCORE_SRC} )
target_link_libraries(${RK_VI_STREAM_CFG_LIB} PRIVATE)
install(TARGETS ${RK_VI_STREAM_CFG_LIB} LIBRARY DESTINATION lib)


if(RKVIDEMO_HAVE_AIQ)
    set(RK_AIQ_LIB rkaiq)
    add_compile_options(-DHAVE_RKAIQ)
endif()
if(RKVIDEMO_HAVE_RGA)
    set(RK_RGA_LIB rga)
    add_compile_options(-DHAVE_RGA)
endif()
if(RKVIDEMO_HAVE_ROCKIT)
    set(RK_ROCKIT_LIB rockit)
    add_compile_options(-DHAVE_ROCKIT)
endif()

message("\n****** rkvi_demo build with: ${RK_VI_STREAM_CFG_LIB} ${RK_AIQ_LIB} ${RK_RGA_LIB} ${RK_ROCKIT_LIB}\n")

set (VI_DEMO_SRCS
    rkvi_demo/rkrawstream_demo.c
    )

set (RKAIQ_TEST_SRCS
    rkaiq_test/rkaiq_test.c
    rkaiq_test/read_reg.c
    rkaiq_test/casefile.c
    rkaiq_test/utils/sds.c
    rkaiq_test/utils/sfmalloc.c
    rkaiq_test/utils/debug_log.c
    rkaiq_test/utils/smartstruct.c
    rkaiq_test/utils/implementation.c
    rkaiq_test/ispsim/testcase.c
    rkaiq_test/ispsim/testargs.c
    rkaiq_test/ispsim/image2d.c
    rkaiq_test/test_cases/test_case.c
    )

cmake_policy(SET CMP0015 NEW)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/all_lib/${CMAKE_BUILD_TYPE})
link_directories("${LIBRARY_OUTPUT_PATH}")

if(RKRAWSTREAM_DEMO)
add_executable(rkrawstream_demo ${VI_DEMO_SRCS})
target_link_libraries(rkrawstream_demo rkrawstream pthread)
endif()

if(RKAIQ_TEST)
include_directories(rkaiq_test rkaiq_test/utils rkaiq_test/ispsim)
link_directories("${AIQ_DIR}/build/linux/output/arm/all_lib/MinSizeRel")
add_executable(rkaiq_test ${RKAIQ_TEST_SRCS})
target_link_libraries(rkaiq_test rkrawstream rkaiq pthread)

set (RKISP_SIM_SRCS
    rkaiq_test/ispsim/isp32.c
    rkaiq_test/ispsim/btnr32.c
    rkaiq_test/ispsim/sharp32.c
    rkaiq_test/ispsim/image2d.c
    rkaiq_test/ispsim/main.c
    rkaiq_test/ispsim/testcase.c
    rkaiq_test/ispsim/testargs.c
    rkaiq_test/ispsim/testlist.c
    rkaiq_test/utils/sds.c
    rkaiq_test/utils/sfmalloc.c
    rkaiq_test/utils/debug_log.c
    rkaiq_test/utils/smartstruct.c
    rkaiq_test/utils/implementation.c
    )
add_executable(rkisp_sim ${RKISP_SIM_SRCS})
endif()
