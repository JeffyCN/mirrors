#!/bin/bash

cmake -D CMAKE_C_COMPILER=/home/lili/hdd/ToolChains/aarch64-rockchip1031-linux-gnu/bin/aarch64-none-linux-gnu-gcc \
-D CMAKE_CXX_COMPILER=/home/lili/hdd/ToolChains/aarch64-rockchip1031-linux-gnu/bin/aarch64-none-linux-gnu-g++ \
CMakeLists.txt
