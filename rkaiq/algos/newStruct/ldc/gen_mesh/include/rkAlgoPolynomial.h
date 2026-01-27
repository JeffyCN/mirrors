#ifndef __RKALGO_POLYNOMIAL_H__
#define __RKALGO_POLYNOMIAL_H__

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 复数结构体 */
typedef struct doubleComplex {
    double re;  // 实部
    double im;  // 虚部
} RkAlgoDoubleComplex;

/*
    [Function]      : 计算实系数三次方程0 = a0 + a1 * x + a2 * x^2 + a3 * x^3的实根
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2024-04-16
    [input]         : cubeCoefs: 三次方程的实系数，按照a0, a1, a2, a3顺序升幂排序
    [output]        : cubeRoots: 三次方程的实根
*/
int solveCubic(const double* cubeCoefs, double* cubeRoots);

/*
    [Function]      : 计算复系数N阶多项式方程0 = a0 + a1 * x + a2 * x^2 + a3 * x^3 + ... + an *
   x^n的复数根 基本原理：使用Durand-Kerner方法 [Author Name]   : Sunbeam Huang [Build Date]    :
   2024-04-16 [input]         : polyRank: 方程的最高阶数 maxIters: 最大迭代次数 cpxCoefs:
   方程的复系数，按照a0, a1, a2, a3, ... , an顺序升幂排序 [output]        : cpxRoots:
   方程的N个复数根
*/
void solveComplexPolyFunc(int polyRank, int maxIters, const RkAlgoDoubleComplex* cpxCoefs,
                          RkAlgoDoubleComplex* cpxRoots);

/*
    [Function]      : 拟合实系数N阶多项式曲线y = a0 + a1 * x + a2 * x^2 + a3 * x^3 + ... + an *
   x^n的系数 基本原理：At * A * C = At * Y，其中At为A的转置矩阵，C为系数矩阵 [Author Name]   :
   Sunbeam Huang [Build Date]    : 2024-04-16 [input]         : xData: 曲线的x采样数据 yData:
   曲线的y采样数据 dataNum: x-y采样数据的对数 polyRank: 曲线的最高阶数 [output]        : realCoefs:
   曲线的实系数，按照a0, a1, a2, a3, ... , an顺序升幂排序
*/
void fitRealPolyCurve(const double* xData, const double* yData, int dataNum, int polyRank,
                      double* realCoefs);

/*
    [Function]      : 计算N阶多项式曲线y = a0 + a1 * x + a2 * x^2 + a3 * x^3 + ... + an * x^n的值
    [Author Name]   : Sunbeam Huang
    [Build Date]    : 2024-04-16
    [input]         : realCoefs: 曲线的实系数，按照a0, a1, a2, a3, ... , an顺序升幂排序
                      xData: 单个x数据
                      polyRank: 曲线的最高阶数
    [output]        : yData: 单个y数据
*/
void evalRealPolyCurve(const double* realCoefs, int polyRank, double xData, double* yData);

#endif  // !__RKALGO_POLYNOMIAL_H__
