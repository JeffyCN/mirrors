
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8...3.22)

#project name
PROJECT(rkaiq_3A_server)

#set BUILDROOT_BUILD_PROJECT
#set(BUILDROOT_BUILD_PROJECT true)
#set(ENABLE_ASAN true)

# ----------------------------------------------------------------------------
# SET THE COMPILER PATH
# ----------------------------------------------------------------------------
#if(BUILDROOT_BUILD_PROJECT)
    # buildroot will set the compiler
#else()
#    SET(CMAKE_C_COMPILER "/home/camera/camera/rv1109_sdk/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc") 
#    SET(CMAKE_CXX_COMPILER "/home/camera/camera/rv1109_sdk/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")
#endif()

# ----------------------------------------------------------------------------
# ADD SYMBOL TABLE FOR GDB
# ----------------------------------------------------------------------------
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -ggdb")

# ----------------------------------------------------------------------------
# set property to classify library kinds
# ----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -fPIC")

#set ADD_RK_AIQ
set(ADD_RK_AIQ true)

#SET RK_AIQ SOURCE DIR
set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../rkaiq)
message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})

if(ENABLE_ASAN)
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
SET(LINK_ASAN asan)
endif()

if(ADD_RK_AIQ)
add_compile_options(-std=c++11)
add_compile_options(-DADD_RK_AIQ)
add_compile_options(-O)
# add_compile_options(-v)
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
message("rkaiq lib path" ${RK_AIQ_LIB_DIR})
SET(RK_AIQ_INCLUDE_DIR
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/uAPI2
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/xcore/
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/algos/
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/isp
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/common/
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/iq_parser/
        ${RK_AIQ_INSTALLED_DIR}/rkaiq/iq_parser_v2/
)
SET(RK_AIQ_LIB rkaiq)
else()
message("Please check librkaiq.so and include files")
endif()
endif()

if(RK_AIQ_IS_SUBDIR)
get_target_property(RK_AIQ_HEADER_DIR rkaiq INTERFACE_INCLUDE_DIRECTORIES)
message("RK_AIQ_HEADER_DIR ${RK_AIQ_HEADER_DIR}")
SET(RK_AIQ_LIB rkaiq)
endif()

include_directories(
${RK_AIQ_INCLUDE_DIR}
${RK_AIQ_HEADER_DIR}
include/common/mediactl
./
./common/
)

if(GENMESH_ONLINE)
	if(ARCH STREQUAL "aarch64")
		set (GENMESH_LIBS_DIR
			${PROJECT_SOURCE_DIR}/../common/gen_mesh/linux/genMesh_static_64bit/
			)
	else()
		set (GENMESH_LIBS_DIR
			${PROJECT_SOURCE_DIR}/../common/gen_mesh/linux/genMesh_static_32bit/
			)
	endif()
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

link_directories("${RK_AIQ_LIB_DIR}" "${GENMESH_LIBS_DIR}")
ADD_EXECUTABLE(rkaiq_3A_server rkaiq_3A_server.cpp common/mediactl/mediactl.c)

target_link_libraries(rkaiq_3A_server ${RK_AIQ_LIB} pthread dl)

#FIXME: buildroot and ipc sdk shall use the same setting
if(BUILDROOT_BUILD_PROJECT)
	install(TARGETS rkaiq_3A_server
		RUNTIME DESTINATION /usr/bin
		LIBRARY DESTINATION /usr/lib
		)

	install(FILES S40rkaiq_3A DESTINATION /etc/init.d/
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
else()
	install(TARGETS rkaiq_3A_server
		RUNTIME DESTINATION "usr/bin"
		LIBRARY DESTINATION "usr/lib"
		)

	install(FILES S40rkaiq_3A DESTINATION "etc/init.d"
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
		GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
endif()
