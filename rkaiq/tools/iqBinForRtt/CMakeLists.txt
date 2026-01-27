cmake_minimum_required(VERSION 2.8...3.22)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

message("RKAIQ_TARGET_ARCH: ${RKAIQ_TARGET_ARCH}")

if (RKAIQ_TARGET_ARCH STREQUAL "arm")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
endif()

# message(FATAL_ERROR "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS} ${RKAIQ_ISP_FLAG}")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_compile_options(-fPIC -Wall -Wno-error -O0 -g -D_FORTIFY_SOURCE=0)
add_compile_options(-Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-misleading-indentation
    -Wno-unused-label
    -Wno-format-truncation
    -fno-strict-aliasing
    -malign-double)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../j2s4b/cmake")
include(GetGitVersion)
get_git_version(GIT_VERSION)

PROJECT(iqBinForRtt)

set(ROOT_PROJ_DIR ${PROJECT_SOURCE_DIR}/../..)
if (ROOT_PROJ_BIN_DIR)
    message("ROOT_PROJ_BIN_DIR:${ROOT_PROJ_BIN_DIR}")
    include_directories(${ROOT_PROJ_BIN_DIR})
else()
    message(FATAL_ERROR "Could not find ROOT_PROJ_BIN_DIR")
endif()

add_compile_options(${ISP_HW_VERSION})

if (RKAIQ_NEWSTRUCT_TEST)
    add_compile_options(-DUSE_NEWSTRUCT=1)
endif()

include_directories(
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/
    ${ROOT_PROJ_DIR}/include/iq_parser_v2
    ${ROOT_PROJ_DIR}
    ${ROOT_PROJ_DIR}/xcore
    ${ROOT_PROJ_DIR}/include/xcore
    ${ROOT_PROJ_DIR}/include/common
    ${ROOT_PROJ_DIR}/include/uAPI2
    ${ROOT_PROJ_DIR}/include/algos
    ${ROOT_PROJ_DIR}/include
    )

set(IQBINFORRTT_SRCS
    iqBinForRtt.cpp)

ADD_EXECUTABLE(iqBinForRtt ${IQBINFORRTT_SRCS})

if (RKAIQ_TARGET_ARCH STREQUAL "arm")
set_target_properties(iqBinForRtt PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

install(TARGETS iqBinForRtt RUNTIME DESTINATION host)