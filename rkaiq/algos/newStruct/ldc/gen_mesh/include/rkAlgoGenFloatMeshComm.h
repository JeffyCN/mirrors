#ifndef __RKALGO_GEN_FLOAT_MESH_COMM_H__
#define __RKALGO_GEN_FLOAT_MESH_COMM_H__

#include "rkAlgoGenMeshComm.h"

/* 计算逆多项式(rho-tanTheta)的系数 */
void calcPolyTanCoeffInv(double a0, double a2, double a3, double a4, double radius,
                         int* invPolyTanNum, double* invPolyTanCoeff);

/* 计算逆多项式(rho-cotTheta)的系数 */
void calcPolyCotCoeffInv(double a0, double a2, double a3, double a4, double radius,
                         int* invPolyCotNum, double* invPolyCotCoeff);

/*
    [Function]      : 判断输入是否为2的幂次
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2024-04-10
    [input]         : n: 输入数字
    [output]        : RKALGO_BOOL_E: 输出结果
*/
RKALGO_BOOL_E isPowerOfTwo(uint32_t n);

/*
    [Function]      : 对3*3矩阵进行初等行变换求其逆
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2022-07-18
    [input]         : srcMatrix: 输入矩阵
    [output]        : matrixInv: 输出逆矩阵
*/
void calcMatrixInverse(const float srcMatrix[9], float matrixInv[9]);

/*
    [Function]      : 下采样浮点表双线性插值为全分辨率浮点表(输出宽高需要加1)
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2024-03-15
*/
void floatMeshUpSample(const float* pSmallMapx, /* [input]: mapx小表 */
                       const float* pSmallMapy, /* [input]: mapy小表 */
                       float* pBigMapx,         /* [output]: mapx大表 */
                       float* pBigMapy,         /* [output]: mapy大表 */
                       int srcWid,              /* [input]: 小表的宽 */
                       int srcHgt,              /* [input]: 小表的高 */
                       int dstWid,              /* [input]: 大表的宽，需要加1 */
                       int dstHgt,              /* [input]: 大表的高，需要加1 */
                       int meshStepW,           /* [input]: 小表的宽步长 */
                       int meshStepH            /* [input]: 小表的高步长 */
);

#endif  // !__RKALGO_GEN_FLOAT_MESH_COMM_H__
