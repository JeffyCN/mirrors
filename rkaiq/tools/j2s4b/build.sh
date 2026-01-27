#!/bin/bash

usage()
{
    echo "USAGE: [-U] [-CK] [-A] [-p] [-o] [-u] [-v VERSION_NAME] "
    echo "No ARGS means use default build option                  "
    echo "WHERE: -C = build cross                                 "
    echo "WHERE: -R = rebuild                                     "
    exit 1
}

function clean_cmake_config()
{
    rm -rf CMakeCache.txt
    rm -rf CMakeFiles
    rm -rf cmake_install.cmake
    rm -rf Makefile
    rm -rf CTestTestfile.cmake
}

BUILD_DIR=output
BUILD_CLEAN=false
BUILD_CROSS=false

# check pass argument
while getopts "RC" arg
do
    case $arg in
        R)
            echo "will reconfigure project"
            BUILD_CLEAN=true
            ;;
        C)
            echo "will build cross platform"
            BUILD_CROSS=true
            ;;
        ?)
            usage ;;
    esac
done

if [ ! -d "$BUILD_DIR/release" ]; then
    mkdir -p "$BUILD_DIR/release"
fi

# build clean
if [ "$BUILD_CLEAN" = true ] ; then
    rm "${BUILD_DIR:?}/*" -rf
fi

cd $BUILD_DIR || exit

# build cross
if [ "$BUILD_CROSS" = true ] ; then
    echo "start build cross paltform"
    cmake -DCMAKE_TOOLCHAIN_FILE=toolchainfile.cmake \
        -DCMAKE_INSTALL_PREFIX=./release ../
else
    cmake -DCMAKE_INSTALL_PREFIX=./release -DROOT_PROJ_BIN_DIR:STRING="../../build/linux/output/arm" \
        -DRKAIQ_IQFILES_DIR:STRING="../../iqfiles/isp32"  ../
fi

make -j8

if [ $? -eq 0  ]; then
    echo Build finished!
    make install
    clean_cmake_config
    cd ..
fi

