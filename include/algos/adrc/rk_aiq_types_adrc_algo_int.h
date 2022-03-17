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
#include "RkAiqCalibDbTypesV2.h"
#include "rk_aiq_types_adrc_stat_v200.h"

typedef struct DrcProcRes_s
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

} DrcProcRes_t;

typedef struct RkAiqAdrcProcResult_s
{
    DrcProcRes_t DrcProcRes;
    drcAttr_t drcAttr;
    CompressMode_t CompressMode;
    bool update;
    bool LongFrameMode;
    bool isHdrGlobalTmo;
    bool bTmoEn;
    bool isLinearTmo;
} RkAiqAdrcProcResult_t;


#endif
