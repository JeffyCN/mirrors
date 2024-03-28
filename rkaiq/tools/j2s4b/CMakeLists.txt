cmake_minimum_required(VERSION 2.8...3.22)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
if (NOT RKAIQ_J2S4B_DEV)
if (RKAIQ_TARGET_ARCH STREQUAL "arm")
    set(CMAKE_C_FLAGS -m32)
    set(CMAKE_CXX_FLAGS -m32)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
endif()
endif()
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

add_compile_options(-std=c++11)
add_compile_options(-fPIC -Wall -Wno-error -O0 -g)
add_compile_options(-Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-misleading-indentation
    -Wno-unused-label
    -Wno-format-truncation
    -fno-strict-aliasing)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GetGitVersion)
get_git_version(GIT_VERSION)

PROJECT(j2s4b)

set(ROOT_PROJ_DIR ${PROJECT_SOURCE_DIR}/../..)
if (ROOT_PROJ_BIN_DIR)
    message("ROOT_PROJ_BIN_DIR:${ROOT_PROJ_BIN_DIR}")
    include_directories(${ROOT_PROJ_BIN_DIR})
else()
    message(FATAL_ERROR "Could not find ROOT_PROJ_BIN_DIR")
endif()

set(RKAIQ_J2S4B_DEV FALSE CACHE INTERNAL "")

if (NOT RKAIQ_J2S4B_DEV)
if (RKAIQ_IQFILES_DIR)
    file(GLOB RKAIQ_IQFILES  ${RKAIQ_IQFILES_DIR}/*.json)
	file(GLOB RKAIQ_AINR_IQFILES  ${RKAIQ_IQFILES_DIR}/ainr/*.json)
else()
    message(FATAL_ERROR "Could not find RKAIQ_IQFILES_DIR")
endif()
else()
add_compile_options(-DRKAIQ_J2S4B_DEV)
endif()

include_directories(
    src
    src/j2s4b
    include
    ${ROOT_PROJ_DIR}/include/iq_parser_v2/j2s/
    ${ROOT_PROJ_DIR}
    )

set(CXX_SRCS
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON.cpp
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON_Utils.cpp
    src/j2s4b/BinRecord.cpp
    src/j2s4b/RkAiqsceneManager.cpp
    src/j2s4b/j2s.cpp
    src/j2s4b/j2s4b_utils.cpp
    src/main.cpp)

add_definitions(-DGIT_VERSION="${GIT_VERSION}")

if (NOT RKAIQ_J2S4B_DEV)
ADD_EXECUTABLE(j2s4b ${CXX_SRCS})
else()
ADD_EXECUTABLE(j2s4b_dev ${CXX_SRCS})
endif()

if (RKAIQ_TARGET_ARCH STREQUAL "arm")
if (NOT RKAIQ_J2S4B_DEV)
set_target_properties(j2s4b PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
endif()

if (NOT RKAIQ_J2S4B_DEV)
foreach(IQPATH ${RKAIQ_IQFILES})
    string(REGEX MATCH "${RKAIQ_IQFILES_DIR}/.*" SHORT_NAME ${IQPATH})
    string(REGEX REPLACE "${RKAIQ_IQFILES_DIR}/" "" ORIGIN_NAME ${SHORT_NAME})
    string(REGEX REPLACE "json" "bin" TAGT_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE ".json" "" BIN_PROJ_NAME ${ORIGIN_NAME})
    add_custom_target(
        ${BIN_PROJ_NAME}
        ALL
        COMMAND j2s4b ${IQPATH} ${TAGT_NAME}
        BYPRODUCTS ${TAGT_NAME}
        DEPENDS j2s4b
        )
    install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_NAME}" DESTINATION usr/share/iqfiles)
endforeach()

foreach(IQPATH ${RKAIQ_AINR_IQFILES})
    string(REGEX MATCH "${RKAIQ_IQFILES_DIR}/ainr/.*" SHORT_NAME ${IQPATH})
    string(REGEX REPLACE "${RKAIQ_IQFILES_DIR}/ainr/" "" ORIGIN_NAME ${SHORT_NAME})
    string(REGEX REPLACE "json" "bin" TAGT_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE "json" "bin.ainr" TAGT_AINR_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE ".json" "ainr" BIN_PROJ_NAME ${ORIGIN_NAME})
    add_custom_target(
        ${BIN_PROJ_NAME}
        ALL
        COMMAND j2s4b ${IQPATH} ${TAGT_AINR_NAME}
        BYPRODUCTS ${TAGT_AINR_NAME}
        DEPENDS j2s4b
        )
    install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_AINR_NAME}" DESTINATION
		usr/share/iqfiles/ainr RENAME ${TAGT_NAME})
endforeach()
endif()

if (NOT RKAIQ_J2S4B_DEV)
install(TARGETS j2s4b RUNTIME DESTINATION host)
else()
install(TARGETS j2s4b_dev RUNTIME DESTINATION bin)
endif()

