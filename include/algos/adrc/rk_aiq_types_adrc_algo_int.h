/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
#ifndef __RK_AIQ_TYPES_ADRC_ALGO_INT_H__
#define __RK_AIQ_TYPES_ADRC_ALGO_INT_H__

#include "RkAiqCalibDbTypes.h"
#include "adrc_head.h"
#include "adrc_uapi_head.h"
#include "rk_aiq_types_adrc_stat_v200.h"

#define AIQ_ISP3X_DRC_Y_NUM 17


typedef struct DrcProcResV21_s
{
    int sw_drc_offset_pow2;
    int sw_drc_compres_scl;
    int sw_drc_position;
    int sw_drc_delta_scalein;
    int sw_drc_hpdetail_ratio;
    int sw_drc_lpdetail_ratio;
    int sw_drc_weicur_pix;
    int sw_drc_weipre_frame;
    int sw_drc_force_sgm_inv0;
    int sw_drc_motion_scl;
    int sw_drc_edge_scl;
    int sw_drc_space_sgm_inv1;
    int sw_drc_space_sgm_inv0;
    int sw_drc_range_sgm_inv1;
    int sw_drc_range_sgm_inv0;
    int sw_drc_weig_maxl;
    int sw_drc_weig_bilat;
    int sw_drc_gain_y[ISP21_DRC_Y_NUM];
    int sw_drc_compres_y[ISP21_DRC_Y_NUM];
    int sw_drc_scale_y[ISP21_DRC_Y_NUM];
    float sw_drc_adrc_gain;
    int sw_drc_iir_weight;
    int sw_drc_min_ogain;
} DrcProcResV21_t;

typedef struct DrcProcResV30_s {
    int bypass_en;
    int offset_pow2;
    int compres_scl;
    int position;
    int delta_scalein;
    int hpdetail_ratio;
    int lpdetail_ratio;
    int weicur_pix;
    int weipre_frame;
    int bilat_wt_off;
    int force_sgm_inv0;
    int motion_scl;
    int edge_scl;
    int space_sgm_inv1;
    int space_sgm_inv0;
    int range_sgm_inv1;
    int range_sgm_inv0;
    int weig_maxl;
    int weig_bilat;
    int enable_soft_thd;
    int bilat_soft_thd;
    int gain_y[AIQ_ISP3X_DRC_Y_NUM];
    int compres_y[AIQ_ISP3X_DRC_Y_NUM];
    int scale_y[AIQ_ISP3X_DRC_Y_NUM];
    float adrc_gain;
    int wr_cycle;
    int iir_weight;
    int min_ogain;
} DrcProcResV30_t;

typedef struct DrcProcRes_s {
    union {
        DrcProcResV21_t Drc_v21;
        DrcProcResV30_t Drc_v30;
    };
} DrcProcRes_t;

typedef struct RkAiqAdrcProcResult_s
{
    DrcProcRes_t DrcProcRes;
    CompressMode_t CompressMode;
    bool update;
    bool LongFrameMode;
    bool isHdrGlobalTmo;
    bool bTmoEn;
    bool isLinearTmo;
} RkAiqAdrcProcResult_t;


typedef enum AdrcVersion_e {
    ADRC_VERSION_356X = 0,
    ADRC_VERSION_3588 = 1,
    ADRC_VERSION_MAX
} AdrcVersion_t;

typedef enum drc_OpMode_s {
    DRC_OPMODE_API_OFF = 0, // run IQ ahdr
    DRC_OPMODE_MANU = 1,    // run api manual ahdr
    DRC_OPMODE_DRC_GAIN = 2,
    DRC_OPMODE_HILIT = 3,
    DRC_OPMODE_LOCAL_TMO = 4,
} drc_OpMode_t;

typedef struct mLocalDataV21_s
{
    float         LocalWeit;
    float         GlobalContrast;
    float         LoLitContrast;
} mLocalDataV21_t;

typedef struct mDrcGain_t {
    float DrcGain;
    float Alpha;
    float Clip;
} mDrcGain_t;

typedef struct mDrcHiLit_s {
    float Strength;
} mDrcHiLit_t;

typedef struct mDrcLocalV21_s {
    mLocalDataV21_t LocalData;
    float curPixWeit;
    float preFrameWeit;
    float Range_force_sgm;
    float Range_sgm_cur;
    float Range_sgm_pre;
    int Space_sgm_cur;
    int Space_sgm_pre;
} mDrcLocalV21_t;

typedef struct mDrcCompress_s {
    CompressMode_t Mode;
    uint16_t Manual_curve[ADRC_Y_NUM];
} mDrcCompress_t;

typedef struct mdrcAttr_V21_s {
    bool Enable;
    mDrcGain_t DrcGain;
    mDrcHiLit_t HiLit;
    mDrcLocalV21_t LocalSetting;
    mDrcCompress_t Compress;
    int Scale_y[ADRC_Y_NUM];
    float Edge_Weit;
    bool  OutPutLongFrame;
    int IIR_frame;
} mdrcAttr_V21_t;

typedef struct mLocalDataV30_s
{
    float         LocalWeit;
    int           LocalAutoEnable;
    float         LocalAutoWeit;
    float         GlobalContrast;
    float         LoLitContrast;
} mLocalDataV30_t;

typedef struct mDrcLocalV30_s {
    mLocalDataV30_t LocalData;
    float curPixWeit;
    float preFrameWeit;
    float Range_force_sgm;
    float Range_sgm_cur;
    float Range_sgm_pre;
    int Space_sgm_cur;
    int Space_sgm_pre;
} mDrcLocalV30_t;

typedef struct mdrcAttr_V30_s {
    bool Enable;
    mDrcGain_t DrcGain;
    mDrcHiLit_t HiLight;
    mDrcLocalV30_t LocalSetting;
    mDrcCompress_t CompressSetting;
    int Scale_y[ADRC_Y_NUM];
    float Edge_Weit;
    bool  OutPutLongFrame;
    int IIR_frame;
} mdrcAttr_V30_t;

typedef struct drcAttr_s {
    rk_aiq_uapi_sync_t sync;

    AdrcVersion_t Version;
    drc_OpMode_t opMode;
    mdrcAttr_V21_t stManualV21;
    mdrcAttr_V30_t stManualV30;
    mDrcGain_t stDrcGain;
    mDrcHiLit_t stHiLit;
    mLocalDataV21_t stLocalDataV21;
    mLocalDataV30_t stLocalDataV30;
    DrcInfo_t Info;
} drcAttr_t;


#endif
