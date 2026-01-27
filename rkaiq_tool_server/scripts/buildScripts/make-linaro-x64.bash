#!/bin/bash
cmake -D CMAKE_C_COMPILER=/home/lili/hdd/ToolChains/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc \
-D CMAKE_CXX_COMPILER=/home/lili/hdd/ToolChains/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++ \
CMakeLists.txt
