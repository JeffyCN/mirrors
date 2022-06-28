if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DBUILD_TYPE_DEBUG)
endif()

cmake_minimum_required(VERSION 2.8)

PROJECT(rk_aiq)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildOptions)
include(CompileOptions)
include(GNUInstallDirs)

find_package(libdl QUIET)
if (LIBDL_FOUND)
    add_compile_options(${LIBDL_CFLAGS})
endif()

find_package(librga QUIET)
if (LIBRGA_FOUND)
    add_compile_options(${LIBRGA_CFLAGS})
endif()

find_package(libdrm QUIET)
if (LIBDRM_FOUND)
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
    )

aux_source_directory(aiq_core/algo_handlers ALGO_HANDLE_SRC)
set(AIQ_CORE_SRC
    ${ALGO_HANDLE_SRC}
    aiq_core/RkAiqHandle.cpp
    aiq_core/RkAiqCore.cpp
    aiq_core/RkLumaCore.cpp
    aiq_core/MessageBus.cpp
    aiq_core/RkAiqAnalyzeGroupManager.cpp
    aiq_core/RkAiqResourceTranslator.cpp
    aiq_core/RkAiqResourceTranslatorV21.cpp
    aiq_core/RkAiqResourceTranslatorV3x.cpp
    aiq_core/thumbnails.cpp
    )

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
        hwi/isp20/SPStreamProcUnit.cpp
        hwi/isp20/PdafStreamProcUnit.cpp
        hwi/isp20/NrStatsStream.cpp
        hwi/isp20/TnrStatsStream.cpp
        hwi/isp20/NrStreamProcUnit.cpp
        hwi/isp20/FecParamStream.cpp
        hwi/isp20/TnrStreamProcUnit.cpp
        hwi/isp20/CaptureRawData.cpp
        hwi/isp21/CamHwIsp21.cpp
        hwi/isp3x/CamHwIsp3x.cpp
        hwi/fakecamera/FakeCamHwIsp20.cpp
        hwi/fakecamera/FakeSensorHw.cpp
        hwi/SensorHw.cpp
        hwi/LensHw.cpp
        hwi/FlashLight.cpp
        hwi/image_processor.cpp
        hwi/isp20/Isp20Evts.cpp
        hwi/IspParamsSplitter.cpp
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

if (LIBDRM_FOUND)
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

if(OPENCV_SUPPORT)
    set(OPENCV_LIBS
        opencv_calib3d
        opencv_core
        opencv_dnn
        opencv_features2d
        opencv_flann
        opencv_gapi
        opencv_highgui
        opencv_imgcodecs
        opencv_imgproc
        opencv_ml
        opencv_objdetect
        opencv_photo
        opencv_stitching
        opencv_video
        opencv_videoio
        )
    set(OPENCV_3RDPARTY_LIBS
        dl
        ade
        IlmImf
        ippicv
        ippiw
        ittnotify
        libprotobuf
        libwebp
        quirc
        )
    set(OPENCV_LIBS_DIR
        ${PROJECT_SOURCE_DIR}/common/opencv2/lib
        ${PROJECT_SOURCE_DIR}/common/opencv2/lib/3rdparty
        )
    add_compile_options(-DOPENCV_SUPPORT)
endif()

if(GENMESH_ONLINE)
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
    add_compile_options(-DGENMESH_ONLINE)
endif()

if(MOTION_DETECT)
    set(MOTION_DETECT_LIBS rkaiq_tnr_md)
endif()

set(PDAF_SUPPORT true)
if(PDAF_SUPPORT)
    set (PDAF_LIBS rkaiq_pdaf)
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

add_subdirectory(rkaiq_3A_server)

add_subdirectory(tests/rkisp_demo)

cmake_policy(SET CMP0015 NEW)

link_directories("${LIBRARY_OUTPUT_PATH}" "${OPENCV_LIBS_DIR}" "${GENMESH_LIBS_DIR}")

add_library(rkaiq SHARED ${RK_AIQ_SRC})
set_target_properties(rkaiq PROPERTIES FOLDER "rk_aiq")
set_target_properties(rkaiq PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(rkaiq PROPERTIES C_VISIBILITY_PRESET default)
set_target_properties(rkaiq PROPERTIES CXX_VISIBILITY_PRESET default)
set_target_properties(rkaiq PROPERTIES LINK_FLAGS -Wl,--version-script,${CMAKE_CURRENT_LIST_DIR}/export.map)
add_dependencies(rkaiq j2s_parser)

target_link_libraries(rkaiq
    PRIVATE
    rkaiq_ae
    rkaiq_awb
    rkaiq_af
    rkaiq_anr
    rkaiq_asd
    rkaiq_amerge
    rkaiq_atmo
    rkaiq_acp
    rkaiq_asharp
    rkaiq_adehaze
    rkaiq_a3dlut
    rkaiq_ablc
    rkaiq_accm
    rkaiq_acgc
    rkaiq_adebayer
    rkaiq_adpcc
    rkaiq_afec
    rkaiq_agamma
    rkaiq_adegamma
    rkaiq_agic
    rkaiq_aie
    rkaiq_aldch
    rkaiq_alsc
    rkaiq_aorb
	rkaiq_acsm
    rkaiq_awdr
    rkaiq_adrc
    rkaiq_asharp_v3
    rkaiq_aynr_v2
    rkaiq_acnr_v1
    rkaiq_arawnr_v2
    rkaiq_aeis
    rkaiq_amd
    rkaiq_arawnr_v1
    rkaiq_aynr_v1
    rkaiq_auvnr_v1
    rkaiq_amfnr_v1
    rkaiq_again_v1
    rkaiq_aynr_v3
    rkaiq_acnr_v2
    rkaiq_asharp_v4
    rkaiq_abayer2dnr_v2
    rkaiq_abayertnr_v2
    rkaiq_again_v2
    rkaiq_acac
    ${OPENCV_LIBS}
    ${OPENCV_3RDPARTY_LIBS}
    ${GENMESH_LIBS}
    ${MOTION_DETECT_LIBS}
    ${PDAF_LIBS}
    pthread)

if (RKAIQ_ENABLE_CAMGROUP)
target_link_libraries(rkaiq
    PRIVATE
    rkaiq_camgroup_ae
    rkaiq_camgroup_awb
    rkaiq_camgroup_misc
    )
endif()

if (LIBDL_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        libdl::libdl
        )
endif()

if (LIBDRM_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        libdrm::libdrm
        )
endif()

set (VERSIONFILEPATH ${PROJECT_SOURCE_DIR}/)
execute_process(COMMAND sh
    ${PROJECT_SOURCE_DIR}/tools/check-version-with-tuningtools.sh
    ${VERSIONFILEPATH})

install(DIRECTORY include/ DESTINATION include/rkaiq
    FILES_MATCHING PATTERN "*.h")
install(DIRECTORY common/ DESTINATION include/rkaiq/common
    FILES_MATCHING PATTERN "*.h")
install(TARGETS rkaiq
    LIBRARY DESTINATION lib)
