#ifndef __RKALGO_GEN_LDCH_LDCV_MESH_H__
#define __RKALGO_GEN_LDCH_LDCV_MESH_H__
// clang-format off

#include "rkAlgoGenMeshComm.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <string>
#include <algorithm>
// #include <opencv2/opencv.hpp>


/*
* @struct   RKALGO_LDCH_LDCV_INFO_S
* @brief    define the RK LDCH and LDCV info
*/
typedef struct RKALGO_LDCH_LDCV_INFO
{
    uint32_t srcW;                                      /* 输入图像宽 */
    uint32_t srcH;                                      /* 输入图像高 */
    uint32_t dstW;                                      /* 输出图像宽 */
    uint32_t dstH;                                      /* 输出图像高 */
    RKALGO_LDCH_LDCV_VERSION_E enLdchLdcvVersion;       /* LDCH和LDCV的版本 */
    /* LDCH定点化mesh相关信息 */
    uint32_t ldchDstWidAlign;                           /* LDCH要求原始全分辨率浮点map表的输出宽16对齐再 + 1 */
    uint32_t ldchDstHgtAlign;                           /* LDCH要求原始全分辨率浮点map表的输出高8对齐再 + 1 */
    uint32_t ldchMeshStepW;                             /* LDCH定点化mesh宽步长 */
    uint32_t ldchMeshStepH;                             /* LDCH定点化mesh高步长 */
    uint32_t ldchMeshW;                                 /* LDCH定点化mesh表的宽 */
    uint32_t ldchMeshH;                                 /* LDCH定点化mesh表的高 */
    uint32_t ldchAlignMeshW;                            /* mesh表的宽做2对齐 */
    uint32_t ldchFixBit;                                /* 小数定点化位数 */
    uint64_t u64LdchMeshBufSize;                        /* LDCH定点化mesh表所需的buffer大小 */
    /* LDCV定点化mesh相关信息 */
    uint32_t ldcvDstWidAlign;                           /* LDCV要求原始全分辨率浮点map表的输出宽16对齐再 + 1 */
    uint32_t ldcvDstHgtAlign;                           /* LDCV要求原始全分辨率浮点map表的输出宽16对齐再 + 1 */
    uint32_t ldcvMeshStepW;                             /* LDCV定点化mesh宽步长 */
    uint32_t ldcvMeshStepH;                             /* LDCV定点化mesh高步长 */
    uint32_t ldcvMeshW;                                 /* LDCV定点化mesh表的宽 */
    uint32_t ldcvMeshH;                                 /* LDCV定点化mesh表的高(包含第0行存放索引) */
    uint32_t ldcvFixBit;                                /* 小数定点化位数 */
    uint64_t u64LdcvMeshBufSize;                        /* LDCV定点化mesh表所需的buffer大小 */

}RKALGO_LDCH_LDCV_INFO_S;


/*
    [Function]      : 根据LDCH和LDCV的版本，计算LDCH和LDCV定点化mesh表所需的buffer大小
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-06-30
    [input]         : pstLdchLdcvInfo->enLdchLdcvVersion: 指定LDCH和LDCV版本: rk3576, etc.
                      pstLdchLdcvInfo->srcW, pstLdchLdcvInfo->srcH: 输入图像的宽高
                      pstLdchLdcvInfo->dstW, pstLdchLdcvInfo->dstH: 输出图像的宽高
    [output]        : pstLdchLdcvInfo: LDCH和LDCV定点化mesh表的相关信息
*/
int32_t calcLdchLdcvMeshSize(RKALGO_LDCH_LDCV_INFO_S *pstLdchLdcvInfo);

/*
    [Function]      : 分步校正的全分辨率浮点表 ---> LDCH和LDCV的定点化mesh表
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-06-30
    [input]         : pstLdchLdcvInfo: LDCH和LDCV定点化表的相关信息
                      bigMapWid, bigMapWid: 分步校正的全分辨率浮点表的宽高
                      newBigMapX, newBigMapY: 分步校正的全分辨率浮点表
    [output]        : pu16MeshX, pu16MeshY: LDCH和LDCV的定点化mesh表
*/
/*
 * int32_t genLdchLdcvMeshFromBigMap(
 *     const RKALGO_LDCH_LDCV_INFO_S *pstLdchLdcvInfo,
 *     const uint32_t bigMapWid, const uint32_t bigMapHgt,
 *     const cv::Mat &newBigMapX, const cv::Mat &newBigMapY,
 *     uint16_t *pu16MeshX, uint16_t *pu16MeshY
 * );
 */

/*
    [Function]      : LDCH/LDCV下采样浮点表 ---> LDCH/LDCV定点化表
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2023-07-06
    [input]         : pstLdchLdcvInfo: LDCH/LDCV定点化表的相关信息
                      ldchFloatMeshW, ldchFloatMeshH: LDCH下采样浮点表的宽高
                      ldcvFloatMeshW, ldcvFloatMeshH: LDCV下采样浮点表的宽高
                      pf32MapxNew, pf32MapyNew: 输入的下采样浮点表
    [input/output]  : pu16MeshX, pu16MeshY: 输出的LDCH/LDCV的定点化表, 根据buffer的指针是否为空, 有以下输出的组合，适配不同需求:
                                (1)NULL!=pf32MapxNew && NULL!=pu16MeshX: 则输出用于LDCH的定点化表;
                                (2)NULL!=pf32MapyNew && NULL!=pu16MeshY: 则输出用于LDCV的定点化表;
                                (3)以上(1)和(2)可以同时输出。
*/
int32_t genLdchLdcvMeshFromFloatMesh(
    const RKALGO_LDCH_LDCV_INFO_S *pstLdchLdcvInfo,
    const uint32_t ldchFloatMeshW, const uint32_t ldchFloatMeshH,
    const uint32_t ldcvFloatMeshW, const uint32_t ldcvFloatMeshH,
    const float *pf32MapxNew, const float *pf32MapyNew,
    uint16_t *pu16MeshX, uint16_t *pu16MeshY
);

// clang-format on
#endif  // !__RKALGO_GEN_LDCH_LDCV_MESH_H__
