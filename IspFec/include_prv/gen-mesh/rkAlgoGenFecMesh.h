#ifndef __RKALGO_GEN_FEC_MESH_H__
#define __RKALGO_GEN_FEC_MESH_H__

#include "rkAlgoGenMeshComm.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/*
* @struct   RKALGO_FEC_INFO_S
* @brief    define the RK FEC info
*/
typedef struct RKALGO_FEC_INFO
{
    /* 外部配置参数 */
    uint32_t srcW;                          /* 输入图像宽 */
    uint32_t srcH;                          /* 输入图像高 */
    uint32_t dstW;                          /* 输出图像宽 */
    uint32_t dstH;                          /* 输出图像高 */
    RKALGO_FEC_VERSION_E enFecVersion;      /* FEC的版本 */
    RKALGO_MESH_STEP_OPT_E enMeshStepOpt;   /* FEC定点化表的采样步长选项 */
    /* FEC定点化表的相关信息 */
    uint32_t dstWidAlign;                   /* 输出图像宽对齐 */
    uint32_t dstHgtAlign;                   /* 输出图像高对齐 */
    uint32_t meshStepW;                     /* FEC定点化表的宽步长 */
    uint32_t meshStepH;                     /* FEC定点化表的高步长 */
    uint32_t meshW;                         /* FEC定点化表的宽 */
    uint32_t meshH;                         /* FEC定点化表的高 */
    uint32_t spbNum, spbMeshPNumH, lastSpbH, lastSpbMeshPNumH;  /* for FEC in RV1109, RV1126, RK3588, etc. */
    uint64_t u64FecMeshBufSize;             /* FEC定点化表大小: (1)for RV1109, RV1126, RK3588: 表示单个buffer的大小; (2)for RV1126B: 表示整个buffer的大小  */
} RKALGO_FEC_INFO_S;

/*
* @struct   RKALGO_FEC_MESH_S
* @brief    define the RK FEC mesh buffer
*/
typedef struct RKALGO_FEC_MESH
{
    /* for FEC in RV1109, RV1126, RK3588, etc. */
    uint16_t *pu16MeshXI;
    uint8_t  *pu8MeshXF;
    uint16_t *pu16MeshYI;
    uint8_t  *pu8MeshYF;
    /* for FEC in RV1126B */
    uint8_t  *pu8FecMesh;
} RKALGO_FEC_MESH_S;

/* 计算FEC定点化表所需的buffer大小 */
int32_t calcFecMeshSize(RKALGO_FEC_INFO_S *pStFecInfo);

/* 下采样浮点表 ---> FEC定点化表 */
int32_t genFecMeshFromFloatMesh(
    const RKALGO_FEC_INFO_S *pStFecInfo,
    const float *pf32MeshX,
    const float *pf32MeshY,
    RKALGO_FEC_MESH_S *pStFecMeshBuf
);


/* 保存FEC定点化表 */
int32_t saveFecMesh(
    const char *savePath,
    const RKALGO_FEC_INFO_S *pStFecInfo,
    const RKALGO_FEC_MESH_S *pStFecMeshBuf,
    int32_t level
);


#if defined(__cplusplus) && (defined(WIN32) || defined(_WIN32))

#include <opencv2/opencv.hpp>

/* 全分辨率浮点表 ---> FEC定点化表 */
int32_t genFecMeshFromBigMap(
    const RKALGO_FEC_INFO_S *pStFecInfo,
    const cv::Mat &mapxBig,
    const cv::Mat &mapyBig,
    RKALGO_FEC_MESH_S *pStFecMeshBuf
);


#endif // defined(__cplusplus) && (defined(WIN32) || defined(_WIN32))



#endif // !__RKALGO_GEN_FEC_MESH_H__
