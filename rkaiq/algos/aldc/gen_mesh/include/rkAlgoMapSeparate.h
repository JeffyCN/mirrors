#ifndef __RKALGO_MAP_SEPARATE_H__
#define __RKALGO_MAP_SEPARATE_H__
// clang-format off

#include "rkAlgoMapSeparateVersion.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <string>
#include <algorithm>


#if defined(_WIN32) || defined(__CYGWIN__)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif


#ifndef CLIP
#define CLIP(a, min, max) (((a) < (min)) ? (min) : (((a) > (max)) ? (max) : (a)))
#endif

/*
* @struct   RKALGO_SEPARATE_MESH_INFO_S
* @brief    define the info of map separation for RK LDCH and LDCV
*/
typedef struct RKALGO_SEPARATE_MESH_INFO
{
    uint32_t srcW;                                          /* 输入图像宽 */
    uint32_t srcH;                                          /* 输入图像高 */
    uint32_t dstW;                                          /* 输出图像宽 */
    uint32_t dstH;                                          /* 输出图像高 */
    uint32_t mapWidAlign;                                   /* 原始映射表的宽，需要16对齐再 + 1 */
    uint32_t mapHgtAlign;                                   /* 原始映射表的高，需要16对齐再 + 1 */
    /* LDCH下采样浮点mesh相关信息 */
    uint32_t ldchFloatMeshStepW;                            /* LDCH下采样浮点mesh宽步长 */
    uint32_t ldchFloatMeshStepH;                            /* LDCH下采样浮点mesh高步长 */
    uint32_t ldchFloatMeshW;                                /* LDCH下采样浮点mesh表的宽 */
    uint32_t ldchFloatMeshH;                                /* LDCH下采样浮点mesh表的高 */
    uint64_t u64LdchFloatMeshBufSize;                       /* LDCH下采样浮点mesh表的buffer大小 */
    /* LDCV下采样浮点mesh相关信息 */
    uint32_t ldcvFloatMeshStepW;                            /* LDCV下采样浮点mesh宽步长 */
    uint32_t ldcvFloatMeshStepH;                            /* LDCV下采样浮点mesh高步长 */
    uint32_t ldcvFloatMeshW;                                /* LDCV下采样浮点mesh表的宽 */
    uint32_t ldcvFloatMeshH;                                /* LDCV下采样浮点mesh表的高 */
    uint64_t u64LdcvFloatMeshBufSize;                       /* LDCV下采样浮点mesh表的buffer大小 */
}RKALGO_SEPARATE_MESH_INFO_S;

/*
    [Function]      : 计算相关参数：原始全分辨率浮点表 ---> 分步校正的下采样浮点表 所需的参数信息
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-07-06
    [input]         : srcW, srcH: 输入图像的宽高
                      dstW, dstH: 输出图像的宽高
    [output]        : pstMapSepaInfo: 输出相关参数
*/
EXPORT int32_t calcSeparateMeshInfo(
    const uint32_t srcW, const uint32_t srcH,
    const uint32_t dstW, const uint32_t dstH,
    RKALGO_SEPARATE_MESH_INFO_S *pstMapSepaInfo
);

/*
    [Function]      : 原始全分辨率浮点表 ---> 分步校正的下采样浮点表
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-07-05
    [input]         : pstMapSepaInfo: 拆分转换相关参数
                      pf32MapxOri, pf32MapyOri: 原始浮点全分辨率表
    [output]        : pf32MapxNew, pf32MapyNew: 分步校正的下采样浮点表
*/
EXPORT int32_t genSeparateMeshFromOriMap(
    const RKALGO_SEPARATE_MESH_INFO_S *pstMapSepaInfo,
    const float *pf32MapxOri, const float *pf32MapyOri,
    float *pf32MapxNew, float *pf32MapyNew
);

// clang-format on
#endif  // !__RKALGO_MAP_SEPARATE_H__
