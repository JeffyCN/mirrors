cmake_minimum_required( VERSION 2.8.8 )

set(SOURCE_FILES_CJSON
    c_RTJsonParser.c
)

add_library(${LIB_STATIC_CJSON} STATIC ${SOURCE_FILES_CJSON})
set_target_properties(${LIB_STATIC_CJSON} PROPERTIES FOLDER "third-party/cjson")

message(STATUS "cmake version ${CMAKE_VERSION} [@@]config library: third-party/cjson")
