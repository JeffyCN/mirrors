#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
export AIQ_BUILD_HOST_DIR=/data/project_codes/arm-rockchip830-linux-uclibcgnueabihf
export AIQ_BUILD_TOOLCHAIN_TRIPLE=arm-rockchip830-linux-uclibcgnueabihf
export AIQ_BUILD_SYSROOT=sysroot
export AIQ_BUILD_ARCH=arm

if [[ "$(pwd)" == *"camera_engine_rkaiq"* ]]; then
    echo "It's camera_engine_rkaiq."
    aiq_dir=../rkaiq
else
    echo "It's rk1109_isp_simulator."
    aiq_dir=../rk_aiq
fi

TOOLCHAIN_FILE=$(pwd)/../../${aiq_dir}/cmake/toolchains/gcc.cmake
SOURCE_PATH=$(pwd)/../../
OUTPUT=$(pwd)/output/${AIQ_BUILD_ARCH}

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo\
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DAIQ_DIR=${aiq_dir} \
    -DRKAIQ_TARGET_SOC=${RKAIQ_TARGET_SOC} \
    -DARCH=${AIQ_BUILD_ARCH} \
    -DRKVIDEMO_HAVE_AIQ=ON \
    -DRKAIQ_TEST=ON \
    $SOURCE_PATH \
&& ninja -j$(nproc)

popd
