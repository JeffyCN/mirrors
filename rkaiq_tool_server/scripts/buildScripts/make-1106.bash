#!/bin/bash

cmake -D CMAKE_C_COMPILER=${PWD}/../../../../tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc \
-D CMAKE_CXX_COMPILER=${PWD}/../../../../tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-g++ \
CMakeLists.txt

