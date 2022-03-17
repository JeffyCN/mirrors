
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DBUILD_TYPE_DEBUG)
endif()

cmake_minimum_required(VERSION 2.8)

#project name
PROJECT(rk_aiq)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

# ----------------------------------------------------------------------------
# set property to classify library kinds
# ----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#set BUILROOT_BUILD_PROJECT
set(BUILROOT_BUILD_PROJECT true)
#set(ENABLE_ASAN true)

#set RK_SIMULATOR_HW
#set(RK_SIMULATOR_HW true)

if(NOT RK_SIMULATOR_HW)
set(GENMESH_ONLINE true)
if("${ARCH}" STREQUAL "arm")
    set(MOTION_DETECT true)
endif()
endif()

# ----------------------------------------------------------------------------
# SET THE COMPILER PATH
# ----------------------------------------------------------------------------
if(RK_SIMULATOR_HW)
	add_compile_options(-DRK_SIMULATOR_HW)
elseif(BUILROOT_BUILD_PROJECT)
	#add_compile_options(-DDISABLE_PP)
	#add_compile_options(-DDISABLE_PP_STATS)
else()
    SET(CMAKE_C_COMPILER "/home/hkj/isp2-fpga/fpga-ramdisk/toolchains/host/bin/arm-buildroot-linux-gnueabi-gcc")
    SET(CMAKE_CXX_COMPILER "/home/hkj/isp2-fpga/fpga-ramdisk/toolchains/host/bin/arm-buildroot-linux-gnueabi-g++")
endif()

# ----------------------------------------------------------------------------
# ADD SYMBOL TABLE FOR GDB
# ----------------------------------------------------------------------------
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -std=c++11 -Wno-reorder -Werror -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -std=c++11 -Werror -Wno-reorder")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#head file path
include_directories(
xcore
xcore/base
aiq_core
algos
common
common/linux
hwi
hwi/isp20
ipc
iq_parser
iq_parser_v2
uAPI
uAPI2
ipc_server
#algos/awb
#../core/inc/luma
#../core/inc/stat_3a_ae
#../core/inc/stat_3a_af
#../core/inc/orb
#../core/inc/common
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
include/algos/ar2y
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
)
#source directory
#AUX_SOURCE_DIRECTORY(demo/src DIR_SRCS)
AUX_SOURCE_DIRECTORY(aiq_core/algo_handlers ALGO_HANDLE_SRC)
set (AIQ_CORE_SRC
	aiq_core/RkAiqHandle.cpp
    ${ALGO_HANDLE_SRC}
	aiq_core/RkAiqHandleInt.cpp
	aiq_core/RkAiqHandleIntV21.cpp
	aiq_core/RkAiqCore.cpp
	aiq_core/RkAiqCoreV21.cpp
    aiq_core/RkLumaCore.cpp
    aiq_core/RkAiqSharedDataManager.cpp
    aiq_core/MessageBus.cpp
    aiq_core/RkAiqAnalyzeGroupManager.cpp
    aiq_core/RkAiqResourceTranslator.cpp
    aiq_core/RkAiqResourceTranslatorV21.cpp
    aiq_core/thumbnails.cpp
	)
set (ALGO_COMMON_SRC
	algos/interpolation.cpp
	)
if(GENMESH_ONLINE)
set (COMMON_SRC
    common/mediactl/mediactl.c
    common/gen_mesh/genMesh.cpp
    common/shared_data_api_wrapper.cpp
    common/code_to_pixel_format.cpp
    common/media_buffer/media_buffer.c
    common/media_buffer/media_buffer_pool.c
    )
else()
set (COMMON_SRC
	common/mediactl/mediactl.c
	)
endif()
if(RK_SIMULATOR_HW)
set (HWI_SRC
        hwi/CamHwBase.cpp
        hwi/simulator/CamHwSimulator.cpp
        )
else()
set (HWI_SRC
        hwi/CamHwBase.cpp
        hwi/isp20/CamHwIsp20.cpp
        hwi/isp20/Isp20Params.cpp
        hwi/isp20/Isp20StatsBuffer.cpp
        hwi/isp20/Isp20_module_dbg.cpp
        hwi/isp20/Stream.cpp
        hwi/isp20/RawStreamCapUnit.cpp
        hwi/isp20/RawStreamProcUnit.cpp
        hwi/isp20/SPStreamProcUnit.cpp
		hwi/isp20/NrStatsStream.cpp
		hwi/isp20/TnrStatsStream.cpp
		hwi/isp20/NrStreamProcUnit.cpp
		hwi/isp20/FecParamStream.cpp
		hwi/isp20/TnrStreamProcUnit.cpp
		hwi/isp20/CaptureRawData.cpp
        hwi/isp21/CamHwIsp21.cpp
        hwi/SensorHw.cpp
        hwi/LensHw.cpp
        hwi/FlashLight.cpp
        hwi/image_processor.cpp
        hwi/isp20/Isp20Evts.cpp
        )
endif()
set (IPC_SRC
	)
set (IQ_PARSER_SRC
	iq_parser/RkAiqCalibParser.cpp
	iq_parser/RkAiqCalibDb.cpp
	iq_parser/RkAiqCalibTag.cpp
	iq_parser/tinyxml2.cpp
	iq_parser/xmltags.cpp
	iq_parser/RkAiqCalibApi.cpp
	)
set (IQ_PARSER_V2_SRC
	iq_parser_v2/awb_xml2json.cpp
	)
set (IQ_PARSER_V2_SRC
	iq_parser_v2/bayernr_xml2json_v1.cpp
	iq_parser_v2/mfnr_xml2json_v1.cpp
	iq_parser_v2/ynr_xml2json_v1.cpp
	iq_parser_v2/uvnr_xml2json_v1.cpp
	iq_parser_v2/sharp_xml2json_v1.cpp
	iq_parser_v2/edgefilter_xml2json_v1.cpp
	)
set (UAPI_SRC
    uAPI/rk_aiq_user_api_sysctl.cpp
    uAPI2/rk_aiq_user_api2_sysctl.cpp
    uAPI/rk_aiq_user_api_debug.cpp
    uAPI2/rk_aiq_user_api2_helper.cpp
    uAPI2/rk_aiq_user_api2_wrapper.cpp
	)
set (XCORE_SRC
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
	RkAiqManager.cpp
    )
if(OPENCV_SUPPORT)
set (OPENCV_LIBS
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
set (OPENCV_3RDPARTY_LIBS
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
set (OPENCV_LIBS_DIR
    ${PROJECT_SOURCE_DIR}/common/opencv2/lib
    ${PROJECT_SOURCE_DIR}/common/opencv2/lib/3rdparty
    )
add_compile_options(-DOPENCV_SUPPORT)
endif()

if(GENMESH_ONLINE)
    set (GENMESH_LIBS genMeshLib)
	if(ARCH STREQUAL "aarch64")
		set (GENMESH_LIBS_DIR
			${PROJECT_SOURCE_DIR}/common/gen_mesh/linux/genMesh_static_64bit/
			)
	else()
		set (GENMESH_LIBS_DIR
			${PROJECT_SOURCE_DIR}/common/gen_mesh/linux/genMesh_static_32bit/
			)
	endif()
    add_compile_options(-DGENMESH_ONLINE)
endif()


if(MOTION_DETECT)
    set (MOTION_DETECT_LIBS rkaiq_tnr_md)
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

if(ENABLE_ASAN)
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
SET(LINK_ASAN asan)
endif()

# add C++11 option
#add_compile_options(-std=c++11)
add_compile_options(-fPIC -Wall -Werror -Os)
add_compile_options(-Wno-unused-variable
					-Wno-unused-but-set-variable
					-Wno-unused-function
					-Wno-misleading-indentation
					-Wno-unused-label
                    -Wno-unused-result
					-Wno-format-truncation
					-fno-strict-aliasing
					-Wno-address-of-packed-member
					-Wno-psabi
					-fstack-protector-strong
					-D_FORTIFY_SOURCE=2
                    )
# add_compile_options(-v)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

###########################################################################
#                            BUILD IQ_PARSERV2                            #
###########################################################################
add_subdirectory(iq_parser_v2)
include_directories(include/iq_parser_v2)
include_directories(include/iq_parser_v2/j2s)

# ----------------------------------------------------------------------------
# add algos implement
# ----------------------------------------------------------------------------
add_subdirectory(algos)

if(ARCH STREQUAL "aarch64")
	file(COPY "algos/ae/linux/arm64/librkaiq_ae.a" DESTINATION all_lib/Release/)
	file(COPY "algos/af/linux/arm64/librkaiq_af.a" DESTINATION all_lib/Release/)
	file(COPY "algos/asd/linux/arm64/librkaiq_asd.a" DESTINATION all_lib/Release/)
	file(COPY "algos/awb/linux/arm64/librkaiq_awb.a" DESTINATION all_lib/Release/)
elseif(ARCH STREQUAL "arm")
	file(COPY "algos/ae/linux/arm/librkaiq_ae.a" DESTINATION all_lib/Release/)
	file(COPY "algos/af/linux/arm/librkaiq_af.a" DESTINATION all_lib/Release/)
	file(COPY "algos/asd/linux/arm/librkaiq_asd.a" DESTINATION all_lib/Release/)
	file(COPY "algos/awb/linux/arm/librkaiq_awb.a" DESTINATION all_lib/Release/)
endif()

add_subdirectory(rkaiq_3A_server)

# surpress link_directories warning
cmake_policy(SET CMP0015 NEW)

#
if (LIBRGA_FOUND)
link_directories("${LIBRARY_OUTPUT_PATH}" "${OPENCV_LIBS_DIR}" "${GENMESH_LIBS_DIR}" "${LIBRGA_LIBRARY_DIR}")
else()
link_directories("${LIBRARY_OUTPUT_PATH}" "${OPENCV_LIBS_DIR}" "${GENMESH_LIBS_DIR}")
endif()

add_library(rkaiq SHARED ${RK_AIQ_SRC})
set_target_properties(rkaiq PROPERTIES FOLDER "rk_aiq")
set_target_properties(rkaiq PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(rkaiq PROPERTIES C_VISIBILITY_PRESET default)
set_target_properties(rkaiq PROPERTIES CXX_VISIBILITY_PRESET default)
add_dependencies(rkaiq j2s_parser)

target_link_libraries(rkaiq rkaiq_ae rkaiq_awb rkaiq_af rkaiq_anr rkaiq_asd rkaiq_amerge rkaiq_atmo
	rkaiq_acp rkaiq_asharp rkaiq_adhaz rkaiq_a3dlut rkaiq_ablc rkaiq_accm rkaiq_acgc
	rkaiq_adebayer rkaiq_adpcc rkaiq_afec rkaiq_agamma rkaiq_adegamma rkaiq_agic rkaiq_aie rkaiq_aldch
	rkaiq_alsc rkaiq_aorb rkaiq_ar2y rkaiq_awdr rkaiq_adrc rkaiq_asharp_v3 rkaiq_aynr_v2 rkaiq_acnr_v1
	rkaiq_arawnr_v2 rkaiq_aeis rkaiq_amd rkaiq_arawnr_v1 rkaiq_aynr_v1 rkaiq_auvnr_v1 rkaiq_amfnr_v1 rkaiq_again_v1 ${OPENCV_LIBS} ${OPENCV_3RDPARTY_LIBS} ${GENMESH_LIBS}
	${LINK_ASAN} pthread ${MOTION_DETECT_LIBS})

if (LIBDL_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        libdl::libdl
    )
endif()

if (LIBRGA_FOUND)
    target_link_libraries(rkaiq
        PRIVATE
        rga
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

#add_subdirectory(tests)
install(TARGETS rkaiq
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
       )

install(DIRECTORY include/ DESTINATION include/rkaiq
		FILES_MATCHING PATTERN "*.h")
install(DIRECTORY common/ DESTINATION include/rkaiq/common
		FILES_MATCHING PATTERN "*.h")

