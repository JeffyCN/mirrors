option(OPTION_BUILD_STATIC "if build algo static library" OFF)

if (NOT TARGET rk_irfpa_sdk_init_marker)
    add_library(rk_irfpa_sdk_init_marker INTERFACE)

    if (NOT RK_IRFPA_SDK_PATH)
        set(RK_IRFPA_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
    endif()
    list(APPEND CMAKE_MODULE_PATH ${RK_IRFPA_SDK_PATH}/cmake)

    include(rk_irfpa_utils)

    if (NOT RK_IRFPA_PLATFORM)
        set(RK_IRFPA_PLATFORM rv1126b)
    endif()

    if(NOT ARCH)
        set(ARCH "arm")
    endif()

    if (CMAKE_TOOLCHAIN_FILE)
        message("using toolchain file ${CMAKE_TOOLCHAIN_FILE}")
        include(${CMAKE_TOOLCHAIN_FILE})
    endif()
    
    message("select platform file platform_${RK_IRFPA_PLATFORM}")
    include(platform_${RK_IRFPA_PLATFORM})

    add_compile_options(-fPIC)
    add_definitions(-DIRFPA_EVB=1)
    add_subdirectory(rk_irfpa)
endif()
