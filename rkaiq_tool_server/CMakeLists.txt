cmake_minimum_required(VERSION 2.8)

project(rkaiq_tool_server C CXX)

# add_compile_options(${ISP_HW_VERSION})
#add_definitions(-DISP_HW_V30)
add_definitions(-DISP_HW_V32)

set(CMAKE_VERBOSE_MAKEFILE OFF)

include_directories(
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/netserver/
	${PROJECT_SOURCE_DIR}/rkaiq/
	${PROJECT_SOURCE_DIR}/camera/
	${PROJECT_SOURCE_DIR}/mediactl/)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

aux_source_directory(${PROJECT_SOURCE_DIR}/logger/ LOGGER_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/netserver NET_SERVER_SOURCE)
set(RKAIQ_SOURCE
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api/rkaiq_media.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api/rkaiq_socket.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/multiframe_process.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_online_protocol.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_raw_protocol.cpp
    ${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_protocol.cpp
    )
aux_source_directory(${PROJECT_SOURCE_DIR}/camera/ CAMERA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/mediactl/ MEDIACTL_SOURCE)

option(BUILD_SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(BUILD_SANITIZER_STATIC)
	add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0 )
	link_libraries(libasan.a dl m rt)
endif()

option(BUILD_DEBUG "compile with debug" OFF)
if(BUILD_SANITIZER_STATIC)
        add_definitions(-g -ggdb -gdwarf -funwind-tables -rdynamic -O0 )
endif()

SET(RKAIQ_PROJECT_DIR "../rkaiq")
#SET(RKAIQ_PROJECT_DIR "/home/lili/hdd/projects/3588-linux-ipc-dev/media/isp/camera_engine_rkaiq/rkaiq")
include_directories(${RKAIQ_PROJECT_DIR}/include/iq_parser_v2/j2s)
include_directories(${RKAIQ_PROJECT_DIR}/xcore)
include_directories(${RKAIQ_PROJECT_DIR}/xcore/base)
include_directories(${RKAIQ_PROJECT_DIR}/aiq_core)
include_directories(${RKAIQ_PROJECT_DIR}/algos)
include_directories(${RKAIQ_PROJECT_DIR}/algos_camgro)
include_directories(${RKAIQ_PROJECT_DIR}/common)
include_directories(${RKAIQ_PROJECT_DIR}/common/linux)
include_directories(${RKAIQ_PROJECT_DIR}/hwi)
include_directories(${RKAIQ_PROJECT_DIR}/hwi/isp20)
include_directories(${RKAIQ_PROJECT_DIR}/ipc)
include_directories(${RKAIQ_PROJECT_DIR}/iq_parser)
include_directories(${RKAIQ_PROJECT_DIR}/iq_parser_v2)
include_directories(${RKAIQ_PROJECT_DIR}/uAPI)
include_directories(${RKAIQ_PROJECT_DIR}/uAPI/include)
include_directories(${RKAIQ_PROJECT_DIR}/uAPI2)
include_directories(${RKAIQ_PROJECT_DIR}/ipc_server)
include_directories(${RKAIQ_PROJECT_DIR})
include_directories(${RKAIQ_PROJECT_DIR}/common)
include_directories(${RKAIQ_PROJECT_DIR}/include)
include_directories(${RKAIQ_PROJECT_DIR}/include/common)
include_directories(${RKAIQ_PROJECT_DIR}/include/common/mediactl)
include_directories(${RKAIQ_PROJECT_DIR}/include/iq_parser)
include_directories(${RKAIQ_PROJECT_DIR}/include/iq_parser_v2)
include_directories(${RKAIQ_PROJECT_DIR}/include/uAPI)
include_directories(${RKAIQ_PROJECT_DIR}/include/uAPI2)
include_directories(${RKAIQ_PROJECT_DIR}/include/xcore)
include_directories(${RKAIQ_PROJECT_DIR}/include/xcore/base)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/a3dlut)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/ablc)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/accm)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acgc)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acp)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/adebayer)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/adehaze)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/adpcc)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/ae)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/af)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/afec)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/agamma)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/adegamma)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/agic)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/amerge)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/atmo)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/adrc)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aie)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aldch)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/alsc)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/anr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/anr/tnr_md)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aorb)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acsm)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/asd)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/asharp)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/awb)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/awdr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/asharp3)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aynr2)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acnr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aeis)
include_directories(${RKAIQ_PROJECT_DIR}/include/common/gen_mesh)
include_directories(${RKAIQ_PROJECT_DIR}/include/ipc_server)
include_directories(${RKAIQ_PROJECT_DIR}/include/amd)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/arawnr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aynr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/auvnr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/amfnr)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/again)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aynr3)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acnr2)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/asharp4)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/abayer2dnr2)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/abayertnr2)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/again2)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acac)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos_camgroup/ae)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos_camgroup/awb)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/abayer2dnrV23)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/abayertnrV23)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/aynrV22)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/acnrV30)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/asharpV33)
include_directories(${RKAIQ_PROJECT_DIR}/include/algos/ablcV32)
include_directories(${RKAIQ_PROJECT_DIR}/include/isp)
include_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/arm)

#include_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/arm/installed/include/rkaiq)
#link_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/arm/all_lib/MinSizeRel)

# include_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/aarch64)
# link_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/aarch64/all_lib/MinSizeRel)
# link_directories(${RKAIQ_PROJECT_DIR}/build/linux/output/aarch64/all_lib/RelWithDebInfo)

include_directories(${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api)

# link_libraries(rkaiq)
# add_definitions(-DENABLE_RKAIQ_UAPI)

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.cc *.h)

set(RKAIQ_TOOL_SERVER_SOURCE
	${LOGGER_SOURCE}
	${NET_SERVER_SOURCE}
	${RKAIQ_SOURCE}
	${CAMERA_SOURCE}
	${MEDIACTL_SOURCE}
	rkaiq_tool_server.cpp)

add_executable(rkaiq_tool_server ${RKAIQ_TOOL_SERVER_SOURCE})
target_link_libraries(rkaiq_tool_server pthread)
install(TARGETS rkaiq_tool_server RUNTIME DESTINATION bin)

option(BUILD_TEST "build test demo" OFF)
if(BUILD_TEST)
	add_subdirectory(test)
endif()
