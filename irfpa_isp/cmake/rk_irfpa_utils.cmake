macro(assert VAR MSG)
    if (NOT ${VAR})
        message(FATAL_ERROR "${MSG}")
    endif ()
endmacro()

function(rk_irfpa_add_subdirectory)
    file(
        GLOB subdir
        LIST_DIRECTORIES false
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/*/CMakeLists.txt)

    foreach(dir ${subdir})
        get_filename_component(dir ${dir} DIRECTORY)
        add_subdirectory(${dir})
    endforeach()
endfunction()

function(rk_irfpa_add_all_sources target)
    file(GLOB srcs
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}
        "*.cpp" "*.c")

    foreach(src ${srcs})
        target_sources(${target} INTERFACE ${src})
    endforeach()
    target_include_directories(${target} INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endfunction()

function(rk_irfpa_add_all_sources_private target)
    file(GLOB srcs
        RELATIVE ${CMAKE_CURRENT_LIST_DIR}
        "*.cpp" "*.c")

    foreach(src ${srcs})
        target_sources(${target} PRIVATE ${src})
    endforeach()
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endfunction()

function(rk_irfpa_add_simple_libaray target)
    #get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    add_library(${target} INTERFACE)
    rk_irfpa_add_all_sources(${target})
endfunction()

