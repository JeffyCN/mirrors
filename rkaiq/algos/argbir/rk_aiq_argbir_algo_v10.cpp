/******************************************************************************
 *
 * Copyright 2024, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/
/**
 * @file rk_aiq_argbir_algo_v10.cpp
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include "math.h"
#include "rk_aiq_types_argbir_algo_prvt.h"
#include "xcam_log.h"

float RgbirGetInterpRatioV12(RgbirDataV10_t* pData, int& lo, int& hi, float CtrlValue,
                             int length_max) {
    float ratio = 0.0f;

    if (CtrlValue < pData[0].iso) {
        lo    = 0;
        hi    = 0;
        ratio = 0.0f;
    } else if (CtrlValue >= pData[length_max - 1].iso) {
        lo    = length_max - 1;
        hi    = length_max - 1;
        ratio = 0.0f;
    } else {
        for (int i = 0; i < length_max - 1; i++) {
            if (CtrlValue >= pData[i].iso && CtrlValue < pData[i + 1].iso) {
                lo    = i;
                hi    = i + 1;
                ratio = (CtrlValue - pData[i].iso) / (pData[i + 1].iso - pData[i].iso);
                break;
            } else
                continue;
        }
    }

    return ratio;
}

/******************************************************************************
 * RgbirSetDefaultValueV10()
 *set default Config data
 *****************************************************************************/
void RgbirSetDefaultValueV10(aRgbirContext_t* pArgbirCtx) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // initial checks
    DCT_ASSERT(pArgbirCtx != NULL);

    // config default Prev Ae data
    pArgbirCtx->CurrData.iso = ISOMIN;

    // config default CurrData data
    pArgbirCtx->CurrData.ApiMode = RGBIR_OPMODE_AUTO;
    pArgbirCtx->CurrData.HandleData.hw_rgbir_remosaic_en =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.hw_rgbir_remosaic_en;
    pArgbirCtx->CurrData.HandleData.RgbirData.hw_rgbir_remosaic_edge_coef =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0].hw_rgbir_remosaic_edge_coef;
    pArgbirCtx->CurrData.HandleData.RgbirData.hw_rgbir_remosaic_ir_blk_level =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0].hw_rgbir_remosaic_ir_blk_level;
    pArgbirCtx->CurrData.HandleData.RgbirData.sw_rgbir_remosaic_r_scale_coef =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0].sw_rgbir_remosaic_r_scale_coef;
    pArgbirCtx->CurrData.HandleData.RgbirData.sw_rgbir_remosaic_gr_scale_coef =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0]
            .sw_rgbir_remosaic_gr_scale_coef;
    pArgbirCtx->CurrData.HandleData.RgbirData.sw_rgbir_remosaic_gb_scale_coef =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0]
            .sw_rgbir_remosaic_gb_scale_coef;
    pArgbirCtx->CurrData.HandleData.RgbirData.sw_rgbir_remosaic_b_scale_coef =
        pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0].sw_rgbir_remosaic_b_scale_coef;
    for (int i = 0; i < ARGBIR_LUM2SCALE_TABLE_LEN; i++) {
        pArgbirCtx->CurrData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
            .hw_rgbir_remosaic_lum2scale_idx[i] =
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0]
                .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_idx[i];
        pArgbirCtx->CurrData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
            .hw_rgbir_remosaic_lum2scale_val[i] =
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[0]
                .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_val[i];
    }

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * RgbirEnableSetting()
 *
 *****************************************************************************/
bool RgbirEnableSetting(aRgbirContext_t* pArgbirCtx, RkAiqArgbirProcResult_t* pArgbirProcRes) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_AUTO)
        pArgbirCtx->NextData.HandleData.hw_rgbir_remosaic_en =
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.hw_rgbir_remosaic_en;
    else if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_MANUAL)
        pArgbirCtx->NextData.HandleData.hw_rgbir_remosaic_en =
            pArgbirCtx->RgbirAttrV10.stManual.hw_rgbir_remosaic_en;
    else {
        LOGE_ARGBIR("%s: RGBIR api in WRONG MODE!!!, drc by pass!!!\n", __FUNCTION__);
        pArgbirCtx->NextData.HandleData.hw_rgbir_remosaic_en = false;
    }
    pArgbirProcRes->bRgbirEn = pArgbirCtx->NextData.HandleData.hw_rgbir_remosaic_en;

    return pArgbirProcRes->bRgbirEn;
    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * ArgbirGetTuningProcResV10()
 *****************************************************************************/
void ArgbirGetTuningProcResV10(RkAiqArgbirProcResult_t* pArgbirProcRes,
                               mRgbirAttr_v10_t* pNextHandleData) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // hw_rgbir_remosaic_ir_blk_level
    pArgbirProcRes->RgbirProcRes.rgbir_v10.coe_theta =
        LIMIT_VALUE(pNextHandleData->RgbirData.hw_rgbir_remosaic_ir_blk_level, BIT_12_MAX, BIT_MIN);
    // hw_rgbir_remosaic_edge_coef
    pArgbirProcRes->RgbirProcRes.rgbir_v10.coe_delta = LIMIT_VALUE(
        pNextHandleData->RgbirData.hw_rgbir_remosaic_edge_coef, BIT_14_MAX, BIT_MIN + 1);
    // sw_rgbir_remosaic_r_scale_coef
    pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[0] =
        LIMIT_VALUE(SHIFT7BIT(pNextHandleData->RgbirData.sw_rgbir_remosaic_r_scale_coef),
                    BIT_8_MAX + 1, BIT_MIN + 1);
    // sw_rgbir_remosaic_gr_scale_coef
    pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[1] =
        LIMIT_VALUE(SHIFT7BIT(pNextHandleData->RgbirData.sw_rgbir_remosaic_gr_scale_coef),
                    BIT_8_MAX + 1, BIT_MIN + 1);
    // sw_rgbir_remosaic_gb_scale_coef
    pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[2] =
        LIMIT_VALUE(SHIFT7BIT(pNextHandleData->RgbirData.sw_rgbir_remosaic_gb_scale_coef),
                    BIT_8_MAX + 1, BIT_MIN + 1);
    // sw_rgbir_remosaic_b_scale_coef
    pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[3] =
        LIMIT_VALUE(SHIFT7BIT(pNextHandleData->RgbirData.sw_rgbir_remosaic_b_scale_coef),
                    BIT_8_MAX + 1, BIT_MIN + 1);
    // lum2scale table
    for (int i = 0; i < ARGBIR_LUM2SCALE_TABLE_LEN - 1; i++) {
        pArgbirProcRes->RgbirProcRes.rgbir_v10.luma_point[i] = LIMIT_VALUE_UNSIGNED(
            pNextHandleData->RgbirData.hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_idx[i],
            BIT_10_MAX);
    }
    pArgbirProcRes->RgbirProcRes.rgbir_v10.luma_point[ARGBIR_LUM2SCALE_TABLE_LEN - 1] =
        LIMIT_VALUE_UNSIGNED(pNextHandleData->RgbirData.hw_Rgbir_lum2scale_table
                                 .hw_rgbir_remosaic_lum2scale_idx[ARGBIR_LUM2SCALE_TABLE_LEN - 1],
                             BIT_10_MAX + 1);
    for (int i = 0; i < ARGBIR_LUM2SCALE_TABLE_LEN; i++) {
        pArgbirProcRes->RgbirProcRes.rgbir_v10.scale_map[i] = LIMIT_VALUE_UNSIGNED(
            pNextHandleData->RgbirData.hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_val[i],
            BIT_8_MAX + 1);
    }

    LOGD_ARGBIR(
        "%s: edge_aware_coef:%d ir_black_level:%d b_scale_coef:%d gb_scale_coef:%d "
        "gr_scale_coef:%d r_scale_coef:%d\n",
        __FUNCTION__, pArgbirProcRes->RgbirProcRes.rgbir_v10.coe_delta,
        pArgbirProcRes->RgbirProcRes.rgbir_v10.coe_theta,
        pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[3],
        pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[2],
        pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[1],
        pArgbirProcRes->RgbirProcRes.rgbir_v10.scale[0]);

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * ArgbirTuningParaProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
void ArgbirTuningParaProcessing(aRgbirContext_t* pArgbirCtx,
                                RkAiqArgbirProcResult_t* pArgbirProcRes) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // para setting
    if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_AUTO) {
        float CtrlValue = pArgbirCtx->NextData.iso;

        int lo = 0, hi = 0;
        float ratio =
            RgbirGetInterpRatioV12(pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData, lo,
                                   hi, CtrlValue, ARGBIR_ISO_STEP_MAX);
        // hw_rgbir_remosaic_edge_coef
        pArgbirCtx->NextData.HandleData.RgbirData.hw_rgbir_remosaic_edge_coef =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .hw_rgbir_remosaic_edge_coef -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .hw_rgbir_remosaic_edge_coef) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .hw_rgbir_remosaic_edge_coef;
        // hw_rgbir_remosaic_ir_blk_level
        pArgbirCtx->NextData.HandleData.RgbirData.hw_rgbir_remosaic_ir_blk_level =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .hw_rgbir_remosaic_ir_blk_level -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .hw_rgbir_remosaic_ir_blk_level) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .hw_rgbir_remosaic_ir_blk_level;
        // sw_rgbir_remosaic_b_scale_coef
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_b_scale_coef =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .sw_rgbir_remosaic_b_scale_coef -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .sw_rgbir_remosaic_b_scale_coef) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .sw_rgbir_remosaic_b_scale_coef;
        // sw_rgbir_remosaic_gb_scale_coef
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_gb_scale_coef =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .sw_rgbir_remosaic_gb_scale_coef -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .sw_rgbir_remosaic_gb_scale_coef) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .sw_rgbir_remosaic_gb_scale_coef;
        // sw_rgbir_remosaic_gr_scale_coef
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_gr_scale_coef =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .sw_rgbir_remosaic_gr_scale_coef -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .sw_rgbir_remosaic_gr_scale_coef) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .sw_rgbir_remosaic_gr_scale_coef;
        // sw_rgbir_remosaic_r_scale_coef
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_r_scale_coef =
            ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                         .sw_rgbir_remosaic_r_scale_coef -
                     pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                         .sw_rgbir_remosaic_r_scale_coef) +
            pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                .sw_rgbir_remosaic_r_scale_coef;

        for (int i = 0; i < ARGBIR_LUM2SCALE_TABLE_LEN; i++) {
            // hw_rgbir_remosaic_lum2scale_idx
            pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                .hw_rgbir_remosaic_lum2scale_idx[i] =
                ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                             .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_idx[i] -
                         pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                             .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_idx[i]) +
                pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                    .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_idx[i];
            // hw_rgbir_remosaic_lum2scale_val
            pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                .hw_rgbir_remosaic_lum2scale_val[i] =
                ratio * (pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[hi]
                             .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_val[i] -
                         pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                             .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_val[i]) +
                pArgbirCtx->RgbirAttrV10.stAuto.RgbirTuningPara.RgbirData[lo]
                    .hw_Rgbir_lum2scale_table.hw_rgbir_remosaic_lum2scale_val[i];
        }
    } else if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_MANUAL) {
        memcpy(&pArgbirCtx->NextData.HandleData.RgbirData,
               &pArgbirCtx->RgbirAttrV10.stManual.RgbirData, sizeof(mRgbirDataV10_t));
    }

    LOGI_ARGBIR(
        "%s: Enable:%d edge_aware_coef:%d ir_black_level:%f b_scale_coef:%f gb_scale_coef:%f "
        "gr_scale_coef:%f "
        "r_scale_coef:%f\n",
        __FUNCTION__, pArgbirCtx->NextData.HandleData.hw_rgbir_remosaic_en,
        pArgbirCtx->NextData.HandleData.RgbirData.hw_rgbir_remosaic_edge_coef,
        pArgbirCtx->NextData.HandleData.RgbirData.hw_rgbir_remosaic_ir_blk_level,
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_b_scale_coef,
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_gb_scale_coef,
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_gr_scale_coef,
        pArgbirCtx->NextData.HandleData.RgbirData.sw_rgbir_remosaic_r_scale_coef);
    LOGI_ARGBIR("%s: lum2scale_idx: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__,
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[0],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[1],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[2],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[3],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[4],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[5],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[6],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[7],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[8],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[9],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[10],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[11],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[12],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[13],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[14],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[15],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_idx[16]);
    LOGI_ARGBIR("%s: lum2scale_val: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                __FUNCTION__,
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[0],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[1],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[2],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[3],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[4],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[5],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[6],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[7],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[8],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[9],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[10],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[11],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[12],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[13],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[14],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[15],
                pArgbirCtx->NextData.HandleData.RgbirData.hw_Rgbir_lum2scale_table
                    .hw_rgbir_remosaic_lum2scale_val[16]);

    // get tuning proc res
    ArgbirGetTuningProcResV10(pArgbirProcRes, &pArgbirCtx->NextData.HandleData);

    // store current handle data to pre data for next loop
    pArgbirCtx->CurrData = pArgbirCtx->NextData;

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
}

/******************************************************************************
 * ArgbirByPassTuningProcessing()
 *get handle para by config and current variate
 *****************************************************************************/
bool ArgbirByPassTuningProcessing(aRgbirContext_t* pArgbirCtx) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    bool bypass = false;
    float diff  = 0.0f;

    if (pArgbirCtx->FrameID <= INIT_CALC_PARAMS_NUM)
        bypass = false;
    else if (pArgbirCtx->RgbirAttrV10.opMode != pArgbirCtx->CurrData.ApiMode)
        bypass = false;
    else if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_MANUAL)
        bypass = !pArgbirCtx->ifReCalcStManual;
    else if (pArgbirCtx->RgbirAttrV10.opMode == RGBIR_OPMODE_AUTO) {
        diff = pArgbirCtx->CurrData.iso - pArgbirCtx->NextData.iso;
        diff /= pArgbirCtx->CurrData.iso;
        if (diff >= RGBIR_BYPASS_THREAD || diff <= (0.0f - RGBIR_BYPASS_THREAD))
            bypass = false;
        else
            bypass = true;
        bypass = bypass && !pArgbirCtx->ifReCalcStAuto;
    }

    LOGI_ARGBIR(
        "%s: FrameID:%d DRCApiMode:%d ifReCalcStAuto:%d "
        "ifReCalcStManual:%d ISO:%f bypass:%d\n",
        __FUNCTION__, pArgbirCtx->FrameID, pArgbirCtx->RgbirAttrV10.opMode,
        pArgbirCtx->ifReCalcStAuto, pArgbirCtx->ifReCalcStManual, pArgbirCtx->NextData.iso, bypass);

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
    return bypass;
}

/******************************************************************************
 * aRgbirStart()
 *****************************************************************************/
XCamReturn aRgbirStart(aRgbirContext_t* pArgbirCtx) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pArgbirCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    if ((ARGBIR_STATE_RUNNING == pArgbirCtx->state) || (ARGBIR_STATE_LOCKED == pArgbirCtx->state)) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pArgbirCtx->state = ARGBIR_STATE_RUNNING;

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);
    return (XCAM_RETURN_NO_ERROR);
}

/******************************************************************************
 * aRgbirStop()
 *****************************************************************************/
XCamReturn aRgbirStop(aRgbirContext_t* pArgbirCtx) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);

    // initial checks
    if (pArgbirCtx == NULL) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    // before stopping, unlock the AHDR if locked
    if (ARGBIR_STATE_LOCKED == pArgbirCtx->state) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    pArgbirCtx->state = ARGBIR_STATE_STOPPED;

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}

/******************************************************************************
 * aRgbirInit()
 *****************************************************************************/
XCamReturn aRgbirInit(aRgbirContext_t** ppArgbirCtx, CamCalibDbV2Context_t* pCalibDb) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    aRgbirContext_t* pArgbirCtx;
    pArgbirCtx = (aRgbirContext_t*)malloc(sizeof(aRgbirContext_t));
    if (pArgbirCtx == NULL) {
        LOGE_ARGBIR("%s(%d): invalid inputparams\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_MEM;
    }

    memset(pArgbirCtx, 0x00, sizeof(aRgbirContext_t));
    *ppArgbirCtx      = pArgbirCtx;
    pArgbirCtx->state = ARGBIR_STATE_INITIALIZED;

    CalibDbV2_rgbir_v10_t* calibv2_argbir_calib =
        (CalibDbV2_rgbir_v10_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDb, argbir_calib));

    pArgbirCtx->RgbirAttrV10.opMode = RGBIR_OPMODE_AUTO;

    memcpy(&pArgbirCtx->RgbirAttrV10.stAuto, calibv2_argbir_calib,
           sizeof(CalibDbV2_rgbir_v10_t));  // load iq paras
    RgbirSetDefaultValueV10(pArgbirCtx);    // set default para

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
/******************************************************************************
 * aRgbirRelease()
 *****************************************************************************/
XCamReturn aRgbirRelease(aRgbirContext_t* pArgbirCtx) {
    LOG1_ARGBIR("%s:enter!\n", __FUNCTION__);
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    // initial checks
    if (NULL == pArgbirCtx) {
        return (XCAM_RETURN_ERROR_PARAM);
    }

    result = aRgbirStop(pArgbirCtx);
    if (result != XCAM_RETURN_NO_ERROR) {
        LOGE_ARGBIR("%s: aRgbirRelease() failed!\n", __FUNCTION__);
        return (result);
    }

    // check state
    if ((ARGBIR_STATE_RUNNING == pArgbirCtx->state) || (ARGBIR_STATE_LOCKED == pArgbirCtx->state)) {
        return (XCAM_RETURN_ERROR_MEM);
    }

    free(pArgbirCtx);
    pArgbirCtx = NULL;

    LOG1_ARGBIR("%s:exit!\n", __FUNCTION__);

    return (XCAM_RETURN_NO_ERROR);
}
