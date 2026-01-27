cmake_minimum_required( VERSION 2.8.8 )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# MODULE IPCS SOCK
option(USE_IPCS_SOCK "enable ipc by socket" ON)
if(USE_IPCS_SOCK)
    include_directories(sock/include)
    add_definitions(-DUSE_IPCS_SOCK)
endif()

# MODULE IPCS
set(SOURCE_FILES_RK_SYS_IPCS
    c_rk_ipcs_request.c
    c_rk_ipcs_response.c
    c_rk_ipcs_service.c
)

if(USE_IPCS_SOCK)
set(SOURCE_FILES_RK_SYS_IPCS
    ${SOURCE_FILES_RK_SYS_IPCS}
    sock/c_rk_ipcs_sock_server.c
)
endif()


add_definitions(-Wno-attributes)
add_library(${AIQ_IPCS_STATIC} STATIC ${SOURCE_FILES_RK_SYS_IPCS})
set_target_properties(${AIQ_IPCS_STATIC} PROPERTIES FOLDER "aiq_ipcs")
message(STATUS "cmake version ${CMAKE_VERSION} [@@]config library: aiq_ipcs")
