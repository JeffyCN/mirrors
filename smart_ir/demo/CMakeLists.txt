if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

PROJECT(smartIr_demo)

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
	set(RK_AIQ_INSTALLED_DIR ${PROJECT_SOURCE_DIR}/../../rk_aiq/build/${OS}/output/${ARCH}/installed)
	set(RK_AIQ_LIB_DIR ${RK_AIQ_INSTALLED_DIR}/lib)
endif()
message("rkaiq lib path: " ${RK_AIQ_LIB_DIR})

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

if (RK_AIQ_IS_SUBDIR)
	get_target_property(RK_AIQ_HEADER_DIR rkaiq INTERFACE_INCLUDE_DIRECTORIES)
	message("RK_AIQ_HEADER_DIR ${RK_AIQ_HEADER_DIR}")
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/../lib)
	set(SMARTIR_LIB_DIR ${PROJECT_SOURCE_DIR}/../lib/${SOC}/${ARCH}/${C_LIBRARY_NAME})
else()
	set(SMARTIR_LIB_DIR ${PROJECT_SOURCE_DIR}/../build/${OS}/output/${ARCH}/installed/lib)
endif()
message("smartIr lib path: " ${SMARTIR_LIB_DIR})
set(SMARTIR_DEMO_DIR ${PROJECT_SOURCE_DIR})
set(SMARTIR_INCLUDE_DIR ${SMARTIR_DEMO_DIR}/../include)
set(SMARTIR_LIB smartIr)

include_directories(demo ${RK_AIQ_INCLUDE_DIR} ${RK_AIQ_HEADER_DIR} ${SMARTIR_INCLUDE_DIR} ${SMARTIR_DEMO_DIR})
set(DIR_SRCS smartIr_demo.c)
add_executable(smartIr_demo ${DIR_SRCS})
add_dependencies(smartIr_demo ${SMARTIR_LIB})
link_directories("${RK_AIQ_LIB_DIR}" "${SMARTIR_LIB_DIR}")
target_link_libraries(smartIr_demo ${RK_AIQ_LIB} ${SMARTIR_LIB})
if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	target_link_libraries(smartIr_demo log utils cutils)
else ()
	target_link_libraries(smartIr_demo pthread)
endif()
target_link_libraries(smartIr_demo m)
install(TARGETS smartIr_demo RUNTIME DESTINATION bin)

