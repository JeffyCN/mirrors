if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DBUILD_TYPE_DEBUG)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

PROJECT(rk_aiq)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(C_LIBRARY_NAME "glibc" CACHE INTERNAL "")
include(CheckGlibc)
CHECK_GLIBC_VERSION()

include(BuildOptions)
include(CompileOptions)
include(GNUInstallDirs)

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/rkaiq_config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

find_package(libdl QUIET)
if (libdl_FOUND)
    add_compile_options(${LIBDL_CFLAGS})
endif()

if (RKAIQ_USE_RAWSTREAM_LIB)
    message("USE_RAWSTREAM_LIB")
    add_compile_options(-DUSE_RAWSTREAM_LIB)
endif()

find_package(librga QUIET)
if (librga_FOUND)
    add_compile_options(${LIBRGA_CFLAGS})
endif()

if (RKAIQ_ENABLE_LIBDRM)
find_package(libdrm QUIET)
endif()
if (libdrm_FOUND)
    add_compile_options(${LIBDRM_CFLAGS})
endif()

add_compile_options(${ISP_HW_VERSION})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

if(NOT RKAIQ_ENABLE_SIMULATOR)
    set(GENMESH_ONLINE true)
    if("${ARCH}" STREQUAL "arm")
        set(MOTION_DETECT true)
    endif()
endif()

include_directories(
    xcore
    xcore/base
    aiq_core
    algos
    algos_camgroup
    common
    common/linux
    hwi
    hwi/isp20
    ipc
    iq_parser
    iq_parser_v2
    uAPI
    uAPI/include
    uAPI2
    ipc_server
    ./
    ./common/
    include
    include/common
    include/common/mediactl
    include/iq_parser
    include/iq_parser_v2
    include/uAPI
    include/uAPI2
    include/xcore
    include/xcore/base
    include/algos
    include/algos/a3dlut
    include/algos/ablc
    include/algos/accm
    include/algos/acgc
    include/algos/acp
    include/algos/adebayer
    include/algos/adehaze
    include/algos/adpcc
    include/algos/ae
    include/algos/af
    include/algos/afec
    include/algos/agamma
    include/algos/adegamma
    include/algos/agic
    include/algos/amerge
    include/algos/atmo
    include/algos/adrc
    include/algos/aie
    include/algos/aldch
    include/algos/alsc
    include/algos/anr
    include/algos/anr/tnr_md
    include/algos/aorb
	include/algos/acsm
    include/algos/asd
    include/algos/asharp
    include/algos/awb
    include/algos/awdr
    include/algos/asharp3
    include/algos/aynr2
    include/algos/acnr
    include/algos/aeis
    include/common/gen_mesh
    include/ipc_server
    include/amd
    include/algos/arawnr
    include/algos/aynr
    include/algos/auvnr
    include/algos/amfnr
    include/algos/again
    include/algos/aynr3
    include/algos/acnr2
    include/algos/asharp4
    include/algos/abayer2dnr2
    include/algos/abayertnr2
    include/algos/again2
    include/algos/acac
    include/algos_camgroup/ae
    include/algos_camgroup/awb
    include/algos/abayer2dnrV23
    include/algos/abayertnrV23
    include/algos/aynrV22
    include/algos/acnrV30
    include/algos/asharpV33
    include/algos/ablcV32
    ${PROJECT_BINARY_DIR}
    )

set(ALGO_HANDLE_SRC
    aiq_core/algo_handlers/RkAiqAwbHandle.cpp
    )

if (RKAIQ_HAVE_YNR_V2)
    add_compile_options(-DRKAIQ_HAVE_YNR_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAynrV2Handle.cpp)
endif()

if (RKAIQ_HAVE_AWB_V20)
    add_compile_options(-DRKAIQ_HAVE_AWB_V20=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAwbHandle.cpp)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp)
endif()

if (RKAIQ_HAVE_AWB_V21)
    add_compile_options(-DRKAIQ_HAVE_AWB_V21=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAwbV21Handle.cpp)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp)
endif()

if (RKAIQ_HAVE_AWB_V32)
    add_compile_options(-DRKAIQ_HAVE_AWB_V32=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAwbV32Handle.cpp)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqCustomAwbHandle.cpp)
    if (RKAIQ_HAVE_AWB_V32LT)
        add_compile_options(-DRKAIQ_HAVE_AWB_V32LT=1)
    endif()
endif()

if (RKAIQ_HAVE_GIC_V1 OR RKAIQ_HAVE_GIC_V2)
    if (RKAIQ_HAVE_GIC_V1)
        add_compile_options(-DRKAIQ_HAVE_GIC_V1=1)
    endif()
    if (RKAIQ_HAVE_GIC_V2)
        add_compile_options(-DRKAIQ_HAVE_GIC_V2=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAgicHandle.cpp)
endif()

if (RKAIQ_HAVE_ORB_V1)
    add_compile_options(-DRKAIQ_HAVE_ORB_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAorbHandle.cpp)
endif()

if (RKAIQ_HAVE_EIS_V1)
    add_compile_options(-DRKAIQ_HAVE_EIS_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAeisHandle.cpp)
endif()

if (RKAIQ_HAVE_CAC_V03)
    add_compile_options(-DRKAIQ_HAVE_CAC_V03=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcacV3Handle.cpp)
endif()

if (RKAIQ_HAVE_CAC_V10)
    add_compile_options(-DRKAIQ_HAVE_CAC_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcacHandle.cpp)
endif()

if (RKAIQ_HAVE_CAC_V11)
    add_compile_options(-DRKAIQ_HAVE_CAC_V11=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcacV11Handle.cpp)
endif()

if (RKAIQ_HAVE_YNR_V1)
    add_compile_options(-DRKAIQ_HAVE_YNR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAynrHandle.cpp)
endif()

if (RKAIQ_HAVE_YNR_V3)
    add_compile_options(-DRKAIQ_HAVE_YNR_V3=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAynrV3Handle.cpp)
endif()

if (RKAIQ_HAVE_YNR_V22)
    add_compile_options(-DRKAIQ_HAVE_YNR_V22=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAynrV22Handle.cpp)
endif()

if (RKAIQ_HAVE_UVNR_V1)
    add_compile_options(-DRKAIQ_HAVE_UVNR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcnrHandle.cpp)
endif()

if (RKAIQ_HAVE_CNR_V1)
    add_compile_options(-DRKAIQ_HAVE_CNR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcnrV1Handle.cpp)
endif()

if (RKAIQ_HAVE_CNR_V2)
    add_compile_options(-DRKAIQ_HAVE_CNR_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcnrV2Handle.cpp)
endif()

if (RKAIQ_HAVE_CNR_V30)
    add_compile_options(-DRKAIQ_HAVE_CNR_V30=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcnrV30Handle.cpp)
endif()

if (RKAIQ_HAVE_CNR_V30_LITE)
    add_compile_options(-DRKAIQ_HAVE_CNR_V30_LITE=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcnrV30Handle.cpp)
endif()

if (RKAIQ_HAVE_SHARP_V1)
    add_compile_options(-DRKAIQ_HAVE_SHARP_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsharpHandle.cpp)
endif()

if (RKAIQ_HAVE_SHARP_V3)
    add_compile_options(-DRKAIQ_HAVE_SHARP_V3=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsharpV3Handle.cpp)
endif()

if (RKAIQ_HAVE_SHARP_V4)
    add_compile_options(-DRKAIQ_HAVE_SHARP_V4=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsharpV4Handle.cpp)
endif()

if (RKAIQ_HAVE_SHARP_V33)
    add_compile_options(-DRKAIQ_HAVE_SHARP_V33=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsharpV33Handle.cpp)
endif()

if (RKAIQ_HAVE_SHARP_V33_LITE)
    add_compile_options(-DRKAIQ_HAVE_SHARP_V33_LITE=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsharpV33Handle.cpp)
endif()

if (RKAIQ_HAVE_MFNR_V1)
    add_compile_options(-DRKAIQ_HAVE_MFNR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAmfnrHandle.cpp)
endif()

if (RKAIQ_HAVE_BAYERNR_V1)
    add_compile_options(-DRKAIQ_HAVE_BAYERNR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqArawnrHandle.cpp)
endif()

if (RKAIQ_HAVE_BAYERNR_V2)
    add_compile_options(-DRKAIQ_HAVE_BAYERNR_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqArawnrV2Handle.cpp)
endif()

if (RKAIQ_HAVE_BAYER2DNR_V2)
    add_compile_options(-DRKAIQ_HAVE_BAYER2DNR_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAbayer2dnrV2Handle.cpp)
endif()

if (RKAIQ_HAVE_BAYER2DNR_V23)
    add_compile_options(-DRKAIQ_HAVE_BAYER2DNR_V23=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAbayer2dnrV23Handle.cpp)
endif()

if (RKAIQ_HAVE_BAYERTNR_V2)
    add_compile_options(-DRKAIQ_HAVE_BAYERTNR_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAbayertnrV2Handle.cpp)
endif()

if (RKAIQ_HAVE_BAYERTNR_V23)
    add_compile_options(-DRKAIQ_HAVE_BAYERTNR_V23=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAbayertnrV23Handle.cpp)
endif()

if (RKAIQ_HAVE_BAYERTNR_V23_LITE)
    add_compile_options(-DRKAIQ_HAVE_BAYERTNR_V23_LITE=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAbayertnrV23Handle.cpp)
endif()

if (RKAIQ_HAVE_GAIN_V1)
    add_compile_options(-DRKAIQ_HAVE_GAIN_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAgainHandle.cpp)
endif()

if (RKAIQ_HAVE_GAIN_V2)
    add_compile_options(-DRKAIQ_HAVE_GAIN_V2=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAgainV2Handle.cpp)
endif()

if (RKAIQ_HAVE_BLC_V1)
    add_compile_options(-DRKAIQ_HAVE_BLC_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAblcHandle.cpp)
endif()

if (RKAIQ_HAVE_BLC_V32)
    add_compile_options(-DRKAIQ_HAVE_BLC_V32=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAblcV32Handle.cpp)
endif()

if (RKAIQ_HAVE_ANR_V1)
    add_compile_options(-DRKAIQ_HAVE_ANR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAnrHandle.cpp)
endif()

if (RKAIQ_HAVE_AMD_V1)
    add_compile_options(-DRKAIQ_HAVE_AMD_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAmdHandle.cpp)
endif()

if (RKAIQ_HAVE_AF_V20 OR RKAIQ_HAVE_AF_V30 OR RKAIQ_HAVE_AF_V31 OR RKAIQ_HAVE_AF_V32_LITE)
    add_compile_options(-DRKAIQ_HAVE_AF=1)
    if (RKAIQ_HAVE_AF_V20)
        add_compile_options(-DRKAIQ_HAVE_AF_V20=1)
    endif()
    if (RKAIQ_HAVE_AF_V30)
        add_compile_options(-DRKAIQ_HAVE_AF_V30=1)
    endif()
    if (RKAIQ_HAVE_AF_V31)
        add_compile_options(-DRKAIQ_HAVE_AF_V31=1)
    endif()
    if (RKAIQ_HAVE_AF_V32_LITE)
        add_compile_options(-DRKAIQ_HAVE_AF_V32_LITE=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAfHandle.cpp)
endif()

if (RKAIQ_ONLY_AF_STATS_V20 OR RKAIQ_ONLY_AF_STATS_V30 OR RKAIQ_ONLY_AF_STATS_V31 OR RKAIQ_ONLY_AF_STATS_V32_LITE)
    add_compile_options(-DRKAIQ_HAVE_AF=1)
    if (RKAIQ_ONLY_AF_STATS_V20)
        add_compile_options(-DRKAIQ_ONLY_AF_STATS_V20=1)
    endif()
    if (RKAIQ_ONLY_AF_STATS_V30)
        add_compile_options(-DRKAIQ_ONLY_AF_STATS_V30=1)
    endif()
    if (RKAIQ_ONLY_AF_STATS_V31)
        add_compile_options(-DRKAIQ_ONLY_AF_STATS_V31=1)
    endif()
    if (RKAIQ_ONLY_AF_STATS_V32_LITE)
        add_compile_options(-DRKAIQ_ONLY_AF_STATS_V32_LITE=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAfHandle.cpp)
endif()

if (RKAIQ_HAVE_3DLUT_V1)
    add_compile_options(-DRKAIQ_HAVE_3DLUT_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqA3dlutHandle.cpp)
endif()

if (RKAIQ_HAVE_CCM_V1 OR RKAIQ_HAVE_CCM_V2)
    if (RKAIQ_HAVE_CCM_V1)
        add_compile_options(-DRKAIQ_HAVE_CCM_V1=1)
    endif()
    if (RKAIQ_HAVE_CCM_V2)
        add_compile_options(-DRKAIQ_HAVE_CCM_V2=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAccmHandle.cpp)
endif()

if (RKAIQ_HAVE_GAMMA_V10 OR RKAIQ_HAVE_GAMMA_V11)
    if (RKAIQ_HAVE_GAMMA_V10)
        add_compile_options(-DRKAIQ_HAVE_GAMMA_V10=1)
    endif()
    if (RKAIQ_HAVE_GAMMA_V11)
        add_compile_options(-DRKAIQ_HAVE_GAMMA_V11=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAgammaHandle.cpp)
endif()

if (RKAIQ_HAVE_MERGE_V10 OR RKAIQ_HAVE_MERGE_V11 OR RKAIQ_HAVE_MERGE_V12)
    if (RKAIQ_HAVE_MERGE_V10)
        add_compile_options(-DRKAIQ_HAVE_MERGE_V10=1)
    endif()
    if (RKAIQ_HAVE_MERGE_V11)
        add_compile_options(-DRKAIQ_HAVE_MERGE_V11=1)
    endif()
    if (RKAIQ_HAVE_MERGE_V12)
        add_compile_options(-DRKAIQ_HAVE_MERGE_V12=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAmergeHandle.cpp)
endif()

if (RKAIQ_HAVE_DRC_V10 OR RKAIQ_HAVE_DRC_V11 OR RKAIQ_HAVE_DRC_V12 OR RKAIQ_HAVE_DRC_V12_LITE)
    if (RKAIQ_HAVE_DRC_V10)
        add_compile_options(-DRKAIQ_HAVE_DRC_V10=1)
    endif()
    if (RKAIQ_HAVE_DRC_V11)
        add_compile_options(-DRKAIQ_HAVE_DRC_V11=1)
    endif()
    if (RKAIQ_HAVE_DRC_V12)
        add_compile_options(-DRKAIQ_HAVE_DRC_V12=1)
    endif()
    if (RKAIQ_HAVE_DRC_V12_LITE)
        add_compile_options(-DRKAIQ_HAVE_DRC_V12_LITE=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAdrcHandle.cpp)
endif()

if (RKAIQ_HAVE_DEHAZE_V10 OR RKAIQ_HAVE_DEHAZE_V11 OR RKAIQ_HAVE_DEHAZE_V11_DUO OR RKAIQ_HAVE_DEHAZE_V12)
    if (RKAIQ_HAVE_DEHAZE_V10)
        add_compile_options(-DRKAIQ_HAVE_DEHAZE_V10=1)
    endif()
    if (RKAIQ_HAVE_DEHAZE_V11)
        add_compile_options(-DRKAIQ_HAVE_DEHAZE_V11=1)
    endif()
    if (RKAIQ_HAVE_DEHAZE_V11_DUO)
        add_compile_options(-DRKAIQ_HAVE_DEHAZE_V11_DUO=1)
    endif()
    if (RKAIQ_HAVE_DEHAZE_V12)
        add_compile_options(-DRKAIQ_HAVE_DEHAZE_V12=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAdehazeHandle.cpp)
endif()

if (RKAIQ_HAVE_DEGAMMA_V1)
    add_compile_options(-DRKAIQ_HAVE_DEGAMMA_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAdegammaHandle.cpp)
endif()

if (RKAIQ_HAVE_TMO_V1)
    add_compile_options(-DRKAIQ_HAVE_TMO_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAtmoHandle.cpp)
endif()

if (RKAIQ_HAVE_WDR_V1)
    add_compile_options(-DRKAIQ_HAVE_WDR_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAwdrHandle.cpp)
endif()

if (RKAIQ_HAVE_LDCH_V10)
    add_compile_options(-DRKAIQ_HAVE_LDCH_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAldchHandle.cpp)
endif()

if (RKAIQ_HAVE_LDCH_V21)
    add_compile_options(-DRKAIQ_HAVE_LDCH_V21=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAldchHandle.cpp)
endif()

if (RKAIQ_HAVE_FEC_V10)
    add_compile_options(-DRKAIQ_HAVE_FEC_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAfecHandle.cpp)
endif()

if (RKAIQ_HAVE_CSM_V1)
    add_compile_options(-DRKAIQ_HAVE_CSM_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcsmHandle.cpp)
endif()

if (RKAIQ_HAVE_CGC_V1)
    add_compile_options(-DRKAIQ_HAVE_CGC_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcgcHandle.cpp)
endif()

if (RKAIQ_HAVE_AE_V1)
    add_compile_options(-DRKAIQ_HAVE_AE_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAeHandle.cpp)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqCustomAeHandle.cpp)
endif()

if (RKAIQ_HAVE_DEBAYER_V1 OR RKAIQ_HAVE_DEBAYER_V2 OR RKAIQ_HAVE_DEBAYER_V2_LITE)
    if (RKAIQ_HAVE_DEBAYER_V1)
        add_compile_options(-DRKAIQ_HAVE_DEBAYER_V1=1)
    endif()
    if (RKAIQ_HAVE_DEBAYER_V2)
        add_compile_options(-DRKAIQ_HAVE_DEBAYER_V2=1)
    endif()
    if (RKAIQ_HAVE_DEBAYER_V2_LITE)
        add_compile_options(-DRKAIQ_HAVE_DEBAYER_V2_LITE=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAdebayerHandle.cpp)
endif()

if (RKAIQ_HAVE_LSC_V1 OR RKAIQ_HAVE_LSC_V2 OR RKAIQ_HAVE_LSC_V3)
    if (RKAIQ_HAVE_LSC_V1)
        add_compile_options(-DRKAIQ_HAVE_LSC_V1=1)
    endif()
    if (RKAIQ_HAVE_LSC_V2)
        add_compile_options(-DRKAIQ_HAVE_LSC_V2=1)
    endif()
    if (RKAIQ_HAVE_LSC_V3)
        add_compile_options(-DRKAIQ_HAVE_LSC_V3=1)
    endif()
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAlscHandle.cpp)
endif()

if (RKAIQ_HAVE_DPCC_V1)
    add_compile_options(-DRKAIQ_HAVE_DPCC_V1=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAdpccHandle.cpp)
endif()

if (RKAIQ_HAVE_ACP_V10)
    add_compile_options(-DRKAIQ_HAVE_ACP_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAcpHandle.cpp)
endif()

if (RKAIQ_HAVE_AIE_V10)
    add_compile_options(-DRKAIQ_HAVE_AIE_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAieHandle.cpp)
endif()

if (RKAIQ_HAVE_ASD_V10)
    add_compile_options(-DRKAIQ_HAVE_ASD_V10=1)
    list(APPEND ALGO_HANDLE_SRC aiq_core/algo_handlers/RkAiqAsdHandle.cpp)
endif()

set(AIQ_CORE_SRC
    ${ALGO_HANDLE_SRC}
    aiq_core/RkAiqHandle.cpp
    aiq_core/RkAiqCore.cpp
    aiq_core/MessageBus.cpp
    aiq_core/RkAiqAnalyzeGroupManager.cpp
    aiq_core/RkAiqResourceTranslator.cpp
    aiq_core/RkAiqResourceTranslatorV21.cpp
    aiq_core/RkAiqResourceTranslatorV3x.cpp
    aiq_core/RkAiqResourceTranslatorV32.cpp
    RkAiqCamProfiles.cpp
    )

if (RKAIQ_ISP_VERSION STREQUAL "20")
    list(APPEND AIQ_CORE_SRC aiq_core/RkLumaCore.cpp)
endif()

if (RKAIQ_HAVE_THUMBNAILS)
    add_compile_options(-DRKAIQ_HAVE_THUMBNAILS)
    set(AIQ_CORE_SRC
        ${AIQ_CORE_SRC}
        aiq_core/thumbnails.cpp
        )
endif()

set(ALGO_COMMON_SRC
    algos/interpolation.cpp
    )

if(GENMESH_ONLINE)
    set(COMMON_SRC
        common/mediactl/mediactl.c
        common/gen_mesh/genMesh.cpp
        common/code_to_pixel_format.cpp
        common/media_buffer/media_buffer.c
        common/media_buffer/media_buffer_pool.c
        )
else()
    set(COMMON_SRC
        common/mediactl/mediactl.c
        )
endif()

if(RKAIQ_ENABLE_SIMULATOR)
    set(HWI_SRC
        hwi/CamHwBase.cpp
        hwi/simulator/CamHwSimulator.cpp
        )
else()
    set(HWI_SRC
        hwi/CamHwBase.cpp
        hwi/isp20/CamHwIsp20.cpp
        hwi/isp20/Isp20Params.cpp
        hwi/isp20/Isp20StatsBuffer.cpp
        hwi/isp20/Isp20_module_dbg.cpp
        hwi/isp20/Stream.cpp
        hwi/isp20/RawStreamCapUnit.cpp
        hwi/isp20/RawStreamProcUnit.cpp
        hwi/isp20/PdafStreamProcUnit.cpp
        hwi/isp20/CaptureRawData.cpp
        hwi/isp21/CamHwIsp21.cpp
        hwi/isp3x/CamHwIsp3x.cpp
        hwi/isp32/CamHwIsp32.cpp
        hwi/SensorHw.cpp
        hwi/LensHw.cpp
        hwi/FlashLight.cpp
        hwi/image_processor.cpp
        hwi/isp20/Isp20Evts.cpp
        )
endif()

if (RKAIQ_HAVE_MULTIISP)
    add_compile_options(-DRKAIQ_HAVE_MUTIISP)
    set(HWI_SRC
        ${HWI_SRC}
        hwi/IspParamsSplitter.cpp
        )
endif()

if (RKAIQ_ISP_VERSION STREQUAL "20")
    list(APPEND HWI_SRC
        hwi/isp20/NrStatsStream.cpp
        hwi/isp20/TnrStatsStream.cpp
        hwi/isp20/NrStreamProcUnit.cpp
        hwi/isp20/FecParamStream.cpp
        hwi/isp20/TnrStreamProcUnit.cpp
        )
endif()

if (RKAIQ_HAVE_FAKECAM)
    add_compile_options(-DRKAIQ_ENABLE_FAKECAM)
    list(APPEND HWI_SRC
        hwi/fakecamera/FakeCamHwIsp20.cpp
        hwi/fakecamera/FakeSensorHw.cpp
        )
endif()

if (RKAIQ_HAVE_SPSTREAM)
    add_compile_options(-DRKAIQ_ENABLE_SPSTREAM)
    list(APPEND HWI_SRC
        hwi/isp20/SPStreamProcUnit.cpp
        )
endif()

if(RKAIQ_ENABLE_PARSER_V1)
    add_compile_options(-DRKAIQ_ENABLE_PARSER_V1)
    set(IQ_PARSER_SRC
        iq_parser/RkAiqCalibParser.cpp
        iq_parser/RkAiqCalibDb.cpp
        iq_parser/RkAiqCalibTag.cpp
        iq_parser/tinyxml2.cpp
        iq_parser/xmltags.cpp
        iq_parser/RkAiqCalibApi.cpp
        )
    set(IQ_PARSER_V2_SRC
        iq_parser_v2/awb_xml2json.cpp
        iq_parser_v2/bayernr_xml2json_v1.cpp
        iq_parser_v2/mfnr_xml2json_v1.cpp
        iq_parser_v2/ynr_xml2json_v1.cpp
        iq_parser_v2/uvnr_xml2json_v1.cpp
        iq_parser_v2/sharp_xml2json_v1.cpp
        iq_parser_v2/edgefilter_xml2json_v1.cpp
        )
endif()

set(UAPI_SRC
	#uAPI/rk_aiq_user_api_sysctl.cpp
    uAPI2/rk_aiq_user_api2_sysctl.cpp
    uAPI2/rk_aiq_user_api2_debug.cpp
    uAPI2/rk_aiq_user_api2_helper.cpp
    uAPI2/rk_aiq_user_api2_wrapper.cpp
    )

set(XCORE_SRC
    xcore/xcam_common.cpp
    xcore/xcam_log.cpp
    xcore/xcam_thread.cpp
    xcore/xcam_buffer.cpp
    xcore/video_buffer.cpp
    xcore/v4l2_buffer_proxy.cpp
    xcore/v4l2_device.cpp
    xcore/fake_v4l2_device.cpp
    xcore/buffer_pool.cpp
    xcore/poll_thread.cpp
    xcore/smart_buffer_priv.cpp
    )

if (libdrm_FOUND)
    set(XCORE_SRC
        ${XCORE_SRC}
        xcore/drm_device.cpp
        xcore/drm_buffer.cpp
        xcore/dma_buffer.cpp
        xcore/dma_video_buffer.cpp
        )
endif()

set (IPC_SERVER_SRC
    ipc_server/socket_server.cpp
    ipc_server/rkaiq_tool_ae.cpp
    ipc_server/rkaiq_tool_anr.cpp
    ipc_server/rkaiq_tool_imgproc.cpp
    ipc_server/rkaiq_tool_sharp.cpp
    ipc_server/rkaiq_tool_sysctl.cpp
    ipc_server/command_process.cpp
    ipc_server/MessageParser.cpp
    )

set (INI_READER_SRC
    common/rkaiq_ini.c
    )

set (RK_AIQ_SRC
    ${ALGO_COMMON_SRC}
    ${COMMON_SRC}
    ${HWI_SRC}
    ${IPC_SRC}
    ${IQ_PARSER_SRC}
    ${IQ_PARSER_V2_SRC}
    ${UAPI_SRC}
    ${XCORE_SRC}
    ${AIQ_CORE_SRC}
    ${IPC_SERVER_SRC}
    ${INI_READER_SRC}
    RkAiqManager.cpp
    )

if (RKAIQ_ENABLE_CAMGROUP)
    aux_source_directory(aiq_core/algo_camgroup_handlers ALGO_CAMGROUP_HANDLE_SRC)
    add_compile_options(-DRKAIQ_ENABLE_CAMGROUP)
    set (RK_AIQ_SRC
        ${RK_AIQ_SRC}
        RkAiqCamGroupManager.cpp
	    aiq_core/RkAiqCamgroupHandle.cpp
        ${ALGO_CAMGROUP_HANDLE_SRC}
        )
endif()

set(RK_AIQ_STATIC_LIB_ARS)
set(STATIC_LIB_PREFIX $<TARGET_FILE:)
set(STATIC_LIB_SUFFIX >)

if(GENMESH_ONLINE AND (RKAIQ_HAVE_LDCH_V10 OR RKAIQ_HAVE_LDCH_V21 OR RKAIQ_HAVE_FEC_V10))
    set(GENMESH_LIBS genMeshLib)
    if(ARCH STREQUAL "aarch64")
        set(GENMESH_LIBS_DIR
            ${PROJECT_SOURCE_DIR}/common/gen_mesh/linux/genMesh_static_64bit/
            )
    else()
        set(GENMESH_LIBS_DIR
            ${PROJECT_SOURCE_DIR}/common/gen_mesh/linux/genMesh_static_32bit/
            )
    endif()
    list(APPEND RK_AIQ_STATIC_LIB_ARS ${GENMESH_LIBS_DIR}/libgenMeshLib.a)
    add_compile_options(-DGENMESH_ONLINE)
endif()

if (RKAIQ_HAVE_AMD_V1)
if(MOTION_DETECT)
    set(MOTION_DETECT_LIBS rkaiq_tnr_md)
endif()
endif()

if (RKAIQ_HAVE_PDAF)
    add_compile_options(-DRKAIQ_HAVE_PDAF=1)
endif()
if (RKAIQ_HAVE_PDAF)
    set (PDAF_LIBS rkaiq_pdaf)
	list(APPEND RK_AIQ_STATIC_LIB_ARS
		${STATIC_LIB_PREFIX}rkaiq_pdaf${STATIC_LIB_SUFFIX})
endif()

# set output binary path
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


add_subdirectory(iq_parser_v2)
include_directories(include/iq_parser_v2)
include_directories(include/iq_parser_v2/j2s)

add_subdirectory(algos)
if (RKAIQ_ENABLE_CAMGROUP)
    add_subdirectory(algos_camgroup)
endif()

cmake_policy(SET CMP0015 NEW)

link_directories("${LIBRARY_OUTPUT_PATH}" "${OPENCV_LIBS_DIR}" "${GENMESH_LIBS_DIR}")

add_library(rkaiq_tmp STATIC ${RK_AIQ_SRC})
set_target_properties(rkaiq_tmp PROPERTIES FOLDER "rk_aiq")
set_target_properties(rkaiq_tmp PROPERTIES CLEAN_DIRECT_OUTPUT 1)
add_dependencies(rkaiq_tmp j2s_parser)

if (RKAIQ_BUILD_BINARY_IQ)
    add_dependencies(rkaiq_tmp j2s4b)
endif()

add_library(rkaiq SHARED ${RK_AIQ_SRC})
set_target_properties(rkaiq PROPERTIES FOLDER "rk_aiq")
set_target_properties(rkaiq PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(rkaiq PROPERTIES C_VISIBILITY_PRESET default)
set_target_properties(rkaiq PROPERTIES CXX_VISIBILITY_PRESET default)

target_link_libraries(rkaiq
	PRIVATE
    rkaiq_tmp
    rkaiq_ae
    rkaiq_awb
    ${OPENCV_LIBS}
    ${OPENCV_3RDPARTY_LIBS}
    ${GENMESH_LIBS}
    ${MOTION_DETECT_LIBS}
    pthread)

list(APPEND RK_AIQ_STATIC_LIB_ARS
    ${STATIC_LIB_PREFIX}rkaiq_tmp${STATIC_LIB_SUFFIX}
    ${STATIC_LIB_PREFIX}rkaiq_ae${STATIC_LIB_SUFFIX}
    ${STATIC_LIB_PREFIX}rkaiq_awb${STATIC_LIB_SUFFIX}
    )

if (RKAIQ_HAVE_YNR_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aynr_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aynr_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_GIC_V1 OR RKAIQ_HAVE_GIC_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_agic
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_agic${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_ORB_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aorb
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aorb${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_EIS_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aeis
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aeis${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CAC_V03 OR RKAIQ_HAVE_CAC_V10 OR RKAIQ_HAVE_CAC_V11)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acac
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acac${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_YNR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aynr_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aynr_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_YNR_V3)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aynr_v3
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aynr_v3${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_YNR_V22)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aynr_v22
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aynr_v22${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_UVNR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_auvnr_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_auvnr_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CNR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acnr_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acnr_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CNR_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acnr_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acnr_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CNR_V30 OR RKAIQ_HAVE_CNR_V30_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acnr_v30
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acnr_v30${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_SHARP_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_asharp
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_asharp${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_SHARP_V3)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_asharp_v3
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_asharp_v3${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_SHARP_V4)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_asharp_v4
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_asharp_v4${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_SHARP_V33 OR RKAIQ_HAVE_SHARP_V33_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_asharp_v33
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_asharp_v33${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_MFNR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_amfnr_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_amfnr_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYERNR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_arawnr_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_arawnr_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYERNR_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_arawnr_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_arawnr_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYER2DNR_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_abayer2dnr_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_abayer2dnr_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYER2DNR_V23)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_abayer2dnr_v23
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_abayer2dnr_v23${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYERTNR_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_abayertnr_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_abayertnr_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BAYERTNR_V23 OR RKAIQ_HAVE_BAYERTNR_V23_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_abayertnr_v23
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_abayertnr_v23${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_GAIN_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_again_v1
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_again_v1${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_GAIN_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_again_v2
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_again_v2${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BLC_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_ablc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_ablc${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_BLC_V32)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_ablc_v32
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_ablc_v32${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_ANR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_anr
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_anr${STATIC_LIB_SUFFIX})
endif()


if (RKAIQ_HAVE_AMD_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_amd
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_amd${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_AF_V20 OR RKAIQ_HAVE_AF_V30 OR RKAIQ_HAVE_AF_V31 OR RKAIQ_HAVE_AF_V32_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_af
    ${PDAF_LIBS}
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_af${STATIC_LIB_SUFFIX})
elseif (RKAIQ_ONLY_AF_STATS_V20 OR RKAIQ_ONLY_AF_STATS_V30 OR RKAIQ_ONLY_AF_STATS_V31 OR RKAIQ_ONLY_AF_STATS_V32_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_thirdaf
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_thirdaf${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_3DLUT_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_a3dlut
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_a3dlut${STATIC_LIB_SUFFIX})
endif()
if (RKAIQ_HAVE_CCM_V1 OR RKAIQ_HAVE_CCM_V2)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_accm
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_accm${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_GAMMA_V10 OR RKAIQ_HAVE_GAMMA_V11)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_agamma
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_agamma${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_MERGE_V10 OR RKAIQ_HAVE_MERGE_V11 OR RKAIQ_HAVE_MERGE_V12)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_amerge
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_amerge${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_DRC_V10 OR RKAIQ_HAVE_DRC_V11 OR RKAIQ_HAVE_DRC_V12 OR RKAIQ_HAVE_DRC_V12_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_adrc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_adrc${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_DEHAZE_V10 OR RKAIQ_HAVE_DEHAZE_V11 OR RKAIQ_HAVE_DEHAZE_V11_DUO OR RKAIQ_HAVE_DEHAZE_V12)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_adehaze
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_adehaze${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_DEGAMMA_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_adegamma
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_adegamma${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_TMO_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_atmo
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_atmo${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_WDR_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_awdr
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_awdr${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_LDCH_V10 OR RKAIQ_HAVE_LDCH_V21)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aldch
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aldch${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_FEC_V10)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_afec
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_afec${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CSM_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acsm
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acsm${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_CGC_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acgc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acgc${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_ENABLE_CAMGROUP)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_camgroup_ae
    rkaiq_camgroup_awb
    rkaiq_camgroup_misc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS
    ${STATIC_LIB_PREFIX}rkaiq_camgroup_ae${STATIC_LIB_SUFFIX}
    ${STATIC_LIB_PREFIX}rkaiq_camgroup_awb${STATIC_LIB_SUFFIX}
    ${STATIC_LIB_PREFIX}rkaiq_camgroup_misc${STATIC_LIB_SUFFIX}
    )
endif()

if (RKAIQ_HAVE_DEBAYER_V1 OR RKAIQ_HAVE_DEBAYER_V2 OR RKAIQ_HAVE_DEBAYER_V2_LITE)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_adebayer
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_adebayer${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_LSC_V1 OR RKAIQ_HAVE_LSC_V2 OR RKAIQ_HAVE_LSC_V3)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_alsc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_alsc${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_DPCC_V1)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_adpcc
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_adpcc${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_AIE_V10)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_aie
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_aie${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_ACP_V10)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_acp
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_acp${STATIC_LIB_SUFFIX})
endif()

if (RKAIQ_HAVE_ASD_V10)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_asd
    )
list(APPEND RK_AIQ_STATIC_LIB_ARS ${STATIC_LIB_PREFIX}rkaiq_asd${STATIC_LIB_SUFFIX})
endif()

if (libdl_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        libdl::libdl
        )
endif()

if (libdrm_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        libdrm::libdrm
        )
endif()

#build static librkaiq.a

foreach(STATIC_LIB_AR ${RK_AIQ_STATIC_LIB_ARS})
	add_custom_command (TARGET rkaiq_tmp POST_BUILD
	COMMAND ${CMAKE_AR} x ${STATIC_LIB_AR}
	)
endforeach()

add_custom_command (TARGET rkaiq_tmp POST_BUILD
	COMMAND ${CMAKE_AR} rcs ${CMAKE_LIBRARY_PATH}/librkaiq.a *.o
	COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_PATH}/librkaiq.a
	COMMAND rm *.o
	)

set(RKAIQ_HEAD_DEPENDENCE
    include/
    include/algos/
    include/uAPI2/
    include/xcore/
    include/common/
    include/iq_parser/
    include/iq_parser_v2/)
target_include_directories(rkaiq PUBLIC ${RKAIQ_HEAD_DEPENDENCE})


install(DIRECTORY include/
    DESTINATION include/rkaiq
    FILES_MATCHING PATTERN "*.h")
install(FILES ${PROJECT_BINARY_DIR}/config.h
    DESTINATION include/rkaiq)
install(TARGETS rkaiq
    LIBRARY DESTINATION lib)
install(FILES ${CMAKE_LIBRARY_PATH}/librkaiq.a DESTINATION lib)
