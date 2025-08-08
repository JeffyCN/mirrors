#ifndef __RKALGO_GEN_FLOAT_MESH_FROM_RK_H__
#define __RKALGO_GEN_FLOAT_MESH_FROM_RK_H__
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

#define INV_POLY_COEFF_NUM 21                               /* 多项式系数个数, 最高次数(INV_POLY_COEFF_NUM-1)次 */

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
    uint32_t mapStepW;                                      /* 浮点映射表的采样步长 */
    uint32_t mapStepH;
    uint32_t mapW;                                          /* 浮点映射表的宽 */
    uint32_t mapH;                                          /* 浮点映射表的高 */
    uint64_t u64MapBufSize;                                 /* 浮点映射表的buffer大小 */
    uint32_t maxLevel;                                      /* 硬件最大可以校正的畸变等级 */
    uint32_t correctX;										/* 水平x方向校正: 1代表校正, 0代表不校正 */
    uint32_t correctY;										/* 垂直y方向校正: 1代表校正, 0代表不校正 */
    uint32_t saveMaxFovX;									/* 保留水平x方向最大FOV: 1代表保留, 0代表不保留 */
    RKALGO_MESH_TYPE_E enMeshType;                          /* 最终需要的定点化表的类型：FEC/LDCH/LDCV */
    RKALGO_FEC_VERSION_E enFecVersion;                      /* 硬件FEC版本，例如rv1109, rv1126，rk3588 */
    RKALGO_LDCH_VERSION_E enLdchVersion;                    /* 硬件LDCH版本(后级没有LDCV)，例如rv1109，rv1126，rk356x，rk3588，1106，rk3562 */
    RKALGO_LDCH_LDCV_VERSION_E enLdchLdcvVersion;           /* 硬件LDCH + LDCV版本，例如rk3576 */
    double cropStepW[2000], cropStepH[2000];
    double cropStartW[2000], cropStartH[2000];
}RKALGO_GEN_FLOAT_MESH_INFO_S;


/* 外部接口：生成下采样浮点表之前的相关参数初始化 */
int32_t genFloatMeshNLevelInit(
    RKALGO_CAM_COEFF_INFO_S *camCoeff,
    RKALGO_GEN_FLOAT_MESH_INFO_S *genFloatMeshInfo
);

/* 外部接口：生成不同校正程度的下采样浮点表，采样步长可配 */
int32_t genFloatMeshNLevel(
    const RKALGO_CAM_COEFF_INFO_S *camCoeff,
    const RKALGO_GEN_FLOAT_MESH_INFO_S *genFloatMeshInfo,
    uint32_t level,
    float *pf32MapxOri, float *pf32MapyOri
);

// clang-format on
#endif  // !__RKALGO_GEN_FLOAT_MESH_FROM_RK_H__
