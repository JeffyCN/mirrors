#!/bin/bash
# Run this from within a bash shell
# x86_64 is for simulation do not enable RK platform
#export AIQ_BUILD_HOST_DIR=/data/project_codes/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu
#export AIQ_BUILD_TOOLCHAIN_TRIPLE=aarch64-linux-gnu
export AIQ_BUILD_HOST_DIR=/data/project_codes/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu
export AIQ_BUILD_TOOLCHAIN_TRIPLE=aarch64-none-linux-gnu
export AIQ_BUILD_SYSROOT=libc
if [ "${RKAIQ_TARGET_SOC}" == "rv1126b" ];then
	export AIQ_BUILD_HOST_DIR=/data/project_codes/aarch64-rockchip1240-linux-gnu
	echo "${AIQ_BUILD_HOST_DIR}"
export AIQ_BUILD_TOOLCHAIN_TRIPLE=aarch64-rockchip1240-linux-gnu
export AIQ_BUILD_SYSROOT=sysroot
fi
export AIQ_BUILD_ARCH=aarch64
TOOLCHAIN_FILE=$(pwd)/../../cmake/toolchains/gcc.cmake
OUTPUT=$(pwd)/output/${AIQ_BUILD_ARCH}
SOURCE_PATH=$OUTPUT/../../../../

mkdir -p $OUTPUT
pushd $OUTPUT
#if make aiq with raw stream lib, modify -DUSE_RAWSTREAM_LIB value to ON, default value OFF 
cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DRKAIQ_TARGET_SOC=${RKAIQ_TARGET_SOC} \
    -DARCH=${AIQ_BUILD_ARCH} \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DRKAIQ_BUILD_BINARY_IQ=ON \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    -DISP_HW_VERSION=${ISP_HW_VERSION} \
    -DCMAKE_INSTALL_PREFIX="installed" \
    -DRKAIQ_USE_RAWSTREAM_LIB=OFF \
    -DRKAIQ_HAVE_FAKECAM=ON \
    -DRKAIQ_ENABLE_AF=ON \
    $SOURCE_PATH \
&& ninja -j$(nproc) \
&& ninja install

status_code=$?

popd

exit $status_code
