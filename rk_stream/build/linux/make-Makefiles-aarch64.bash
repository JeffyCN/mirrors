#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
export AIQ_BUILD_HOST_DIR=/home/camera/camera/rk356x_linux/buildroot/output/rockchip_rk3566/host

if [[ "$(pwd)" == *"camera_engine_rkaiq"* ]]; then
    echo "It's camera_engine_rkaiq."
    aiq_dir=..
else
    echo "It's rk1109_isp_simulator."
    aiq_dir=../rk_aiq
fi

TOOLCHAIN_FILE=$(pwd)/../../${aiq_dir}/cmake/toolchains/aarch64_linux_buildroot.cmake
SOURCE_PATH=$(pwd)/../../
OUTPUT=$(pwd)/output/aarch64

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo\
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DAIQ_DIR=${aiq_dir} \
    $SOURCE_PATH \
&& ninja -j$(nproc)

popd
