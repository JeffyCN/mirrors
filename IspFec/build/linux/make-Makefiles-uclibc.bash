#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
export AIQ_BUILD_HOST_DIR=/data/project_codes/arm-rockchip830-linux-uclibcgnueabihf
export AIQ_BUILD_TOOLCHAIN_TRIPLE=arm-rockchip830-linux-uclibcgnueabihf
export AIQ_BUILD_SYSROOT=sysroot
export AIQ_BUILD_ARCH=arm
TOOLCHAIN_FILE=$(pwd)/../../cmake/toolchains/gcc.cmake
OUTPUT=$(pwd)/output/${AIQ_BUILD_ARCH}
SOURCE_PATH=$(pwd)/../../

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DARCH=${AIQ_BUILD_ARCH} \
    -DRKPLATFORM=OFF \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DRKFEC_TARGET_SOC=${RKFEC_TARGET_SOC} \
    -DRKFEC_HW_VERSION=${RKFEC_HW_VERSION} \
    -DCMAKE_INSTALL_PREFIX="installed" \
    $SOURCE_PATH \
&& ninja -j$(nproc) \
&& ninja install

popd
