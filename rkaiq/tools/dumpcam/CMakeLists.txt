cmake_minimum_required(VERSION 2.8)

if(RKAIQ_HAVE_DUMPSYS)

add_compile_options(-std=c++11 -Wno-psabi)

option(USE_IPCS_SOCK_UNIX_DOMAIN "enable unix domain socket" OFF)
if(USE_IPCS_SOCK_UNIX_DOMAIN)
    add_definitions(-DUSE_IPCS_SOCK_UNIX_DOMAIN)
endif()

include_directories(include)
include_directories(utils/include)

set(SOURCE
    main.cpp
    DumpSys.cpp
    DumpClient.cpp
    DumpMemory.cpp
    DumpVersion.cpp
    utils/stringprintf.cpp
    utils/binutils.cpp
)

ADD_EXECUTABLE(dumpcam ${SOURCE})
install(TARGETS dumpcam DESTINATION bin)

endif()
