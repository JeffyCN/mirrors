#ifndef __RKALGO_GEN_FLOAT_MESH_FROM_RK_H__
#define __RKALGO_GEN_FLOAT_MESH_FROM_RK_H__

#include "rkAlgoGenMeshComm.h"
#include "rkAlgoGenFloatMeshComm.h"

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/* 相机参数 */
typedef struct RKALGO_CAM_COEFF_INFO
{
    double cx, cy;                                          /* 镜头的光心 */
    double a0, a2, a3, a4;                                  /* 镜头的畸变系数 */
    double c, d, e;                                         /* 内参[c d;e 1] */
    double sf;                                              /* sf控制视角, sf越大视角越大 */

    int invPolyTanNum0;                                     /* level = 0时的rho-tanTheta多项式的系数个数 */
    double invPolyTanCoeff0[INV_POLY_COEFF_NUM];            /* level = 0时的rho-tanTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */

    int invPolyCotNum0;                                     /* level = 0时的rho-cotTheta多项式的系数个数 */
    double invPolyCotCoeff0[INV_POLY_COEFF_NUM];            /* level = 0时的rho-cotTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */

    int invPolyTanNum255;                                   /* level = 255时的rho-tanTheta多项式的系数个数 */
    double invPolyTanCoeff255[INV_POLY_COEFF_NUM];          /* level = 255时的rho-tanTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */

    int invPolyCotNum255;                                   /* level = 255时的rho-cotTheta多项式的系数个数 */
    double invPolyCotCoeff255[INV_POLY_COEFF_NUM];          /* level = 255时的rho-cotTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */
}RKALGO_CAM_COEFF_INFO_S;

typedef struct RKALGO_CAM_COEFF_ONE_LEVEL_INFO
{
    double cx, cy;                                          /* 镜头的光心 */
    double a0, a2, a3, a4;                                  /* 镜头的畸变系数 */
    double c, d, e;                                         /* 内参[c d;e 1] */
    double sf;                                              /* sf控制视角, sf越大视角越大 */
    uint32_t level;                                         /* 当前畸变校正等级 */
    int invPolyTanNum;                                      /* rho-tanTheta多项式的系数个数 */
    double invPolyTanCoeff[INV_POLY_COEFF_NUM];             /* level = 0时的rho-tanTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */
    int invPolyCotNum;                                      /* rho-cotTheta多项式的系数个数 */
    double invPolyCotCoeff[INV_POLY_COEFF_NUM];             /* level = 0时的rho-cotTheta多项式系数, 最高次数(INV_POLY_COEFF_NUM-1)次 */
}RKALGO_CAM_COEFF_ONE_LEVEL_INFO_S;

typedef struct RKALGO_GEN_FLOAT_MESH_INFO
{
    uint32_t srcW;                                          /* 输入图像宽 */
    uint32_t srcH;                                          /* 输入图像高 */
    uint32_t dstW;                                          /* 输出图像宽 */
    uint32_t dstH;                                          /* 输出图像高 */
    RKALGO_MESH_TYPE_E enMeshType;                          /* 硬件类型：FEC/LDCH/LDCV/LDCH+LDCV/debug */
    RKALGO_FEC_VERSION_E enFecVersion;                      /* 硬件FEC版本，例如rv1109, rv1126, rk3588, rv1126b */
    RKALGO_LDCH_VERSION_E enLdchVersion;                    /* 硬件LDCH版本(后级没有LDCV)，例如rv1109, rv1126, rk356x, rk3588, rv1106, rk3562 */
    RKALGO_LDCH_LDCV_VERSION_E enLdchLdcvVersion;           /* 硬件LDCH + LDCV版本，例如rk3576 */
    RKALGO_MESH_STEP_OPT_E enMeshStepOpt;                   /* 浮点映射表的采样步长选项 */

    uint32_t dstWidAlign;                                   /* 输出图像宽对齐 */
    uint32_t dstHgtAlign;                                   /* 输出图像高对齐 */
    uint32_t meshStepW;                                     /* 浮点映射表的宽步长 */
    uint32_t meshStepH;                                     /* 浮点映射表的高步长 */
    uint32_t meshW;                                         /* 浮点映射表的宽 */
    uint32_t meshH;                                         /* 浮点映射表的高 */
    uint64_t u64FloatMeshBufSize;                           /* 单个浮点映射表的buffer大小 */
    uint32_t maxLevel;                                      /* 硬件最大可以校正的畸变等级 */
    uint32_t correctX;                                      /* 水平x方向校正: 1代表校正, 0代表不校正 */
    uint32_t correctY;                                      /* 垂直y方向校正: 1代表校正, 0代表不校正 */
    uint32_t saveMaxFovX;                                   /* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */

    double cropStepW[2000], cropStepH[2000];
    double cropStartW[2000], cropStartH[2000];
}RKALGO_FLOAT_MESH_INFO_RK_S;


/* 外部接口：计算RK参数模型下的浮点映射表的宽高 */
int32_t calcFloatMeshSizeRK(RKALGO_FLOAT_MESH_INFO_RK_S *pStFloatMeshInfoRk);

/* 外部接口：生成下采样浮点表之前的相关参数初始化 */
int32_t genFloatMeshNLevelInit(
    RKALGO_CAM_COEFF_INFO_S *pCamCoeff,
    RKALGO_FLOAT_MESH_INFO_RK_S *genFloatMeshInfo
);

/* 外部接口：生成不同校正程度的下采样浮点表，采样步长可配 */
int32_t genFloatMeshNLevel(
    const RKALGO_CAM_COEFF_INFO_S *pCamCoeff,
    const RKALGO_FLOAT_MESH_INFO_RK_S *genFloatMeshInfo,
    uint32_t level,
    float *pf32MapxOri, float *pf32MapyOri
);

/* 外部接口：保存下采样浮点表 */
int32_t saveFloatMeshRK(
    const char *savePath,
    const RKALGO_FLOAT_MESH_INFO_RK_S *pStFloatMeshInfoRk,
    const float *pf32MeshX,
    const float *pf32MeshY,
    int32_t level
);

#endif // !__RKALGO_GEN_FLOAT_MESH_FROM_RK_H__
