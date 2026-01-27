add_library(rk_irfpa_algos INTERFACE)
find_program(Python_EXECUTABLE python NO_CMAKE_FIND_ROOT_PATH)

function (rk_irfpa_algo_add_static_lib target)
    file(GLOB srcs
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}
        "*.cpp" "*.c")

    add_library(${target} STATIC ${srcs})
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/algos")
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/algos/sw_drc")
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/algos/hw_ynr")
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/isphw")
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/third/cJSON")
    target_include_directories(${target} PRIVATE "${RK_IRFPA_SDK_PATH}/rk_irfpa/common")
endfunction()

function(rk_irfpa_add_algo)
    get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(target "rk_irfpa_${CURRENT_FOLDER}")
    add_library(${target} INTERFACE)
    target_link_libraries(rk_irfpa_algos INTERFACE ${target})
    target_include_directories(${target} INTERFACE "${RK_IRFPA_SDK_PATH}/rk_irfpa/algos")
    target_sources(${target} INTERFACE "${RK_IRFPA_SDK_PATH}/rk_irfpa/algos/interpolation.c")

    set(stalib _${target}_${ARCH})

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib${stalib}.a")
        message("${target}: using static library")
        target_link_libraries(${target} INTERFACE ${stalib})
        target_link_directories(${target} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
        target_include_directories(${target} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
    else()
        message("${target}: using source code")
        rk_irfpa_add_all_sources(${target})
        target_link_libraries(${target} INTERFACE rk_irfpa_common)
        if (OPTION_BUILD_STATIC)
            rk_irfpa_algo_add_static_lib(${stalib})
        endif()
    endif()
endfunction()

if (OPTION_ISP_INTERNAL)
    file(GLOB params
        LIST_DIRECTORIES false
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/*/*_params.h)

    foreach(hdr ${params})
        execute_process(COMMAND
            ${Python_EXECUTABLE} 
            ${RK_IRFPA_SDK_PATH}/rk_irfpa/algos/autoconfig.py 
            ${RK_IRFPA_SDK_PATH}/rk_irfpa/algos/${hdr}
        )
    endforeach()
endif()

rk_irfpa_add_subdirectory()
