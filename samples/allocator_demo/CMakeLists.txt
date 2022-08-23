cmake_minimum_required(VERSION 3.5)

    message("test ${BUILD_TOOLCHAINS_PATH}")

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

# allocator_obj
add_library(allocator_obj OBJECT
    allocator/dma_alloc.cpp
    allocator/drm_alloc.cpp
)

# allocator demo utils_obj
add_library(utils_obj OBJECT
    ../utils/utils.cpp)

# rga_malloc_demo
add_executable(rga_malloc_demo
    $<TARGET_OBJECTS:utils_obj>
    src/rga_malloc_demo.cpp
)
target_link_libraries(rga_malloc_demo
	${RGA_LIB}
)
install(TARGETS rga_malloc_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

if (TARGET_SOC STREQUAL "RV1106")
    # rga_dma32_demo
    add_executable(rga_1106_cma_demo
        $<TARGET_OBJECTS:allocator_obj>
        $<TARGET_OBJECTS:utils_obj>
        src/rga_1106_cma_demo.cpp
    )
    target_link_libraries(rga_1106_cma_demo
        ${RGA_LIB}
    )
    install(TARGETS rga_1106_cma_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
else ()
    # rga_drm_demo
    add_executable(rga_drm_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_drm_demo.cpp
    )
    target_link_libraries(rga_drm_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_drm_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_drm_phy_demo
    add_executable(rga_drm_phy_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_drm_phy_demo.cpp
    )
    target_link_libraries(rga_drm_phy_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_drm_phy_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_dma_demo
    add_executable(rga_dma_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_dma_demo.cpp
    )
    target_link_libraries(rga_dma_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_dma_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_dma_cache_demo
    add_executable(rga_dma_cache_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_dma_cache_demo.cpp
    )
    target_link_libraries(rga_dma_cache_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_dma_cache_demo DESTINATION ${CMAKE_INSTALL_BINDIR})

    # rga_dma32_demo
    add_executable(rga_dma32_demo
    $<TARGET_OBJECTS:allocator_obj>
    $<TARGET_OBJECTS:utils_obj>
    src/rga_dma32_demo.cpp
    )
    target_link_libraries(rga_dma32_demo
    ${RGA_LIB}
    )
    install(TARGETS rga_dma32_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
