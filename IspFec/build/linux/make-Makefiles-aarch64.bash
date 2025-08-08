#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
export AIQ_BUILD_HOST_DIR=/data/project_codes/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu
export AIQ_BUILD_TOOLCHAIN_TRIPLE=aarch64-linux-gnu
export AIQ_BUILD_SYSROOT=libc
export AIQ_BUILD_ARCH=aarch64
TOOLCHAIN_FILE=$(pwd)/../../cmake/toolchains/gcc.cmake
OUTPUT=$(pwd)/output/${AIQ_BUILD_ARCH}
SOURCE_PATH=$(pwd)/../../

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DRKFEC_TARGET_SOC=${RKFEC_TARGET_SOC} \
    -DRKFEC_HW_VERSION=${RKFEC_HW_VERSION} \
    -DARCH=${AIQ_BUILD_ARCH} \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DCMAKE_INSTALL_PREFIX="installed" \
    -DRK_GDC_GEN_MESH_ONLINE=ON \
    -DENABLE_SAMPLE_ISPFEC=ON \
    -DENABLE_SAMPLE_FEC_FOR_AVSP=OFF \
    $SOURCE_PATH \
&& ninja -j$(nproc) \
&& ninja install

popd
