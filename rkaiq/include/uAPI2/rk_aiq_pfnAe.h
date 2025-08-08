/*
 *  Copyright (c) 2021 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _RK_AIQ_PFN_AE_H_
#define _RK_AIQ_PFN_AE_H_

#include "common/rk_aiq_types.h" /* common structs */

RKAIQ_BEGIN_DECLARE

#ifndef RK_AIQ_MAX_HDR_FRAME
#define RK_AIQ_MAX_HDR_FRAME 3
#endif

#define AE_BIT(a)          (1ULL << a)
#define AE_HwEnt0_Bit      0
#define AE_HwEnt1_Bit      1
#define AE_HwEnt2_Bit      2
#define AE_HwEnt3_Bit      3
#define AE_SwCoWkEnt03_Bit    8

//#define AE_EntValidChk(a, b)        (a & b == b)

/*****************************************************************************/
/**
 * @brief   ISP3.9 ae algo_thread processing input
 */
/*****************************************************************************/
// TODO: USE UNION
typedef struct ae_rkExp_s {
    RkAiqExpParamComb_t linear_exp;
    RkAiqExpParamComb_t hdr_exp[RK_AIQ_MAX_HDR_FRAME];
} ae_rkExp_t;

typedef struct ae_pfnAe_info_s
{
    // 0) common
    uint16_t entValidBit;

    // 1) single hw stats
    aeStats_entityStats_t *pHwEnt0;
    aeStats_entityStats_t *pHwEnt1;
    aeStats_entityStats_t *pHwEnt2;
    aeStats_entityStats_t *pHwEnt3;
    aeStats_entityStats_t *pSwCoWkEnt03;

    // 2) single cis exposure
    ae_rkExp_t cisRkExp;

    struct ae_pfnAe_info_s* next; // for surround view(multiple cams)
} ae_pfnAe_info_t;

/*****************************************************************************/
/**
 * @brief   ISP3.9 ae algo_thread processing output
 */
/*****************************************************************************/

typedef struct ae_win_s {
    uint16_t hw_aeCfg_win_x;
    uint16_t hw_aeCfg_win_y;
    uint16_t hw_aeCfg_win_width;
    uint16_t hw_aeCfg_win_height;
} ae_win_t;

typedef struct ae_hist_s {
    uint8_t hw_aeCfg_zone_wgt[AESTATS_ZONE_15x15_NUM];
} ae_hist_t;

typedef struct ae_entity_s {
    ae_win_t mainWin;
    ae_hist_t hist;
} ae_entity_t;

typedef struct ae_statsCfg_s {
    uint16_t entValidBit;
    ae_entity_t hwEnt0;
    ae_entity_t hwEnt1;
    ae_entity_t hwEnt2;
    ae_entity_t hwEnt3;

    ae_entity_t *pSwCoWkEnt03;
} ae_statsCfg_t;

typedef struct ae_i2cExp_s {
    unsigned int    nNumRegs;
    unsigned int*   pRegAddr;
    unsigned int*   pAddrByteNum;
    unsigned int*   pRegValue;
    unsigned int*   pValueByteNum;
    unsigned int*   pDelayFrames;
} ae_i2cExp_t;

typedef struct ae_pfnAe_single_results_s
{
    ae_rkExp_t cisRkExp;
    ae_i2cExp_t cisI2cExp;
    ae_statsCfg_t statsCfg;

    struct ae_pfnAe_single_results_s* next; // for surround view(multiple cams)
} ae_pfnAe_single_results_t;

typedef struct ae_pfnAe_results_s
{
    // 0) common
    bool isConverged;
    bool isLongFrmMode;
    bool isRkExpValid;
    uint32_t frmLengthLines; //vts

    // 1) single result
    ae_rkExp_t cisRkExp; // if isRkExpValid = false ,also need float real exposure value
    ae_i2cExp_t cisI2cExp;

    RkAiqIrisParamComb_t iris;
    ae_statsCfg_t statsCfg;

    struct ae_pfnAe_single_results_s* next; // for surround view(multiple cams)
} ae_pfnAe_results_t;

typedef struct rk_aiq_pfnAe_s
{
    /* ctx is the rk_aiq_sys_ctx_t/rk_aiq_camgroup_ctx_t
     * which is corresponded to
     * camera/camgroup, could be mapped to camera id.
     */
    int32_t (*pfn_ae_init)(void* ctx);
    int32_t (*pfn_ae_run)(void* ctx, const ae_pfnAe_info_t* pstAeInfo,
                          ae_pfnAe_results_t* pstAeResult);
    /* not used now */
    int32_t (*pfn_ae_ctrl)(void* ctx, uint32_t u32Cmd, void *pValue);
    int32_t (*pfn_ae_exit)(void* ctx);
} rk_aiq_pfnAe_t;

RKAIQ_END_DECLARE

#endif
