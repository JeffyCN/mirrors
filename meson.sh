#!/bin/bash

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
SOURCE_PATH=${SCRIPT_DIR}

# Modify to the local toolchain path.
TOOLCHAIN_PATH=cross/cross_file_aarch64.txt
BUILD_PATH=build/meson_aarch64
INSTALL_PATH="${SOURCE_PATH}/${BUILD_PATH}/install"

rm -rf ${BUILD_PATH}

meson ${BUILD_PATH} --prefix=${INSTALL_PATH} --cross-file=${TOOLCHAIN_PATH}

ninja -C ${BUILD_PATH} install
