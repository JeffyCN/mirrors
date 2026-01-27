#ifndef __RK_ALGO_GEN_MESH_VERSION_H__
#define __RK_ALGO_GEN_MESH_VERSION_H__
// clang-format off

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
                1. some adjustment about genLdchMeshFromFloatMesh: change fixBit value according srcW range
                2. some adjustment about genFloatMeshFromOpencvPinholeModel: cancel the limit of x and y coordinate for float map(mesh)
                2. add function(s): calcLdchLdcvMeshSize(...), genLdchLdcvMeshFromSeparateMesh(...), genLdchLdcvMeshFromSeparateBigMap(...)
                3. add rkAlgoMapSeparateLib

* v1.0.5
*  - date: 2023-07-12
*  - content:
                1. some adjustment about calcFloatMeshSize and genFloatMeshFromOpencvPinholeModel
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

*/

/* 版本模式 */
#ifdef _DEBUG
#define GEN_MESH_VER_MODE "debug"
#elif NDEBUG
#define GEN_MESH_VER_MODE "release"
#endif

#define GEN_MESH_VER_NUM "rkAlgoGenMesh Version: 1.0.6"

#define GEN_MESH_VER_DATE "date: 2023-07-25"

/* 版本号字符串：lib版本号.编译日期.版本模式 */
#define RKALGO_GEN_MESH_TOOL_VER (GEN_MESH_VER_NUM " " GEN_MESH_VER_DATE " " GEN_MESH_VER_MODE)

// clang-format on
#endif  // !__RK_ALGO_GEN_MESH_VERSION_H__
