cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# MODULE INFO
set(SOURCE_FILES_RK_SYS_INFO
    rk_info_utils.c
    c_rk_info_service_control.c
    rk_cmd_service_control.c
    st_string.c
)

add_library(${AIQ_INFO_STATIC} STATIC ${SOURCE_FILES_RK_SYS_INFO})
set_target_properties(${AIQ_INFO_STATIC} PROPERTIES FOLDER "aiq_info")
message(STATUS "cmake version ${CMAKE_VERSION} [@@]config library: aiq_info")
