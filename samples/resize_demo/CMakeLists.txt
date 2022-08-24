cmake_minimum_required(VERSION 3.5)

if (EXISTS ${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH})
    message("load ${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH}")
    include(${CMAKE_SOURCE_DIR}/${BUILD_TOOLCHAINS_PATH})
endif()

if (EXISTS ${LIBRGA_FILE_LIB}/librga.so)
	message("load ${LIBRGA_FILE_LIB}/librga.so")
    set(RGA_LIB ${LIBRGA_FILE_LIB}/librga.so)
else ()
    set(RGA_LIB rga)
endif()

project(rga_resize_demo)

#install path
if (NOT DEFINED CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR bin)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--allow-shlib-undefined -ldl")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--allow-shlib-undefined -ldl")

set(RGA_INCLUDE
    ${CMAKE_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/../../im2d_api)

include_directories(${RGA_INCLUDE})
include_directories(${CMAKE_SOURCE_DIR}/../utils)

add_subdirectory(${CMAKE_SOURCE_DIR}/../utils ./utils_obj)

# rga_resize_demo
add_executable(rga_resize_demo
    $<TARGET_OBJECTS:utils_obj>
    src/rga_resize_demo.cpp
)
target_link_libraries(rga_resize_demo
	${RGA_LIB}
)
install(TARGETS rga_resize_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

# rga_resize_rect_demo
add_executable(rga_resize_rect_demo
    $<TARGET_OBJECTS:utils_obj>
    src/rga_resize_rect_demo.cpp
)
target_link_libraries(rga_resize_rect_demo
	${RGA_LIB}
)
install(TARGETS rga_resize_rect_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
