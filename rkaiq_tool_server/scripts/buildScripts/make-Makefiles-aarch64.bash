#!/bin/bash

export TOPDIR=$(pwd)
export CURDIR=$TOPDIR/scripts/build
#export AIQ_BUILD_HOST_DIR=/home/camera/camera/rk356x_linux/buildroot/output/rockchip_rk3566/host
export AIQ_BUILD_HOST_DIR=/home/lili/hdd/projects/3588-linux-release/buildroot/output/rockchip_rk3588/host
TOOLCHAIN_FILE=$CURDIR/aarch64_linux_buildroot.cmake
#TOOLCHAIN_FILE=$AIQ_BUILD_HOST_DIR/share/buildroot/toolchainfile.cmake
export OUTPUT=$TOPDIR/output/aarch64
export SOURCE_PATH=$TOPDIR

mkdir -p $OUTPUT
pushd $OUTPUT

cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=debug \
    -DARCH="aarch64" \
    -DRKPLATFORM=OFF \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE \
    -DCMAKE_SKIP_RPATH=TRUE \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
    $SOURCE_PATH \
&& make -j8

popd

#python3 ./scripts/run-clang-tidy.py '' output/aarch64 scripts,rkmedia c,h,C,H,cpp,hpp,cc,hh,c++,h++,cxx,hxx > clang-tidy-log.txt 2>&1
