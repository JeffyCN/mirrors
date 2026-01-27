# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

if(ARCH STREQUAL "arm")
# which compilers to use for C and C++
set(CMAKE_C_COMPILER   /data/project_codes/arm-rockchip1240-linux-gnueabihf/bin/arm-rockchip1240-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /data/project_codes/arm-rockchip1240-linux-gnueabihf/bin/arm-rockchip1240-linux-gnueabihf-g++)
# where is the target environment located
set(CMAKE_FIND_ROOT_PATH  /data/project_codes/arm-rockchip1240-linux-gnueabihf)
endif()

if(ARCH STREQUAL "aarch64")
set(CMAKE_C_COMPILER   /data/project_codes/aarch64-rockchip1240-linux-gnu/bin/aarch64-rockchip1240-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /data/project_codes/aarch64-rockchip1240-linux-gnu/bin/aarch64-rockchip1240-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH  /data/project_codes/aarch64-rockchip1240-linux-gnu)
endif()

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
