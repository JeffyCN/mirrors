if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8)

PROJECT(rkisp_demo)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildOptions)
include(CompileOptions)
include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

set(ADD_RK_AIQ true)
option(ENABLE_IIO_API "Enable IIO API for EIS algorithm" OFF)

set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rk_aiq)
message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})

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
    set(RK_AIQ_INCLUDE_DIR
		#${RK_AIQ_SOURCE_DIR}
        ${RK_AIQ_SOURCE_DIR}/include/
        ${RK_AIQ_SOURCE_DIR}/include/uAPI2
        ${RK_AIQ_SOURCE_DIR}/include/xcore/
        ${RK_AIQ_SOURCE_DIR}/include/algos/
        ${RK_AIQ_SOURCE_DIR}/include/common/
        ${RK_AIQ_SOURCE_DIR}/include/iq_parser/
        ${RK_AIQ_SOURCE_DIR}/include/iq_parser_v2/
        #${FPGA_RGA_INCLUDE_PATH}/include/
        )
    set(RK_AIQ_LIB rkaiq)
endif()

if (ARCH STREQUAL "aarch64")
    set(RK_DRM_LIB_DIR ${PROJECT_SOURCE_DIR}/demo/libs/arm64/)
elseif (ARCH STREQUAL "arm")
    set(RK_DRM_LIB_DIR
        ${PROJECT_SOURCE_DIR}/demo/libs/arm32/
        )
endif()

SET(RK_DRM_LIB drm)
if(NOT ARCH STREQUAL "fpga")
    set(RK_RGA_LIB rga)
endif()

if (ENABLE_IIO_API)
    set(IIO_LIB_DIR ${PROJECT_SOURCE_DIR}/demo/iio/lib)
    set(IIO_LIBS easyiio iio serialport xml2 )
endif()

include_directories(
    demo
    demo/include
    demo/include/rga
    demo/include/libdrm
    demo/drmDsp
    demo/iio/include
    ${RK_AIQ_INCLUDE_DIR}
    demo/sample/
	${SDK_MEDIA_DIR}/include
    )

if (NOT ARCH STREQUAL "fpga")
    aux_source_directory(demo DIR_SRCS)
    aux_source_directory(demo/drmDsp DRMDSP_DIR_SRCS)
    aux_source_directory(demo/sample SAMPLE_DIR_SRCS)
else ()
    set (DIR_SRCS
        demo/rkisp_demo.cpp
        demo/drmDsp.c
        demo/drmDsp/bo.c
        demo/drmDsp/dev.c
        demo/drmDsp/modeset.c
        )
endif()

if (ENABLE_IIO_API)
    AUX_SOURCE_DIRECTORY(demo/iio/demo IIO_DIR_SRCS)
endif()
list(APPEND DIR_SRCS ${DRMDSP_DIR_SRCS} ${IIO_DIR_SRCS} ${SAMPLE_DIR_SRCS})

AUX_SOURCE_DIRECTORY(demo/ae_algo_demo AEDEMO_DIR_SRCS)
list(APPEND DIR_SRCS ${AEDEMO_DIR_SRCS})

AUX_SOURCE_DIRECTORY(demo/awb_algo_demo AWBDEMO_DIR_SRCS)
list(APPEND DIR_SRCS ${AWBDEMO_DIR_SRCS})

AUX_SOURCE_DIRECTORY(demo/af_algo_demo AFDEMO_DIR_SRCS)
list(APPEND DIR_SRCS ${AFDEMO_DIR_SRCS})

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

link_directories("${LIBRARY_OUTPUT_PATH}" "${RK_AIQ_LIB_DIR}" "${RK_DRM_LIB_DIR}" "${IIO_LIB_DIR}")
add_executable(rkisp_demo ${DIR_SRCS})
target_link_libraries(rkisp_demo ${RK_AIQ_LIB} ${RK_DRM_LIB} ${RK_RGA_LIB} ${IIO_LIBS} pthread dl)

#FIXME: buildroot and ipc sdk shall use the same setting
if(BUILDROOT_BUILD_PROJECT)
	install(TARGETS rkisp_demo
	RUNTIME DESTINATION /usr/bin
	LIBRARY DESTINATION /usr/lib
	)

else()
	install(TARGETS rkisp_demo
	RUNTIME DESTINATION "usr/bin"
	LIBRARY DESTINATION "usr/lib"
)
endif()
