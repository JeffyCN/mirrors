#!/bin/bash

ANDROID_NDK_HOME=/Users/jacobchen/Library/Android/sdk/ndk/21.3.6528147
CMAKE_ANDROID=/Users/jacobchen/Library/Android/sdk/cmake/3.6.4111459/bin/cmake

${CMAKE_ANDROID} -DCMAKE_BUILD_TARGET=android_ndk -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_NDK=$ANDROID_NDK_HOME \
      -DANDROID_ABI=armeabi-v7a \
      -DANDROID_TOOLCHAIN=clang \
      -DANDROID_PLATFORM=android-27 \
      -DANDROID_STL=c++_shared \
	  ..
make
