set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


if(ARCH STREQUAL "arm")
    add_compile_options(-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    add_link_options(-march=armv8-a -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
endif()

if(ARCH STREQUAL "aarch64")
    add_compile_options(-march=armv8-a -mtune=cortex-a53)
    add_link_options(-march=armv8-a -mtune=cortex-a53)
endif()


add_compile_options(-fopenmp -ftree-vectorize -funroll-loops)
add_link_options(-fopenmp -ftree-vectorize -funroll-loops)
