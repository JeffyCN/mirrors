#!/bin/bash

PLATFORM="Ingenic"
TOOLCHAIN_PATH="/home/linmude/Workspaces/Ingenic/Toolchain/mips-gcc472-glibc216-64bit/bin/"

if [ -n "$1" ];
then
	PLATFORM="$1"
fi

if [ -n "$2" ];
then
	TOOLCHAIN_PATH="$2"
fi

if [ $PLATFORM == "Ingenic" ];
then
	CC="mips-linux-gnu-gcc"
	AR="mips-linux-gnu-ar cr"
	CC_PATH=${TOOLCHAIN_PATH}${CC}
	AR_PATH=${TOOLCHAIN_PATH}${AR}
	echo ${TOOLCHAIN_PATH}
	echo ${CC_PATH}
elif [ $PLATFORM == "Rockchip" ];
then
	CC="arm-linux-gnueabihf-gcc"
	AR="arm-linux-gnueabihf-ar cr"
	CC_PATH=${TOOLCHAIN_PATH}${CC}
	AR_PATH=${TOOLCHAIN_PATH}${AR}
else
	echo "Unsupported platform."
	exit
fi

## For build library internally.
if [ ! -f libwl.a ];
then
	cd ./libwl
	make CC="$CC_PATH" AR="$AR_PATH" CONFIG_PLATFORM="$PLATFORM"
	cd ..
	cp ./libwl/libwl.a ./
fi

if [ ! -f wl.h ];
then
	cp ./libwl/wl.h ./
fi

## Build Application
make CC="$CC_PATH" CONFIG_PLATFORM="$PLATFORM"
cd ..
