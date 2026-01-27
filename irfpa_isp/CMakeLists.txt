cmake_minimum_required(VERSION 3.15)
project(rk_irfpa C CXX ASM)

option(OPTION_IPCSDK "build IPCSDK targets" ON)
option(OPTION_ISP_INTERNAL "use isp internal make toolchain file" ON)

option(OPTION_BUILD_STATIC "if build algo static library" OFF)

option(OPTION_BUILD_DEMO "if build any demo" ON)
option(OPTION_BUILD_STREAMHW_DEMO "if build streamhw demo" ON)
option(OPTION_BUILD_SENSORHW_DEMO "if build sensorhw demo" ON)
option(OPTION_BUILD_SIMPLE_DEMO "if build simple demo" OFF)

set(RK_IRFPA_PLATFORM rv1126b)

if (OPTION_ISP_INTERNAL)
    set(CMAKE_TOOLCHAIN_FILE cmake/isp_internal.cmake)
endif()

include(rk_irfpa_sdk_init.cmake) 

if (OPTION_BUILD_DEMO)
    if (OPTION_BUILD_STREAMHW_DEMO)
        add_subdirectory(demo/streamhw_demo)
    endif()

    if (OPTION_BUILD_SENSORHW_DEMO)
        add_subdirectory(demo/sensorhw_demo)
    endif()

    if (OPTION_BUILD_SIMPLE_DEMO)
        add_subdirectory(demo/simple_demo)
    endif()
endif()

if (OPTION_IPCSDK)
    message("IPCSDK toolchain: ${CMAKE_C_COMPILER}")
    
    add_library(rkirfpa SHARED)
    target_link_libraries(rkirfpa PRIVATE rk_irfpa_simple_api)

    add_executable(rk_irfpa_simple_demo)
    target_sources(rk_irfpa_simple_demo PRIVATE demo/simple_demo/main.c)
    target_include_directories(rk_irfpa_simple_demo PRIVATE rk_irfpa)
    target_link_libraries(rk_irfpa_simple_demo PRIVATE rkirfpa)

    install(TARGETS rkirfpa LIBRARY DESTINATION lib)
    install(TARGETS rk_irfpa_simple_demo DESTINATION bin)

    install(DIRECTORY "${RK_IRFPA_SDK_PATH}/rk_irfpa"
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")

    install(DIRECTORY "${RK_IRFPA_SDK_PATH}/rk_irfpa/iqfiles"
        DESTINATION usr/share/iqfiles/rk_irfpa
        FILES_MATCHING PATTERN "*.json")
endif()

