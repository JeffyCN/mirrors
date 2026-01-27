# - Check glibc version
# CHECK_GLIBC_VERSION()
#
# Once done this will define
#
#   GLIBC_VERSION - glibc version
#
macro (CHECK_GLIBC_VERSION)
    execute_process (
        COMMAND ${CMAKE_C_COMPILER} -print-file-name=ld-uClibc.so.1
        OUTPUT_VARIABLE UCLIBC
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(C_LIBRARY_NAME "clang")
        message (STATUS "it is ${C_LIBRARY_NAME}")
    else ()
        if (EXISTS "${UCLIBC}")
            set(C_LIBRARY_NAME "uclibc")
            message (STATUS "it is ${C_LIBRARY_NAME}")
        else()
            set(C_LIBRARY_NAME "glibc")
            message (WARNING "assumming it is ${C_LIBRARY_NAME}")
        endif (EXISTS "${UCLIBC}")
    endif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
endmacro (CHECK_GLIBC_VERSION)
