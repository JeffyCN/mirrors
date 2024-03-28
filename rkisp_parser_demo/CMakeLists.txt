
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

#project name
PROJECT(rkisp_parser_demo)

# ----------------------------------------------------------------------------
# set property to classify library kinds
# ----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#set ADD_RK_AIQ
#set(ADD_RK_AIQ true)

#set(ENABLE_ASAN true)

if(NOT WIN32)
if(ENABLE_ASAN)
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
SET(LINK_ASAN asan)
endif()
add_compile_options(-std=c++11)
endif()

if(ADD_RK_AIQ)
add_compile_options(-DADD_RK_AIQ)
SET(RK_AIQ_LIB_DIR ${PROJECT_SOURCE_DIR}/../rk_aiq/build/linux/all_lib/${CMAKE_BUILD_TYPE})
message("rkaiq lib path" ${RK_AIQ_LIB_DIR})
SET(RK_AIQ_LIB rkaiq)
endif()

set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rk_aiq)
message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})
#head file path
include_directories(
${RK_AIQ_SOURCE_DIR}
${RK_AIQ_SOURCE_DIR}/iq_parser
${RK_AIQ_SOURCE_DIR}/xcore
${RK_AIQ_SOURCE_DIR}/xcore/base
${RK_AIQ_SOURCE_DIR}/include/iq_parser
${RK_AIQ_SOURCE_DIR}/include/algos
${RK_AIQ_SOURCE_DIR}/include/xcore
${RK_AIQ_SOURCE_DIR}/include/xcore/base
${RK_AIQ_SOURCE_DIR}/include/common
)
#source directory
#AUX_SOURCE_DIRECTORY(demo DIR_SRCS)
set (DIR_SRCS
	demo/rkisp_parse.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/RkAiqCalibDb.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/RkAiqCalibParser.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/RkAiqCalibTag.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/tinyxml2.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/xmltags.cpp
	${RK_AIQ_SOURCE_DIR}/xcore/xcam_log.cpp
	${RK_AIQ_SOURCE_DIR}/iq_parser/RkAiqCalibApi.cpp
    )

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

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#
link_directories("${LIBRARY_OUTPUT_PATH}" "${RK_AIQ_LIB_DIR}")
#add executable file
ADD_EXECUTABLE(rkisp_parser ${DIR_SRCS})
target_link_libraries(rkisp_parser ${RK_AIQ_LIB} ${LINK_ASAN})
if(ARCH STREQUAL "m32")
message("arch m32")
set_target_properties(rkisp_parser PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
else()
message("arch m64")
set_target_properties(rkisp_parser PROPERTIES COMPILE_FLAGS "-m64" LINK_FLAGS "-m64")
endif()
