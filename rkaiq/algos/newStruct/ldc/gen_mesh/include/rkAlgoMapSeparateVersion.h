#ifndef __RKALGO_MAP_SEPARATE_VERSION_H__
#define __RKALGO_MAP_SEPARATE_VERSION_H__

/*!
 * ==================== VERSION HISTORY ====================
 *
* v1.0.0
*  - date: 2023-07-07
*  - content:
                1. base version of rkAlgoMapSeparateTool

* v1.0.1
*  - date: 2023-10-09
*  - content:
                1. add macro definition "RKALGO_DEBUG"

* v1.0.2
*  - date: 2024-01-15
*  - content:
                1. fix the code in genSeparateMeshFromOriMap and genSeparateBigMapFromOriMap to
avoid value "-1" in mapxNew

* v1.0.3
*  - date: 2024-04-26
*  - content:
                1. distinguishing C and C++ using macro definitions "__cplusplus" in
./rkAlgoMapSeparate/rkAlgoMapSeparate.h
                2. distinguishing C and C++ codes using 2 subdirectorys with 2 sets of codes,
                   having the same codes but different files' suffixes(.c/.cpp):
                    ./code_c
                    ./code_cxx
                3. add compile scripts:
                    linux_c_build.sh
                    linux_cxx_build.sh
                    android_arm64-v8a_c_build.sh
                    android_arm64-v8a_cxx_build.sh
                    android_armeabi-v7a_c_build.sh
                    android_armeabi-v7a_cxx_build.sh
                    all_android_linux_build.sh
                    win64_vs2017_c_build.bat
                    win64_vs2017_cxx_build.bat

*/

/* 版本模式 */
#ifdef _DEBUG
#define MAP_SEPARATE_VER_MODE "debug"
#elif NDEBUG
#define MAP_SEPARATE_VER_MODE "release"
#endif

#define MAP_SEPARATE_VER_NUM "rkAlgoMapSeparate Version: 1.0.3"

#define MAP_SEPARATE_VER_DATE "date: 2024-04-26"

/* 版本号字符串：lib版本号.编译日期.版本模式 */
#define RKALGO_MAP_SEPARATE_VER \
    (MAP_SEPARATE_VER_NUM " " MAP_SEPARATE_VER_DATE " " MAP_SEPARATE_VER_MODE)

#endif  // !__RKALGO_MAP_SEPARATE_VERSION_H__
