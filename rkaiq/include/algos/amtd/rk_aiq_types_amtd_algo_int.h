#ifndef __RKAIQ_TYPES_AMTD_ALGO_INT_H__
#define __RKAIQ_TYPES_AMTD_ALGO_INT_H__
/**
 * @file rk_aiq_types_amtd_algo_int.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup AFD Auto flicker detection Module
 * @{
 *
 */

#include "algos/amtd/rk_aiq_types_amtd_algo.h"
#include "algos/ae/rk_aiq_types_ae_algo_int.h"

/*****************************************************************************/
/**
 * @brief   config params of amd
 */
/*****************************************************************************/
typedef struct AmtdConfig_s {

    int                           workMode;
    float                         gyroThred;
    float                         accSigma;

} AmtdConfig_t;

/*****************************************************************************/
/**
 *          AmtdInstanceConfig_t
 *
 * @brief   AMTD Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AmtdContext_s* AmtdHandle_t;    /**< handle to AMTD context */

/*****************************************************************************/
/**
 * @brief   ISP2.0 AMTD Algo Result Params
 */
/*****************************************************************************/
typedef struct AmtdPreResult_s {
    // TODO
} AmtdPreResult_t;

/*****************************************************************************/
/**
*     AmtdProcResult_t
* @brief   Amtd_Result.
*
*/
/*****************************************************************************/
typedef struct AmtdProcResult_s {
    bool                   isRun;
    int32_t                frmID;
    float                  avgTraj;
} AmtdProcResult_t;

typedef struct AmtdPostResult_s {

} AmtdPostResult_t;

/* @} AMTD */

#endif//__RKAIQ_TYPES_AMTD_ALGO_INT_H__