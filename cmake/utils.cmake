FUNCTION(im2d_api_extract_version)
    FILE(READ "${CMAKE_CURRENT_LIST_DIR}/im2d_api/im2d_version.h" version_file)

    STRING(REGEX MATCH "RGA_API_MAJOR_VERSION       ([0-9]+)" _  "${version_file}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract major version number from im2d_version.h")
    endif()
    set(VER_MAJOR ${CMAKE_MATCH_1})

    STRING(REGEX MATCH "RGA_API_MINOR_VERSION       ([0-9]+)" _  "${version_file}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract minor version number from im2d_version.h")
    endif()
    set(VER_MINOR ${CMAKE_MATCH_1})

    STRING(REGEX MATCH "RGA_API_REVISION_VERSION    ([0-9]+)" _  "${version_file}")
    if(NOT CMAKE_MATCH_COUNT EQUAL 1)
        message(FATAL_ERROR "Could not extract revision version number from im2d_version.h")
    endif()
    set(VER_REVISION ${CMAKE_MATCH_1})

    set(IM2D_API_VERSION_MAJOR ${VER_MAJOR} PARENT_SCOPE)
    set(IM2D_API_VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}" PARENT_SCOPE)
ENDFUNCTION()
