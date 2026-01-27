#!/bin/bash
cmake -D CMAKE_C_COMPILER=/home/lili/hdd/ToolChains/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc \
-D CMAKE_CXX_COMPILER=/home/lili/hdd/ToolChains/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++ \
CMakeLists.txt
