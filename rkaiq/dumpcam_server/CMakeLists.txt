cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

set(AIQ_DUMPCAM_SERVER_STATIC   "rkaiq_dumpcam_server")
set(AIQ_IPCS_STATIC             "rkaiq_ipcs")
set(AIQ_INFO_STATIC             "rkaiq_info")
set(LIB_STATIC_CJSON            "rkaiq_cjson")
set(LIB_STATIC_ARGPARSE         "rkaiq_argparse")

set(SOURCE_DIR_CJSON ${CMAKE_SOURCE_DIR}/iq_parser_v2/j2s CACHE STRING "Path to CJSON source directory")

include_directories(include)
include_directories(ipcs/include)
include_directories(info/include)
include_directories(third-party/cjson)
include_directories(third-party/argparse)
include_directories(${SOURCE_DIR_CJSON})

add_subdirectory(ipcs)
add_subdirectory(info)
add_subdirectory(third-party/cjson)
add_subdirectory(third-party/argparse)

set(SOURCE_FILES_DUMPCAM_SERVER rk_aiq_registry.c)

add_library(${AIQ_DUMPCAM_SERVER_STATIC} STATIC ${SOURCE_FILES_DUMPCAM_SERVER})
set_target_properties(${AIQ_DUMPCAM_SERVER_STATIC} PROPERTIES FOLDER "aiq_dumpcam_server")
target_link_libraries(${AIQ_DUMPCAM_SERVER_STATIC} ${AIQ_IPCS_STATIC}
                      ${AIQ_INFO_STATIC} ${LIB_STATIC_CJSON} ${LIB_STATIC_ARGPARSE})
message(STATUS "cmake version ${CMAKE_VERSION} [@@]config library: aiq_dumpcam_server")
