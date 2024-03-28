if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

PROJECT(smart_ir)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/rk_smart_ir.cpp")
	message("${CMAKE_CURRENT_SOURCE_DIR}/src/rk_smart_ir.cpp not exist, install the libs directly")
	set(C_LIBRARY_NAME "glibc" CACHE INTERNAL "")
	if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/../rkaiq/)
		include(${CMAKE_CURRENT_LIST_DIR}/../rkaiq/cmake/CheckGlibc.cmake)
		CHECK_GLIBC_VERSION()
	endif()
	string(TOLOWER ${RKAIQ_TARGET_SOC} SOC)
	set(CMAKE_LIBRARY_PATH
		"${CMAKE_CURRENT_LIST_DIR}/lib/${SOC}/${ARCH}/${C_LIBRARY_NAME}")
	install(DIRECTORY include/ DESTINATION include/rkaiq/smartIr FILES_MATCHING PATTERN "*.h")
	install(FILES ${CMAKE_LIBRARY_PATH}/libsmartIr.so DESTINATION lib)
	install(FILES ${CMAKE_LIBRARY_PATH}/libsmartIr.a DESTINATION lib RENAME libsmartIr.a)
	return()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(BuildOptions)
include(CompileOptions)

if (${CMAKE_SYSTEM_NAME} EQUAL "")
    set(OS "linux")
else()
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_compile_options(-DANDROID_OS)
endif()

# include(BuildOptions)
# include(CompileOptions)
# include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rk_aiq)
message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})

add_compile_options(-Os)
add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2)

find_path(RKAIQ_INCLUDE_DIR
    NAMES
	rk_aiq.h
    PATH
	common
	rkaiq/common
)
find_library(RKAIQ_LIBRARY_DIR
	rkaiq
)
if(RKAIQ_INCLUDE_DIR AND RKAIQ_LIBRARY_DIR)
set(RK_AIQ_INSTALLED_DIR ${RKAIQ_INCLUDE_DIR})
set(RK_AIQ_LIB_DIR ${RKAIQ_LIBRARY_DIR})
else()
set(RK_AIQ_INSTALLED_DIR "${RK_AIQ_SOURCE_DIR}/build/${OS}/output/${ARCH}/installed" CACHE STRING "AIQ library installed dir")
set(RK_AIQ_LIB_DIR ${RK_AIQ_INSTALLED_DIR}/lib)
endif()
message("rkaiq lib path" ${RK_AIQ_LIB_DIR})
set(RK_AIQ_INCLUDE_DIR
	${RK_AIQ_INSTALLED_DIR}/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/uAPI2
	${RK_AIQ_INSTALLED_DIR}/rkaiq/xcore/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/algos/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/common/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/iq_parser/
	${RK_AIQ_INSTALLED_DIR}/rkaiq/iq_parser_v2/
	${RK_AIQ_INSTALLED_DIR}/include/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/uAPI2
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/xcore/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/algos/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/common/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/iq_parser/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/iq_parser_v2/
	${RK_AIQ_INSTALLED_DIR}/include/rkaiq/isp/
	)
set(RK_AIQ_LIB rkaiq)

if(RK_AIQ_IS_SUBDIR)
get_target_property(RK_AIQ_HEADER_DIR rkaiq INTERFACE_INCLUDE_DIRECTORIES)
message("RK_AIQ_HEADER_DIR ${RK_AIQ_HEADER_DIR}")
endif()

include_directories(
    include
    ${RK_AIQ_INCLUDE_DIR}
	${RK_AIQ_HEADER_DIR}
    )

set (SMARTIR_SRC
	src/rk_smart_ir.cpp
	)
list(APPEND DIR_SRCS ${AFDEMO_DIR_SRCS})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/all_lib/${CMAKE_BUILD_TYPE})
SET(CMAKE_LIBRARY_PATH ${LIBRARY_OUTPUT_PATH})

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
message(${CMAKE_LIBRARY_PATH})
message(${LIBRARY_OUTPUT_PATH})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

link_directories("${RK_AIQ_LIB_DIR}")
add_library(smartIr SHARED ${SMARTIR_SRC})
target_link_libraries(smartIr PUBLIC ${RK_AIQ_LIB})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    target_link_libraries(smartIr
        PRIVATE
        cutils
        utils
        log
        )
endif()

add_library(smartIr_static STATIC ${SMARTIR_SRC})

install(DIRECTORY include/ DESTINATION include/rkaiq/smartIr FILES_MATCHING PATTERN "*.h")
install(FILES ${CMAKE_LIBRARY_PATH}/libsmartIr.so DESTINATION lib)
install(FILES ${CMAKE_LIBRARY_PATH}/libsmartIr_static.a DESTINATION lib RENAME libsmartIr.a)
