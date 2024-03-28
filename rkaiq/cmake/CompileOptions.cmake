set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -fPIC")
set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -gdwarf -fexceptions -funwind-tables")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -gdwarf -fexceptions -funwind-tables")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (ARCH STREQUAL "arm")
    add_compile_options(
        -march=armv7-a
        )
endif()

if (ARCH STREQUAL "aarch64")
    add_compile_options(
        -march=armv8-a
        )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (ARCH STREQUAL "arm")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb -mthumb-interwork")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb -mthumb-interwork")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 8.3 OR GCC_VERSION VERSION_EQUAL 8.3))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 8.3 or greater.")
    endif ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,-Map,librkaiq.map")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/librkaiq.version")

    # Flags that affects code size
    #if (NOT ARCH STREQUAL "arm")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-clash-protection")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -fstack-protector-strong")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -fstack-clash-protection")
        string(FIND ${CMAKE_CXX_FLAGS} "-D_FORTIFY_SOURCE=" RKAIQ_CXX_FLAGS)
        if(RKAIQ_CXX_FLAGS LESS 0)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
            set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -D_FORTIFY_SOURCE=2")
        endif()
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro")
    #endif (NOT ARCH STREQUAL "arm")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -fgnuc-version=8.3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fgnuc-version=8.3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -fstack-protector-strong")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,-Map,librkaiq.map")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--push-state -Wl,--no-as-needed -Wl,--pop-state")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--push-state -Wl,--as-needed -latomic -Wl,--pop-state")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld -Wl,--icf=all -Wl,--gdb-index")
    if (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Android"))
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/librkaiq.version")
    else()
        add_compile_options(-DANDROID_OS
                            -DCMAKE_BUILD_ANDROID=1)
        add_compile_options(-Wno-unused-but-set-variable
                            -Wno-unused-variable
                            -Wno-unused-label
                            -Wno-implicit-const-int-float-conversion)

    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /stdlib=libc++")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

# Workaround Compile Errors
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(
        -fwrapv
        -Wformat-security
        )
    add_compile_options(
        -Wno-psabi
        -Wno-unused
        -Wno-unused-result
        )
    if (GCC_VERSION VERSION_GREATER 9 OR GCC_VERSION VERSION_EQUAL 9)
        add_compile_options(
            -Wno-address-of-packed-member
            )
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(-D__ARM_NEON)
    add_compile_options(
        -fwrapv
        -Wformat-security
        )
    add_compile_options(
        -Wno-unused-private-field
        -Wno-extern-c-compat
        -Wno-c99-designator
        -Wno-unused-function
        -Wno-unused-variable
        -Wno-address-of-packed-member
        -Wno-unused-parameter
        )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (RKAIQ_ENABLE_ASAN)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_LINKER_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address")
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)
