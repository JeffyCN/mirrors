find_package(PkgConfig QUIET)
pkg_check_modules(libdl QUIET "libdl")

include(FindPackageHandleStandardArgs)
find_path(LIBDL_INCLUDE_DIR
    NAMES
    dlfcn.h
    PATH
    include
)
find_library(LIBDL_LIBRARY
    dl
    PATH_SUFFIXES
    lib64
    lib
)
find_package_handle_standard_args(libdl
    DEFAULT_MSGS
    LIBDL_INCLUDE_DIR
    LIBDL_LIBRARY
)
mark_as_advanced(
    LIBDL_INCLUDE_DIR
    LIBDL_LIBRARY
)

if (libdl_FOUND)
    set(LIBDL_LIBRARIES    ${LIBDL_LIBRARY})
    set(LIBDL_INCLUDE_DIRS ${LIBDL_INCLUDE_DIR})
    set(LIBDL_CFLAGS "-DHAS_LIBDL")

    if (LIBDL_LIBRARY AND NOT TARGET libdl::libdl)
        if(IS_ABSOLUTE "${LIBDL_LIBRARY}")
            add_library(libdl::libdl UNKNOWN IMPORTED)
            set_target_properties(libdl::libdl PROPERTIES IMPORTED_LOCATION
                "${LIBDL_LIBRARY}")
        else()
            add_library(libdl::libdl INTERFACE IMPORTED)
            set_target_properties(libdl::libdl PROPERTIES IMPORTED_LIBNAME
                "${LIBDL_LIBRARY}")
        endif()
        set_target_properties(libdl::libdl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
            "${LIBDL_INCLUDE_DIR}")
    endif()
endif()
