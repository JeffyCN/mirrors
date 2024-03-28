if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

PROJECT(media_enquiry)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildOptions)
include(CompileOptions)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

set(ADD_RK_AIQ true)
option(ENABLE_IIO_API "Enable IIO API for EIS algorithm" OFF)

if(EXISTS "${PROJECT_SOURCE_DIR}/../rk_aiq")
    set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rk_aiq)
    message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})
else ()
    set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rkaiq)
    message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})
endif (EXISTS "${PROJECT_SOURCE_DIR}/../rk_aiq")

if(ADD_RK_AIQ)
    add_compile_options(-Os)
    add_compile_options(-DADD_RK_AIQ)
    if(NOT ARCH STREQUAL "fpga")
        add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)
    else ()
        add_compile_options(-DARCH_FPGA)
    endif()
    set(RK_AIQ_LIB_DIR ${RK_AIQ_SOURCE_DIR}/build/linux/output/${ARCH}/all_lib/${CMAKE_BUILD_TYPE})
    message("rkaiq lib path" ${RK_AIQ_LIB_DIR})
    set(RK_AIQ_LIB rkaiq)
endif()

if (ARCH STREQUAL "aarch64")
    set(RK_DRM_LIB_DIR ${PROJECT_SOURCE_DIR}/media_enquiry/libs/arm64/)
elseif (ARCH STREQUAL "arm")
    set(RK_DRM_LIB_DIR
        ${PROJECT_SOURCE_DIR}/media_enquiry/libs/arm32/
        )
endif()

SET(RK_DRM_LIB drm)
if(NOT ARCH STREQUAL "fpga")
    set(RK_RGA_LIB rga)
endif()

if (ENABLE_IIO_API)
    set(IIO_LIB_DIR ${PROJECT_SOURCE_DIR}/media_enquiry/iio/lib)
    set(IIO_LIBS easyiio iio serialport xml2 )
endif()

include_directories(
    media_enquiry
    media_enquiry/mediactl
    ${RK_AIQ_SOURCE_DIR}/aiq_core
    ${RK_AIQ_SOURCE_DIR}/algos
    ${RK_AIQ_SOURCE_DIR}/algos_camgroup
    ${RK_AIQ_SOURCE_DIR}/common
    ${RK_AIQ_SOURCE_DIR}/common/linux
    ${RK_AIQ_SOURCE_DIR}/common/mediactl
    ${RK_AIQ_SOURCE_DIR}/
    ${RK_AIQ_SOURCE_DIR}/hwi/isp20
    ${RK_AIQ_SOURCE_DIR}/ipc
    ${RK_AIQ_SOURCE_DIR}/iq_parser
    ${RK_AIQ_SOURCE_DIR}/iq_parser_v2
    ${RK_AIQ_SOURCE_DIR}/uAPI
    ${RK_AIQ_SOURCE_DIR}/uAPI/include
    ${RK_AIQ_SOURCE_DIR}/uAPI2
    ${RK_AIQ_SOURCE_DIR}/ipc_server
    ${RK_AIQ_SOURCE_DIR}/./
    ${RK_AIQ_SOURCE_DIR}/xcore/
    ${RK_AIQ_SOURCE_DIR}/xcore/base
    ${RK_AIQ_SOURCE_DIR}/include
    ${RK_AIQ_SOURCE_DIR}/include/xcore
    ${RK_AIQ_SOURCE_DIR}/include/xcore/base
    ${RK_AIQ_SOURCE_DIR}/include/iq_parser
    ${RK_AIQ_SOURCE_DIR}/include/iq_parser_v2
    ${RK_AIQ_SOURCE_DIR}/include/uAPI
    ${RK_AIQ_SOURCE_DIR}/include/uAPI2
    ${RK_AIQ_SOURCE_DIR}/include/algos
    ${RK_AIQ_SOURCE_DIR}/include/algos/a3dlut
    ${RK_AIQ_SOURCE_DIR}/include/algos/ablc
    ${RK_AIQ_SOURCE_DIR}/include/algos/accm
    ${RK_AIQ_SOURCE_DIR}/include/algos/acgc
    ${RK_AIQ_SOURCE_DIR}/include/algos/acp
    ${RK_AIQ_SOURCE_DIR}/include/algos/adebayer
    ${RK_AIQ_SOURCE_DIR}/include/algos/adehaze
    ${RK_AIQ_SOURCE_DIR}/include/algos/adpcc
    ${RK_AIQ_SOURCE_DIR}/include/algos/ae
    ${RK_AIQ_SOURCE_DIR}/include/algos/af
    ${RK_AIQ_SOURCE_DIR}/include/algos/afec
    ${RK_AIQ_SOURCE_DIR}/include/algos/agamma
    ${RK_AIQ_SOURCE_DIR}/include/algos/adegamma
    ${RK_AIQ_SOURCE_DIR}/include/algos/agic
    ${RK_AIQ_SOURCE_DIR}/include/algos/amerge
    ${RK_AIQ_SOURCE_DIR}/include/algos/atmo
    ${RK_AIQ_SOURCE_DIR}/include/algos/adrc
    ${RK_AIQ_SOURCE_DIR}/include/algos/aie
    ${RK_AIQ_SOURCE_DIR}/include/algos/aldch
    ${RK_AIQ_SOURCE_DIR}/include/algos/alsc
    ${RK_AIQ_SOURCE_DIR}/include/algos/anr
    ${RK_AIQ_SOURCE_DIR}/include/algos/anr/tnr_md
    ${RK_AIQ_SOURCE_DIR}/include/algos/aorb
	${RK_AIQ_SOURCE_DIR}/include/algos/acsm
    ${RK_AIQ_SOURCE_DIR}/include/algos/asd
    ${RK_AIQ_SOURCE_DIR}/include/algos/asharp
    ${RK_AIQ_SOURCE_DIR}/include/algos/awb
    ${RK_AIQ_SOURCE_DIR}/include/algos/awdr
    ${RK_AIQ_SOURCE_DIR}/include/algos/asharp3
    ${RK_AIQ_SOURCE_DIR}/include/algos/aynr2
    ${RK_AIQ_SOURCE_DIR}/include/algos/acnr
    ${RK_AIQ_SOURCE_DIR}/include/algos/aeis
    ${RK_AIQ_SOURCE_DIR}/include/common
    ${RK_AIQ_SOURCE_DIR}/include/common/mediactl
    ${RK_AIQ_SOURCE_DIR}/include/common/gen_mesh
    ${RK_AIQ_SOURCE_DIR}/include/ipc_server
    ${RK_AIQ_SOURCE_DIR}/include/amd
    ${RK_AIQ_SOURCE_DIR}/include/algos/arawnr
    ${RK_AIQ_SOURCE_DIR}/include/algos/aynr
    ${RK_AIQ_SOURCE_DIR}/include/algos/auvnr
    ${RK_AIQ_SOURCE_DIR}/include/algos/amfnr
    ${RK_AIQ_SOURCE_DIR}/include/algos/again
    ${RK_AIQ_SOURCE_DIR}/include/algos/aynr3
    ${RK_AIQ_SOURCE_DIR}/include/algos/acnr2
    ${RK_AIQ_SOURCE_DIR}/include/algos/asharp4
    ${RK_AIQ_SOURCE_DIR}/include/algos/abayer2dnr2
    ${RK_AIQ_SOURCE_DIR}/include/algos/abayertnr2
    ${RK_AIQ_SOURCE_DIR}/include/algos/again2
    ${RK_AIQ_SOURCE_DIR}/include/algos/acac
    ${RK_AIQ_SOURCE_DIR}/include/algos_camgroup/ae
    ${RK_AIQ_SOURCE_DIR}/include/algos_camgroup/awb
    ${RK_AIQ_SOURCE_DIR}/include/isp
    )

set(DIR_SRCS
    media_enquiry/media_enquiry.cpp
    ${RK_AIQ_SOURCE_DIR}/common/mediactl/mediactl.c
    media_enquiry/mediactl/v4l2subdev.c
    )

set(XCORE_SRC
    ${RK_AIQ_SOURCE_DIR}/xcore/xcam_common.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/xcam_log.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/xcam_buffer.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/video_buffer.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/v4l2_buffer_proxy.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/v4l2_device.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/fake_v4l2_device.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/buffer_pool.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/poll_thread.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/smart_buffer_priv.cpp
    ${RK_AIQ_SOURCE_DIR}/xcore/xcam_thread.cpp
    )

if(WIN32)
    SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/exe)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/all_lib/)
    SET(CMAKE_LIBRARY_PATH ${LIBRARY_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
else()
    SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/exe/${CMAKE_BUILD_TYPE})
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/all_lib/${CMAKE_BUILD_TYPE})
    SET(CMAKE_LIBRARY_PATH ${LIBRARY_OUTPUT_PATH})
endif()

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
message(${CMAKE_LIBRARY_PATH})
message(${LIBRARY_OUTPUT_PATH})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(media_enquiry ${DIR_SRCS} ${XCORE_SRC})
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    target_link_libraries(media_enquiry pthread)
endif()
