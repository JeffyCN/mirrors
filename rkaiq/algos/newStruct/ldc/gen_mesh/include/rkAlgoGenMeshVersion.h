#ifndef __RK_ALGO_GEN_MESH_VERSION_H__
#define __RK_ALGO_GEN_MESH_VERSION_H__

/*!
 * ==================== VERSION HISTORY ====================
 *
* v1.0.0
*  - date: 2023-02-16
*  - content:
                1. base version of rkAlgoGenMeshTool
                2. generate float mesh
                3. generate fixed mesh from float mesh for RK FEC & LDCH

* v1.0.1
*  - date: 2023-03-06
*  - content:
                1. add function(s): genLdchMeshZeroCorrect

* v1.0.2
*  - date: 2023-05-24
*  - content:
                1. some adjustment about genFloatMeshFromOpencvPinholeModel:
                   (1) interface: add newCameraMatrix as input for vitual camera Intrisic matrix
                   (2) fix a bug in loop about _x, _y, _w

* v1.0.3
*  - date: 2023-05-26
*  - content:
                1. some adjustment about genFecMeshFromBigMap:
                   (1) fix a bug about lowerBoundary and upperBoundary

* v1.0.4
*  - date: 2023-07-07
*  - content:
                1. some adjustment about genLdchMeshFromFloatMesh: change fixBit value according
srcW range
                2. some adjustment about genFloatMeshFromOpencvPinholeModel: cancel the limit of x
and y coordinate for float map(mesh)
                2. add function(s): calcLdchLdcvMeshSize(...), genLdchLdcvMeshFromSeparateMesh(...),
genLdchLdcvMeshFromSeparateBigMap(...)
                3. add rkAlgoMapSeparateLib

* v1.0.5
*  - date: 2023-07-12
*  - content:
                1. some adjustment about calcFloatMeshSizeCV and genFloatMeshFromOpencvPinholeModel
                2. some adjustment about calcLdchLdcvMeshSize and genLdchLdcvMeshFromFloatMesh
                3. there is 3 usages about generate LDCH/LDCV mesh, see test003.cpp for demo codes:
                   (1)LDCH on  + LDCV on
                   (2)LDCH off + LDCV on
                   (3)LDCH on  + LDCV off

* v1.0.6
*  - date: 2023-07-25
*  - content:
                1. add function(s): genFloatMeshNLevel(...) and some related functions
                2. some adjustment about other functions
                3. there is 3 usages about generate LDCH/LDCV mesh, see test006.cpp for demo codes:
                   (1)LDCH on  + LDCV on
                   (2)LDCH off + LDCV on
                   (3)LDCH on  + LDCV off

* v1.0.7
*  - date: 2023-10-09
*  - content:
                1. some adjustment about folder structure
                2. remove the dependency on OpenCV from the codes, except for the codes in the
"rkAlgoGenFloatMeshCV" folder

* v1.0.8
*  - date: 2023-10-27
*  - content:
                1. some adjustment about genLdchMeshFromFloatMesh
                (1) fix a bug about xVal in genLdchMeshFromFloatMesh
                2. add some demo codes for 0-255 level correction, see about test004.cpp,
test005.cpp, and test006.cpp

* v1.0.9
*  - date: 2023-12-28
*  - content:
                1. remove the dependency on OpenCV from the codes in function:
genLdchLdcvMeshFromFloatMesh()

* v1.1.0
*  - date: 2024-01-18
*  - content:
                1. add ldcvAlignMeshW in struct "RKALGO_LDCH_LDCV_INFO_S" for LDCV mesh to do 2
bytes align

* v1.1.1
*  - date: 2024-03-15
*  - content:
                1. some adjustment about codes depended on OpenCV: only used and compiled on Windows
platform
                2. add a new method for generate LDCH+LDCV mesh avoid generating full resolution
mesh
                3. add function(s): floatMeshUpSample in rkAlgoGenFloatMeshComm.cpp
                4. some adjustment about calcFloatMeshSizeRK

* v1.1.2
*  - date: 2024-04-26
*  - content:
                1. add files: rkAlgoPolynomial.c/cpp and rkAlgoPolynomial.h for solving polynomial
functions and fitting polynomial curves
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
#define GEN_MESH_VER_MODE "debug"
#elif NDEBUG
#define GEN_MESH_VER_MODE "release"
#endif

#define GEN_MESH_VER_NUM "rkAlgoGenMesh Version: 1.1.2"

#define GEN_MESH_VER_DATE "date: 2024-04-26"

/* 版本号字符串：lib版本号.编译日期.版本模式 */
#define RKALGO_GEN_MESH_TOOL_VER (GEN_MESH_VER_NUM " " GEN_MESH_VER_DATE " " GEN_MESH_VER_MODE)

#endif  // !__RK_ALGO_GEN_MESH_VERSION_H__
