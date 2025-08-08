cmake_minimum_required(VERSION 2.8...3.22)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
if (NOT RKAIQ_J2S4B_DEV)
if (RKAIQ_TARGET_ARCH STREQUAL "arm")
    set(CMAKE_C_FLAGS -m32)
    set(CMAKE_CXX_FLAGS -m32)
    set(CMAKE_EXE_LINKER_FLAGS -m32)
endif()
endif()
string(FIND "${CMAKE_C_FLAGS}" "-fsanitize=address" DEBUG_WITH_ASAN)
if(NOT DEBUG_WITH_ASAN EQUAL -1)
	message(STATUS "Build with asan")
else()
	message(STATUS "Build without asan")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_compile_options(-fPIC -Wall -Wno-error -O0 -g -D_FORTIFY_SOURCE=0)
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
    file(GLOB RKAIQ_THUNDERBOOT_IQFILES  ${RKAIQ_IQFILES_DIR}/thunderboot/*.json)
    file(GLOB RKAIQ_COMMON_IQFILES  ${RKAIQ_IQFILES_DIR}/common/*.json)
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
    ${ROOT_PROJ_DIR}/xcore
    ${ROOT_PROJ_DIR}/include/xcore
    ${ROOT_PROJ_DIR}/include/common
    )

set(J2S4B_SRCS
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON.c
    ${ROOT_PROJ_DIR}/iq_parser_v2/j2s/cJSON_Utils.c
    ${ROOT_PROJ_DIR}/c_base/aiq_base.c
    ${ROOT_PROJ_DIR}/c_base/aiq_mutex.c
    ${ROOT_PROJ_DIR}/xcore/xcam_log.c
    src/j2s4b/BinRecord.cpp
    src/j2s4b/RkAiqsceneManager.cpp
    src/j2s4b/j2s.cpp
    src/j2s4b/j2s4b_utils.cpp
    src/main.cpp)

add_definitions(-DGIT_VERSION="${GIT_VERSION}")

if (NOT RKAIQ_J2S4B_DEV)
ADD_EXECUTABLE(j2s4b ${J2S4B_SRCS})
if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Android"))
target_link_libraries(j2s4b
	PRIVATE
	pthread)
endif()
else()
ADD_EXECUTABLE(j2s4b_dev ${J2S4B_SRCS})
if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Android"))
target_link_libraries(j2s4b_dev
	PRIVATE
	pthread)
endif()
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
    if (IS_SYMLINK ${IQPATH})
        get_filename_component(REAL_FILE_PATH ${IQPATH} REALPATH)
        get_filename_component(REAL_FILE_NAME ${REAL_FILE_PATH} NAME_WE)
        set(TARGET_BIN_NAME ${REAL_FILE_NAME}.bin)
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME})
            # create a empty file for link
            file(WRITE ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME} "")
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET_BIN_NAME} ${TAGT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            OUTPUT_VARIABLE SYMLINK_RESULT
        )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_NAME}" DESTINATION
            usr/share/iqfiles/ainr)
    else ()
        add_custom_target(
            ${BIN_PROJ_NAME}
            ALL
            COMMAND j2s4b ${IQPATH} ${TAGT_AINR_NAME}
            BYPRODUCTS ${TAGT_AINR_NAME}
            DEPENDS j2s4b
            )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_AINR_NAME}" DESTINATION
            usr/share/iqfiles/ainr RENAME ${TAGT_NAME})
    endif()
endforeach()

foreach(IQPATH ${RKAIQ_THUNDERBOOT_IQFILES})
    string(REGEX MATCH "${RKAIQ_IQFILES_DIR}/thunderboot/.*" SHORT_NAME ${IQPATH})
    string(REGEX REPLACE "${RKAIQ_IQFILES_DIR}/thunderboot/" "" ORIGIN_NAME ${SHORT_NAME})
    string(REGEX REPLACE "json" "bin" TAGT_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE "json" "bin.thunderboot" TAGT_THUNDERBOOT_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE ".json" "thunderboot" BIN_PROJ_NAME ${ORIGIN_NAME})
    if (IS_SYMLINK ${IQPATH})
        get_filename_component(REAL_FILE_PATH ${IQPATH} REALPATH)
        get_filename_component(REAL_FILE_NAME ${REAL_FILE_PATH} NAME_WE)
        set(TARGET_BIN_NAME ${REAL_FILE_NAME}.bin)
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME})
            # create a empty file for link
            file(WRITE ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME} "")
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET_BIN_NAME} ${TAGT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            OUTPUT_VARIABLE SYMLINK_RESULT
        )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_NAME}" DESTINATION
            usr/share/iqfiles/thunderboot)
    else ()
        add_custom_target(
            ${BIN_PROJ_NAME}
            ALL
            COMMAND j2s4b ${IQPATH} ${TAGT_THUNDERBOOT_NAME}
            BYPRODUCTS ${TAGT_THUNDERBOOT_NAME}
            DEPENDS j2s4b
            )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_THUNDERBOOT_NAME}" DESTINATION
            usr/share/iqfiles/thunderboot RENAME ${TAGT_NAME})
    endif()
endforeach()

foreach(IQPATH ${RKAIQ_COMMON_IQFILES})
    string(REGEX MATCH "${RKAIQ_IQFILES_DIR}/common/.*" SHORT_NAME ${IQPATH})
    string(REGEX REPLACE "${RKAIQ_IQFILES_DIR}/common/" "" ORIGIN_NAME ${SHORT_NAME})
    string(REGEX REPLACE "json" "bin" TAGT_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE "json" "bin.common" TAGT_COMMON_NAME ${ORIGIN_NAME})
    string(REGEX REPLACE ".json" "common" BIN_PROJ_NAME ${ORIGIN_NAME})
    if (IS_SYMLINK ${IQPATH})
        get_filename_component(REAL_FILE_PATH ${IQPATH} REALPATH)
        get_filename_component(REAL_FILE_NAME ${REAL_FILE_PATH} NAME_WE)
        set(TARGET_BIN_NAME ${REAL_FILE_NAME}.bin)
        if(NOT EXISTS ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME})
            # create a empty file for link
            file(WRITE ${CMAKE_BINARY_DIR}/${TARGET_BIN_NAME} "")
        endif()
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink ${TARGET_BIN_NAME} ${TAGT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            OUTPUT_VARIABLE SYMLINK_RESULT
        )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_NAME}" DESTINATION
            usr/share/iqfiles/common)
    else ()
        add_custom_target(
            ${BIN_PROJ_NAME}
            ALL
            COMMAND j2s4b ${IQPATH} ${TAGT_COMMON_NAME}
            BYPRODUCTS ${TAGT_COMMON_NAME}
            DEPENDS j2s4b
            )
        install(PROGRAMS "${CMAKE_BINARY_DIR}/${TAGT_COMMON_NAME}" DESTINATION
            usr/share/iqfiles/common RENAME ${TAGT_NAME})
    endif()
endforeach()
endif()

if (NOT RKAIQ_J2S4B_DEV)
install(TARGETS j2s4b RUNTIME DESTINATION host)
else()
install(TARGETS j2s4b_dev RUNTIME DESTINATION bin)
endif()

install(DIRECTORY ${RKAIQ_IQFILES_DIR}/ DESTINATION usr/share/iqfiles/)

