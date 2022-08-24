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

project(rga_allocator_demo)

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
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty/libdrm/include)
include_directories(${CMAKE_SOURCE_DIR}/../3rdparty/libdrm/include/libdrm)
include_directories(${CMAKE_SOURCE_DIR}/allocator/include/)

add_subdirectory(${CMAKE_SOURCE_DIR}/../utils ./utils_obj)

# allocator_obj
add_library(allocator_obj OBJECT
    allocator/dma_alloc.cpp
    allocator/drm_alloc.cpp
)

# rga_allocator_malloc_demo
add_executable(rga_allocator_malloc_demo
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_malloc_demo.cpp
)
target_link_libraries(rga_allocator_malloc_demo
	${RGA_LIB}
)
install(TARGETS rga_allocator_malloc_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

if (TARGET_SOC STREQUAL "RV1106")
    # rga_allocator_dma32_demo
    add_executable(rga_allocator_1106_cma_demo
        $<TARGET_OBJECTS:allocator_obj>
        $<TARGET_OBJECTS:utils_obj>
        src/rga_allocator_1106_cma_demo.cpp
    )
    target_link_libraries(rga_allocator_1106_cma_demo
        ${RGA_LIB}
    )
    install(TARGETS rga_allocator_1106_cma_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
else ()
    # rga_allocator_drm_demo
    add_executable(rga_allocator_drm_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_drm_demo.cpp
    )
    target_link_libraries(rga_allocator_drm_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_allocator_drm_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_allocator_drm_phy_demo
    add_executable(rga_allocator_drm_phy_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_drm_phy_demo.cpp
    )
    target_link_libraries(rga_allocator_drm_phy_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_allocator_drm_phy_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_allocator_dma_demo
    add_executable(rga_allocator_dma_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_dma_demo.cpp
    )
    target_link_libraries(rga_allocator_dma_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_allocator_dma_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_allocator_dma_cache_demo
    add_executable(rga_allocator_dma_cache_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_dma_cache_demo.cpp
    )
    target_link_libraries(rga_allocator_dma_cache_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_allocator_dma_cache_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_allocator_dma32_demo
    add_executable(rga_allocator_dma32_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_allocator_dma32_demo.cpp
    )
    target_link_libraries(rga_allocator_dma32_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_allocator_dma32_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
