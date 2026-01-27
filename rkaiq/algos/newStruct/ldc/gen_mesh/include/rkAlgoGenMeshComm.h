#ifndef __RKALGO_GEN_MESH_COMM_H__
#define __RKALGO_GEN_MESH_COMM_H__

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CLIP
#define CLIP(a, min, max) (((a) < (min)) ? (min) : (((a) > (max)) ? (max) : (a)))
#endif

#define MY_EPSILON        2.2204460492503131e-016
#define IS_DOUBLE_ZERO(d) (((d) < MY_EPSILON) && ((d) > -MY_EPSILON))

#define INV_POLY_COEFF_NUM \
    21 /* 多项式曲线拟合的系数个数, 则多项式曲线的最高阶数为(INV_POLY_COEFF_NUM - 1)次 */
#define RKALGO_PI        3.1415926535897932384626433832795 /* 圆周率 */
#define POLYFIT_RANK_MIN 1                                 /* 多项式曲线拟合的最低阶数 */
#define POLYFIT_RANK_MAX (INV_POLY_COEFF_NUM - 1)          /* 多项式曲线拟合的最高阶数 */
#define NUM_INVPOLYFIT   100                               /* 计算多项式系数的拟合点数量 */

/*
 * @enum    RKALGO_BOOL
 * @brief   define the bool enum
 */
typedef enum rkALGO_BOOL {
    RKALGO_FALSE = 0,
    RKALGO_TRUE  = 1,
} RKALGO_BOOL_E;

/*
 * @enum    RKALGO_MESH_TYPE_E
 * @brief   define the mesh type
 */
typedef enum rkALGO_MESH_TYPE {
    RKALGO_MESH_TYPE_DEBUG     = 0,
    RKALGO_MESH_TYPE_FEC       = 1, /* for FEC mesh */
    RKALGO_MESH_TYPE_LDCH      = 2, /* for LDCH mesh(no LDCV) */
    RKALGO_MESH_TYPE_LDCV      = 3, /* for LDCV mesh(no LDCH) */
    RKALGO_MESH_TYPE_LDCH_LDCV = 4, /* for LDCH + LDCV mesh */
    RKALGO_MESH_TYPE_BUTT,
} RKALGO_MESH_TYPE_E;

/*
 * @enum    RKALGO_FEC_VERSION_E
 * @brief   define the RK FEC version
 */
typedef enum rkALGO_FEC_VERSION {
    RKALGO_FEC_VERSION_NO_NEED   = -1,
    RKALGO_FEC_VERSION_1109_1126 = 0, /* for rv1109, rv1126 */
    RKALGO_FEC_VERSION_3588      = 1, /* for rk3588 */
    RKALGO_FEC_VERSION_BUTT,
} RKALGO_FEC_VERSION_E;

/*
 * @enum    RKALGO_LDCH_VERSION_E
 * @brief   define the RK LDCH version
 */
typedef enum RKALGO_LDCH_VERSION {
    RKALGO_LDCH_VERSION_NO_NEED = -1,
    RKALGO_LDCH_VERSION_0       = 0, /* old version: for rv1109，rv1126，rk356x，rk3588 */
    RKALGO_LDCH_VERSION_1       = 1, /* new version: for rv1106, rk3562, rk3576 */
    RKALGO_LDCH_VERSION_BUTT
} RKALGO_LDCH_VERSION_E;

/*
 * @enum     RKALGO_LDCH_LDCV_VERSION_E
 * @brief    define the RK LDCH and LDCV version
 */
typedef enum RKALGO_LDCH_LDCV_VERSION {
    RKALGO_LDCH_LDCV_VERSION_NO_NEED = -1,
    RKALGO_LDCH_LDCV_VERSION_3576    = 0, /* version: for rk3576 */
    RKALGO_LDCH_LDCV_VERSION_BUTT
} RKALGO_LDCH_LDCV_VERSION_E;

#endif  // !__RKALGO_GEN_MESH_COMM_H__
