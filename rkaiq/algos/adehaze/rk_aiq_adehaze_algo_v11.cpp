/*
 * rk_aiq_adehaze_algo_v11.cpp
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
#include "rk_aiq_adehaze_algo_v11.h"
#include <string.h>
#include "xcam_log.h"
#include "rkisp21-config.h"

float GetInterpRatioV11(float* pX, int& lo, int& hi, float CtrlValue, int length_max) {
    float ratio = 0.0f;

    if (CtrlValue < pX[0]) {
        lo    = 0;
        hi    = 0;
        ratio = 0.0f;
    } else if (CtrlValue >= pX[length_max - 1]) {
        lo    = length_max - 1;
        hi    = length_max - 1;
        ratio = 0.0f;
    } else {
        for (int i = 0; i < length_max - 1; i++) {
            if (CtrlValue >= pX[i] && CtrlValue < pX[i + 1]) {
                lo    = i;
                hi    = i + 1;
                ratio = (CtrlValue - pX[i]) / (pX[i + 1] - pX[i]);
                break;
            } else
                continue;
        }
    }

    return ratio;
}

int ClipValueV11(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

void stManuGetDehazeParamsV11(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                              int rawWidth, int rawHeight, unsigned int MDehazeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11.air_lc_en =
        pStManu->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV11.dc_min_th =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.dc_min_th, 8, 0);
    pProcRes->ProcResV11.dc_max_th =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.dc_max_th, 8, 0);
    pProcRes->ProcResV11.yhist_th = ClipValueV11(pStManu->dehaze_setting.DehazeData.yhist_th, 8, 0);
    pProcRes->ProcResV11.yblk_th  = int(pStManu->dehaze_setting.DehazeData.yblk_th *
                                       ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16));
    pProcRes->ProcResV11.dark_th  = ClipValueV11(pStManu->dehaze_setting.DehazeData.dark_th, 8, 0);
    pProcRes->ProcResV11.bright_min =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.bright_min, 8, 0);
    pProcRes->ProcResV11.bright_max =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.bright_max, 8, 0);
    pProcRes->ProcResV11.wt_max  = ClipValueV11(pStManu->dehaze_setting.DehazeData.wt_max, 0, 8);
    pProcRes->ProcResV11.air_min = ClipValueV11(pStManu->dehaze_setting.DehazeData.air_min, 8, 0);
    pProcRes->ProcResV11.air_max = ClipValueV11(pStManu->dehaze_setting.DehazeData.air_max, 8, 0);
    pProcRes->ProcResV11.tmax_base =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.tmax_base, 8, 0);
    pProcRes->ProcResV11.tmax_off =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.tmax_off, 0, 10);
    pProcRes->ProcResV11.tmax_max =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.tmax_max, 0, 10);
    pProcRes->ProcResV11.cfg_wt  = ClipValueV11(pStManu->dehaze_setting.DehazeData.cfg_wt, 0, 8);
    pProcRes->ProcResV11.cfg_air = ClipValueV11(pStManu->dehaze_setting.DehazeData.cfg_air, 8, 0);
    pProcRes->ProcResV11.cfg_tmax =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.cfg_tmax, 0, 10);
    pProcRes->ProcResV11.range_sima =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.range_sigma, 0, 8);
    pProcRes->ProcResV11.space_sigma_cur =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.space_sigma_cur, 0, 8);
    pProcRes->ProcResV11.space_sigma_pre =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.space_sigma_pre, 0, 8);
    pProcRes->ProcResV11.bf_weight =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.bf_weight, 0, 8);
    pProcRes->ProcResV11.dc_weitcur =
        ClipValueV11(pStManu->dehaze_setting.DehazeData.dc_weitcur, 0, 8);
    pProcRes->ProcResV11.stab_fnum      = ClipValueV11(pStManu->dehaze_setting.stab_fnum, 5, 0);
    if (pStManu->dehaze_setting.sigma)
        pProcRes->ProcResV11.iir_sigma =
            LIMIT_VALUE(int(256.0f / pStManu->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV11.iir_sigma = 0x1;
    if (pStManu->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV11.iir_wt_sigma =
            LIMIT_VALUE(int(1024.0f / (8.0f * pStManu->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV11.iir_wt_sigma = 0x7ff;
    if (pStManu->dehaze_setting.air_sigma)
        pProcRes->ProcResV11.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pStManu->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV11.iir_air_sigma = 0x8;
    if (pStManu->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV11.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pStManu->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV11.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV11.iir_pre_wet =
        LIMIT_VALUE(int(pStManu->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV11.gaus_h0        = DEHAZE_GAUS_H4;
    pProcRes->ProcResV11.gaus_h1        = DEHAZE_GAUS_H1;
    pProcRes->ProcResV11.gaus_h2        = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV11.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV11.cfg_wt -= level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11.cfg_air -= level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11.cfg_tmax -= level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        }
        pProcRes->ProcResV11.cfg_wt = LIMIT_VALUE(pProcRes->ProcResV11.cfg_wt, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11.cfg_air =
            LIMIT_VALUE(pProcRes->ProcResV11.cfg_air, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11.cfg_tmax =
            LIMIT_VALUE(pProcRes->ProcResV11.cfg_tmax, BIT_10_MAX, BIT_MIN);
    }

    if (pProcRes->ProcResV11.dc_en && !(pProcRes->ProcResV11.enhance_en)) {
        if (pProcRes->ProcResV11.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 MDehazeStrth:%d cfg_air:%f cfg_tmax:%f cfg_wt:%f\n",
                         __func__, MDehazeStrth, pProcRes->ProcResV11.cfg_air / 1.0f,
                         pProcRes->ProcResV11.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV11.cfg_wt / 255.0f);
            LOGV_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV11.cfg_air, pProcRes->ProcResV11.cfg_tmax,
                         pProcRes->ProcResV11.cfg_wt);
        } else if (pProcRes->ProcResV11.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__,
                         pProcRes->ProcResV11.air_max / 1.0f, pProcRes->ProcResV11.air_min / 1.0f,
                         pProcRes->ProcResV11.tmax_base / 1.0f,
                         pProcRes->ProcResV11.wt_max / 255.0f);
            LOGV_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV11.air_max, pProcRes->ProcResV11.air_min,
                pProcRes->ProcResV11.tmax_base, pProcRes->ProcResV11.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV11(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                               unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;

    pProcRes->ProcResV11.enhance_value =
        ClipValueV11(pStManu->enhance_setting.EnhanceData.enhance_value, 4, 10);
    pProcRes->ProcResV11.enhance_chroma =
        ClipValueV11(pStManu->enhance_setting.EnhanceData.enhance_chroma, 4, 10);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11.enhance_value -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11.enhance_value =
            LIMIT_VALUE(pProcRes->ProcResV11.enhance_value, BIT_14_MAX, BIT_MIN);
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11.enhance_chroma -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11.enhance_chroma =
            LIMIT_VALUE(pProcRes->ProcResV11.enhance_chroma, BIT_14_MAX, BIT_MIN);
    }

    for (int i = 0; i < DHAZ_V11_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV11.enh_curve[i] = (int)(pStManu->enhance_setting.enhance_curve[i]);

    if (pProcRes->ProcResV11.dc_en && pProcRes->ProcResV11.enhance_en) {
        LOGD_ADEHAZE(
            "%s MEnhanceStrth:%d MEnhanceChromeStrth:%d enhance_value:%f enhance_chroma:%f\n",
            __func__, MEnhanceStrth, MEnhanceChromeStrth,
            pStManu->enhance_setting.EnhanceData.enhance_value / 1024.0f,
            pStManu->enhance_setting.EnhanceData.enhance_chroma / 1024.0f);
        LOGV_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV11.enhance_value, pProcRes->ProcResV11.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV11(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11.hpara_en =
        pStManu->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV11.hpara_en =
        pProcRes->ProcResV11.dc_en ? pProcRes->ProcResV11.hpara_en : FUNCTION_ENABLE;

    pProcRes->ProcResV11.hist_gratio =
        ClipValueV11(pStManu->hist_setting.HistData.hist_gratio, 5, 3);
    pProcRes->ProcResV11.hist_th_off =
        ClipValueV11(pStManu->hist_setting.HistData.hist_th_off, 8, 0);
    pProcRes->ProcResV11.hist_k     = ClipValueV11(pStManu->hist_setting.HistData.hist_k, 3, 2);
    pProcRes->ProcResV11.hist_min   = ClipValueV11(pStManu->hist_setting.HistData.hist_min, 1, 8);
    pProcRes->ProcResV11.cfg_gratio = ClipValueV11(pStManu->hist_setting.HistData.cfg_gratio, 5, 8);
    pProcRes->ProcResV11.hist_scale = ClipValueV11(pStManu->hist_setting.HistData.hist_scale, 5, 8);

    if (pProcRes->ProcResV11.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV11.cfg_alpha / 255.0f, pProcRes->ProcResV11.hpara_en,
            pProcRes->ProcResV11.hist_gratio / 255.0f, pProcRes->ProcResV11.hist_th_off / 1.0f,
            pProcRes->ProcResV11.hist_k / 4.0f, pProcRes->ProcResV11.hist_min / 256.0f,
            pProcRes->ProcResV11.hist_scale / 256.0f, pProcRes->ProcResV11.cfg_gratio / 256.0f);
        LOGV_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV11.cfg_alpha, pProcRes->ProcResV11.hist_gratio,
            pProcRes->ProcResV11.hist_th_off, pProcRes->ProcResV11.hist_k,
            pProcRes->ProcResV11.hist_min, pProcRes->ProcResV11.hist_scale,
            pProcRes->ProcResV11.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV11(CalibDbDehazeV11_t* pCalibV11, RkAiqAdehazeProcResult_t* pProcRes,
                        int rawWidth, int rawHeight, unsigned int MDehazeStrth, float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    int lo = 0, hi = 0;
    float ratio = GetInterpRatioV11(pCalibV11->dehaze_setting.DehazeData.CtrlData, lo, hi,
                                    CtrlValue, DHAZ_CTRL_DATA_STEP_MAX);

    pProcRes->ProcResV11.dc_min_th =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.dc_min_th[hi] -
                              pCalibV11->dehaze_setting.DehazeData.dc_min_th[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.dc_min_th[lo],
                     8, 0);
    pProcRes->ProcResV11.dc_max_th =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.dc_max_th[hi] -
                              pCalibV11->dehaze_setting.DehazeData.dc_max_th[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.dc_max_th[lo],
                     8, 0);
    pProcRes->ProcResV11.yhist_th =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.yhist_th[hi] -
                              pCalibV11->dehaze_setting.DehazeData.yhist_th[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.yhist_th[lo],
                     8, 0);
    pProcRes->ProcResV11.yblk_th = (ratio * (pCalibV11->dehaze_setting.DehazeData.yblk_th[hi] -
                                             pCalibV11->dehaze_setting.DehazeData.yblk_th[lo]) +
                                    pCalibV11->dehaze_setting.DehazeData.yblk_th[lo]) *
                                   ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16);
    pProcRes->ProcResV11.dark_th =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.dark_th[hi] -
                              pCalibV11->dehaze_setting.DehazeData.dark_th[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.dark_th[lo],
                     8, 0);

    pProcRes->ProcResV11.bright_min =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.bright_min[hi] -
                              pCalibV11->dehaze_setting.DehazeData.bright_min[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.bright_min[lo],
                     8, 0);
    pProcRes->ProcResV11.bright_max =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.bright_max[hi] -
                              pCalibV11->dehaze_setting.DehazeData.bright_max[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.bright_max[lo],
                     8, 0);
    pProcRes->ProcResV11.wt_max =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.wt_max[hi] -
                              pCalibV11->dehaze_setting.DehazeData.wt_max[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.wt_max[lo],
                     0, 8);
    pProcRes->ProcResV11.air_min =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.air_min[hi] -
                              pCalibV11->dehaze_setting.DehazeData.air_min[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.air_min[lo],
                     8, 0);
    pProcRes->ProcResV11.air_max =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.air_max[hi] -
                              pCalibV11->dehaze_setting.DehazeData.air_max[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.air_max[lo],
                     8, 0);
    pProcRes->ProcResV11.tmax_base =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.tmax_base[hi] -
                              pCalibV11->dehaze_setting.DehazeData.tmax_base[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.tmax_base[lo],
                     8, 0);
    pProcRes->ProcResV11.tmax_off =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.tmax_off[hi] -
                              pCalibV11->dehaze_setting.DehazeData.tmax_off[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.tmax_off[lo],
                     0, 10);
    pProcRes->ProcResV11.tmax_max =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.tmax_max[hi] -
                              pCalibV11->dehaze_setting.DehazeData.tmax_max[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.tmax_max[lo],
                     0, 10);
    pProcRes->ProcResV11.cfg_wt =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.cfg_wt[hi] -
                              pCalibV11->dehaze_setting.DehazeData.cfg_wt[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.cfg_wt[lo],
                     0, 8);
    pProcRes->ProcResV11.cfg_air =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.cfg_air[hi] -
                              pCalibV11->dehaze_setting.DehazeData.cfg_air[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.cfg_air[lo],
                     8, 0);
    pProcRes->ProcResV11.cfg_tmax =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.cfg_tmax[hi] -
                              pCalibV11->dehaze_setting.DehazeData.cfg_tmax[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.cfg_tmax[lo],
                     0, 10);
    pProcRes->ProcResV11.range_sima =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.range_sigma[hi] -
                              pCalibV11->dehaze_setting.DehazeData.range_sigma[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.range_sigma[lo],
                     0, 8);
    pProcRes->ProcResV11.space_sigma_cur =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.space_sigma_cur[hi] -
                              pCalibV11->dehaze_setting.DehazeData.space_sigma_cur[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.space_sigma_cur[lo],
                     0, 8);
    pProcRes->ProcResV11.space_sigma_pre =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.space_sigma_pre[hi] -
                              pCalibV11->dehaze_setting.DehazeData.space_sigma_pre[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.space_sigma_pre[lo],
                     0, 8);
    pProcRes->ProcResV11.bf_weight =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.bf_weight[hi] -
                              pCalibV11->dehaze_setting.DehazeData.bf_weight[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.bf_weight[lo],
                     0, 8);
    pProcRes->ProcResV11.dc_weitcur =
        ClipValueV11(ratio * (pCalibV11->dehaze_setting.DehazeData.dc_weitcur[hi] -
                              pCalibV11->dehaze_setting.DehazeData.dc_weitcur[lo]) +
                         pCalibV11->dehaze_setting.DehazeData.dc_weitcur[lo],
                     0, 8);
    pProcRes->ProcResV11.air_lc_en =
        pCalibV11->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV11.stab_fnum      = ClipValueV11(pCalibV11->dehaze_setting.stab_fnum, 5, 0);
    if (pCalibV11->dehaze_setting.sigma)
        pProcRes->ProcResV11.iir_sigma =
            LIMIT_VALUE(int(256.0f / pCalibV11->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV11.iir_sigma = 0x1;
    if (pCalibV11->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV11.iir_wt_sigma = LIMIT_VALUE(
            int(1024.0f / (8.0f * pCalibV11->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV11.iir_wt_sigma = 0x7ff;
    if (pCalibV11->dehaze_setting.air_sigma)
        pProcRes->ProcResV11.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pCalibV11->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV11.iir_air_sigma = 0x8;
    if (pCalibV11->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV11.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pCalibV11->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV11.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV11.iir_pre_wet =
        LIMIT_VALUE(int(pCalibV11->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV11.gaus_h0        = DEHAZE_GAUS_H4;
    pProcRes->ProcResV11.gaus_h1        = DEHAZE_GAUS_H1;
    pProcRes->ProcResV11.gaus_h2        = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV11.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff        = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV11.cfg_wt -= level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11.cfg_air -= level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11.cfg_tmax -= level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        }
        pProcRes->ProcResV11.cfg_wt = LIMIT_VALUE(pProcRes->ProcResV11.cfg_wt, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11.cfg_air =
            LIMIT_VALUE(pProcRes->ProcResV11.cfg_air, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11.cfg_tmax =
            LIMIT_VALUE(pProcRes->ProcResV11.cfg_tmax, BIT_10_MAX, BIT_MIN);
    }

    if (pProcRes->ProcResV11.dc_en && !(pProcRes->ProcResV11.enhance_en)) {
        if (pProcRes->ProcResV11.cfg_alpha == 255) {
            LOGD_ADEHAZE(
                "%s cfg_alpha:1 CtrlValue:%f MDehazeStrth:%d cfg_air:%f cfg_tmax:%f cfg_wt:%f\n",
                __func__, CtrlValue, MDehazeStrth, pProcRes->ProcResV11.cfg_air / 1.0f,
                pProcRes->ProcResV11.cfg_tmax / 1023.0f, pProcRes->ProcResV11.cfg_wt / 255.0f);
            LOGV_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV11.cfg_air, pProcRes->ProcResV11.cfg_tmax,
                         pProcRes->ProcResV11.cfg_wt);
        } else if (pProcRes->ProcResV11.cfg_alpha == 0) {
            LOGD_ADEHAZE(
                "%s cfg_alpha:0 CtrlValue:%f air_max:%f air_min:%f tmax_base:%f wt_max:%f\n",
                __func__, CtrlValue, pProcRes->ProcResV11.air_max / 1.0f,
                pProcRes->ProcResV11.air_min / 1.0f, pProcRes->ProcResV11.tmax_base / 1.0f,
                pProcRes->ProcResV11.wt_max / 255.0f);
            LOGV_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV11.air_max, pProcRes->ProcResV11.air_min,
                pProcRes->ProcResV11.tmax_base, pProcRes->ProcResV11.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV11(CalibDbDehazeV11_t* pCalibV11, RkAiqAdehazeProcResult_t* pProcRes,
                         unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth,
                         float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;
    int lo = 0, hi = 0;
    float ratio = GetInterpRatioV11(pCalibV11->enhance_setting.EnhanceData.CtrlData, lo, hi,
                                    CtrlValue, DHAZ_CTRL_DATA_STEP_MAX);

    pProcRes->ProcResV11.enhance_value =
        ClipValueV11(ratio * (pCalibV11->enhance_setting.EnhanceData.enhance_value[hi] -
                              pCalibV11->enhance_setting.EnhanceData.enhance_value[lo]) +
                         pCalibV11->enhance_setting.EnhanceData.enhance_value[lo],
                     4, 10);
    pProcRes->ProcResV11.enhance_chroma =
        ClipValueV11(ratio * (pCalibV11->enhance_setting.EnhanceData.enhance_chroma[hi] -
                              pCalibV11->enhance_setting.EnhanceData.enhance_chroma[lo]) +
                         pCalibV11->enhance_setting.EnhanceData.enhance_chroma[lo],
                     4, 10);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11.enhance_value -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11.enhance_value =
            LIMIT_VALUE(pProcRes->ProcResV11.enhance_value, BIT_14_MAX, BIT_MIN);
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11.enhance_chroma -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11.enhance_chroma =
            LIMIT_VALUE(pProcRes->ProcResV11.enhance_chroma, BIT_14_MAX, BIT_MIN);
    }

    for (int i = 0; i < DHAZ_V11_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV11.enh_curve[i] = (int)(pCalibV11->enhance_setting.enhance_curve[i]);

    if (pProcRes->ProcResV11.dc_en && pProcRes->ProcResV11.enhance_en) {
        LOGD_ADEHAZE(
            "%s CtrlValue:%f MEnhanceStrth:%d MEnhanceChromeStrth:%d enhance_value:%f "
            "enhance_chroma:%f\n",
            __func__, CtrlValue, MEnhanceStrth, MEnhanceChromeStrth,
            pProcRes->ProcResV11.enhance_value / 1024.0f,
            pProcRes->ProcResV11.enhance_chroma / 1024.0f);
        LOGV_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV11.enhance_value, pProcRes->ProcResV11.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV11(CalibDbDehazeV11_t* pCalibV11, RkAiqAdehazeProcResult_t* pProcRes,
                      float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    int lo = 0, hi = 0;
    float ratio = GetInterpRatioV11(pCalibV11->hist_setting.HistData.CtrlData, lo, hi, CtrlValue,
                                    DHAZ_CTRL_DATA_STEP_MAX);

    pProcRes->ProcResV11.hist_gratio =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.hist_gratio[hi] -
                              pCalibV11->hist_setting.HistData.hist_gratio[lo]) +
                         pCalibV11->hist_setting.HistData.hist_gratio[lo],
                     5, 3);
    pProcRes->ProcResV11.hist_th_off =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.hist_th_off[hi] -
                              pCalibV11->hist_setting.HistData.hist_th_off[lo]) +
                         pCalibV11->hist_setting.HistData.hist_th_off[lo],
                     8, 0);
    pProcRes->ProcResV11.hist_k =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.hist_k[hi] -
                              pCalibV11->hist_setting.HistData.hist_k[lo]) +
                         pCalibV11->hist_setting.HistData.hist_k[lo],
                     3, 2);
    pProcRes->ProcResV11.hist_min =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.hist_min[hi] -
                              pCalibV11->hist_setting.HistData.hist_min[lo]) +
                         pCalibV11->hist_setting.HistData.hist_min[lo],
                     1, 8);
    pProcRes->ProcResV11.cfg_gratio =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.cfg_gratio[hi] -
                              pCalibV11->hist_setting.HistData.cfg_gratio[lo]) +
                         pCalibV11->hist_setting.HistData.cfg_gratio[lo],
                     5, 8);
    pProcRes->ProcResV11.hist_scale =
        ClipValueV11(ratio * (pCalibV11->hist_setting.HistData.hist_scale[hi] -
                              pCalibV11->hist_setting.HistData.hist_scale[lo]) +
                         pCalibV11->hist_setting.HistData.hist_scale[lo],
                     5, 8);
    pProcRes->ProcResV11.hpara_en =
        pCalibV11->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV11.hpara_en = pProcRes->ProcResV11.dc_en
                                        ? pProcRes->ProcResV11.hpara_en
                                        : FUNCTION_ENABLE;  //  dc en 关闭，hpara必需开

    if (pProcRes->ProcResV11.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f CtrlValue:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV11.cfg_alpha / 255.0f, CtrlValue,
            pProcRes->ProcResV11.hpara_en, pProcRes->ProcResV11.hist_gratio / 255.0f,
            pProcRes->ProcResV11.hist_th_off / 1.0f, pProcRes->ProcResV11.hist_k / 4.0f,
            pProcRes->ProcResV11.hist_min / 256.0f, pProcRes->ProcResV11.hist_scale / 256.0f,
            pProcRes->ProcResV11.cfg_gratio / 256.0f);
        LOGV_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV11.cfg_alpha, pProcRes->ProcResV11.hist_gratio,
            pProcRes->ProcResV11.hist_th_off, pProcRes->ProcResV11.hist_k,
            pProcRes->ProcResV11.hist_min, pProcRes->ProcResV11.hist_scale,
            pProcRes->ProcResV11.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

#ifdef RKAIQ_ENABLE_CAMGROUP
XCamReturn AdehazeGetCurrDataGroup(AdehazeHandle_t* pAdehazeCtx,
                                   rk_aiq_singlecam_3a_result_t* pCamgrpParams) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    if (pCamgrpParams) {
        // get EnvLv
        if (pCamgrpParams->aec._aePreRes) {
            RkAiqAlgoPreResAe* pAEPreRes =
                (RkAiqAlgoPreResAe*)pCamgrpParams->aec._aePreRes->map(pCamgrpParams->aec._aePreRes);

            if (pAEPreRes) {
                switch (pAdehazeCtx->FrameNumber) {
                    case LINEAR_NUM:
                        pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                        break;
                    case HDR_2X_NUM:
                        pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    case HDR_3X_NUM:
                        pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    default:
                        LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                        break;
                }

                // Normalize the current envLv for AEC
                pAdehazeCtx->CurrDataV11.EnvLv =
                    (pAdehazeCtx->CurrDataV11.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
                pAdehazeCtx->CurrDataV11.EnvLv =
                    LIMIT_VALUE(pAdehazeCtx->CurrDataV11.EnvLv, ENVLVMAX, ENVLVMIN);
            } else {
                pAdehazeCtx->CurrDataV11.EnvLv = ENVLVMIN;
                LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
            }
        } else {
            pAdehazeCtx->CurrDataV11.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
        }

        // get iso
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV11.ISO =
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain * ISOMIN;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV11.ISO =
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV11.EnvLv = ENVLVMIN;
        pAdehazeCtx->CurrDataV11.ISO   = ISOMIN;
        LOGW_ADEHAZE("%s: camgroupParmasArray[0] Res is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}
#endif
XCamReturn AdehazeGetCurrData(AdehazeHandle_t* pAdehazeCtx, RkAiqAlgoProcAdhaz* pProcPara) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;

    // get EnvLv
    XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
    if (xCamAePreRes) {
        RkAiqAlgoPreResAe* pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);

        if (pAEPreRes) {
            switch (pAdehazeCtx->FrameNumber) {
                case LINEAR_NUM:
                    pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                    break;
                case HDR_2X_NUM:
                    pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                case HDR_3X_NUM:
                    pAdehazeCtx->CurrDataV11.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                default:
                    LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                    break;
            }
            // Normalize the current envLv for AEC
            pAdehazeCtx->CurrDataV11.EnvLv =
                (pAdehazeCtx->CurrDataV11.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
            pAdehazeCtx->CurrDataV11.EnvLv =
                LIMIT_VALUE(pAdehazeCtx->CurrDataV11.EnvLv, ENVLVMAX, ENVLVMIN);
        } else {
            pAdehazeCtx->CurrDataV11.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    } else {
        pAdehazeCtx->CurrDataV11.EnvLv = ENVLVMIN;
        LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
    }

    // get ISO
    if (pProcPara->com.u.proc.curExp) {
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV11.ISO =
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV11.ISO =
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV11.ISO = ISOMIN;
        LOGW_ADEHAZE("%s:AE cur expo is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}

XCamReturn AdehazeInit(AdehazeHandle_t** pAdehazeCtx, CamCalibDbV2Context_t* pCalib) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* handle = (AdehazeHandle_t*)calloc(1, sizeof(AdehazeHandle_t));

    CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V21 =
        (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib));
    memcpy(&handle->AdehazeAtrrV11.stAuto, calibv2_adehaze_calib_V21,
           sizeof(CalibDbV2_dehaze_v11_t));  // set default stauto
    handle->AdehazeAtrrV11.mode                                               = DEHAZE_API_AUTO;
    handle->AdehazeAtrrV11.Info.MDehazeStrth        = DEHAZE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV11.Info.MEnhanceStrth       = ENHANCE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV11.Info.MEnhanceChromeStrth = ENHANCE_DEFAULT_LEVEL;
    handle->ifReCalcStAuto                          = true;
    handle->ifReCalcStManual                        = false;
    handle->isCapture                               = false;
    handle->is_multi_isp_mode                       = false;

    *pAdehazeCtx = handle;
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeRelease(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pAdehazeCtx) free(pAdehazeCtx);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return (ret);
}

XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx, dehaze_stats_v11_t* pStats,
                          RkAiqAdehazeProcResult_t* pAdehzeProcRes) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 pAdehzeProcRes->enable,
                 (pAdehzeProcRes->ProcResV11.dc_en & FUNCTION_ENABLE) &&
                     (!(pAdehzeProcRes->ProcResV11.enhance_en & FUNCTION_ENABLE)),
                 (pAdehzeProcRes->ProcResV11.dc_en & FUNCTION_ENABLE) &&
                     (pAdehzeProcRes->ProcResV11.enhance_en & FUNCTION_ENABLE),
                 pAdehzeProcRes->ProcResV11.hist_en);

    if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_AUTO) {
        float CtrlValue = pAdehazeCtx->CurrDataV11.EnvLv;
        if (pAdehazeCtx->CurrDataV11.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdehazeCtx->CurrDataV11.ISO;

        // cfg setting
        pAdehzeProcRes->ProcResV11.cfg_alpha =
            LIMIT_VALUE(SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.cfg_alpha),
                        BIT_8_MAX, BIT_MIN);

        // dehaze setting
        if (pAdehzeProcRes->ProcResV11.dc_en)
            GetDehazeParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara, pAdehzeProcRes,
                               pAdehazeCtx->width, pAdehazeCtx->height,
                               pAdehazeCtx->AdehazeAtrrV11.Info.MDehazeStrth, CtrlValue);

        // enhance setting
        // enhance curve is effective in dehaze function. when dc_en on, GetEnhanceParamsV11 is on
        if (pAdehzeProcRes->ProcResV11.dc_en)
            GetEnhanceParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara,
                                pAdehzeProcRes, pAdehazeCtx->AdehazeAtrrV11.Info.MEnhanceStrth,
                                pAdehazeCtx->AdehazeAtrrV11.Info.MEnhanceChromeStrth, CtrlValue);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.hist_setting.en)
            GetHistParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara, pAdehzeProcRes,
                             CtrlValue);
    } else if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_MANUAL) {
        // cfg setting
        pAdehzeProcRes->ProcResV11.cfg_alpha = LIMIT_VALUE(
            SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV11.stManual.cfg_alpha), BIT_8_MAX, BIT_MIN);

        // dehaze setting
        if (pAdehzeProcRes->ProcResV11.dc_en)
            stManuGetDehazeParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stManual, pAdehzeProcRes,
                                     pAdehazeCtx->width, pAdehazeCtx->height,
                                     pAdehazeCtx->AdehazeAtrrV11.Info.MDehazeStrth);

        // enhance setting
        // enhance curve is effective in dehaze function. when dc_en on, stManuGetEnhanceParamsV11
        // is on
        if (pAdehzeProcRes->ProcResV11.dc_en)
            stManuGetEnhanceParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stManual, pAdehzeProcRes,
                                      pAdehazeCtx->AdehazeAtrrV11.Info.MEnhanceStrth,
                                      pAdehazeCtx->AdehazeAtrrV11.Info.MEnhanceChromeStrth);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV11.stManual.hist_setting.en)
            stManuGetHistParamsV11(&pAdehazeCtx->AdehazeAtrrV11.stManual, pAdehzeProcRes);
    } else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    // store pre data
    pAdehazeCtx->PreDataV11.EnvLv = pAdehazeCtx->CurrDataV11.EnvLv;
    pAdehazeCtx->PreDataV11.ISO     = pAdehazeCtx->CurrDataV11.ISO;
    pAdehazeCtx->PreDataV11.ApiMode = pAdehazeCtx->CurrDataV11.ApiMode;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool byPassProc = true;

    pAdehazeCtx->CurrDataV11.CtrlDataType =
        pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.CtrlDataType;

    if (pAdehazeCtx->FrameID <= INIT_CALC_PARAMS_NUM)
        byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV11.mode != pAdehazeCtx->PreDataV11.ApiMode)
        byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_MANUAL)
        byPassProc = !pAdehazeCtx->ifReCalcStManual;
    else if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_AUTO) {
        float diff = 0.0f;
        if (pAdehazeCtx->CurrDataV11.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdehazeCtx->PreDataV11.EnvLv - pAdehazeCtx->CurrDataV11.EnvLv;
            if (pAdehazeCtx->PreDataV11.EnvLv <= FLT_EPSILON) {
                diff = pAdehazeCtx->CurrDataV11.EnvLv;
                if (diff <= FLT_EPSILON)
                    byPassProc = true;
                else
                    byPassProc = false;
            } else {
                diff /= pAdehazeCtx->PreDataV11.EnvLv;
                if (diff >= pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.ByPassThr ||
                    diff <= -pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.ByPassThr)
                    byPassProc = false;
                else
                    byPassProc = true;
            }
        } else if (pAdehazeCtx->CurrDataV11.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdehazeCtx->PreDataV11.ISO - pAdehazeCtx->CurrDataV11.ISO;
            diff /= pAdehazeCtx->PreDataV11.ISO;
            if (diff >= pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.ByPassThr ||
                diff <= -pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.ByPassThr)
                byPassProc = false;
            else
                byPassProc = true;
        }
        byPassProc = byPassProc && !pAdehazeCtx->ifReCalcStAuto;
    }

    LOGD_ADEHAZE(
        "%s:FrameID:%d DehazeApiMode:%d ifReCalcStAuto:%d ifReCalcStManual:%d CtrlDataType:%d "
        "EnvLv:%f ISO:%f byPassProc:%d\n",
        __func__, pAdehazeCtx->FrameID, pAdehazeCtx->AdehazeAtrrV11.mode,
        pAdehazeCtx->ifReCalcStAuto, pAdehazeCtx->ifReCalcStManual,
        pAdehazeCtx->CurrDataV11.CtrlDataType, pAdehazeCtx->CurrDataV11.EnvLv,
        pAdehazeCtx->CurrDataV11.ISO, byPassProc);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return byPassProc;
}
/******************************************************************************
 * DehazeEnableSetting()
 *
 *****************************************************************************/
bool DehazeEnableSetting(AdehazeHandle_t* pAdehazeCtx, RkAiqAdehazeProcResult_t* pAdehzeProcRes) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_AUTO) {
        pAdehzeProcRes->enable = pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.Enable;

        if (pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_DISABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV11.stAuto.DehazeTuningPara.hist_setting.en)
                pAdehzeProcRes->ProcResV11.hist_en = FUNCTION_ENABLE;
            else
                pAdehzeProcRes->ProcResV11.hist_en = FUNCTION_DISABLE;
        }
    } else if (pAdehazeCtx->AdehazeAtrrV11.mode == DEHAZE_API_MANUAL) {
        pAdehzeProcRes->enable = pAdehazeCtx->AdehazeAtrrV11.stManual.Enable;

        if (pAdehazeCtx->AdehazeAtrrV11.stManual.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV11.stManual.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV11.stManual.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV11.stManual.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV11.stManual.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV11.stManual.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV11.stManual.enhance_setting.en) {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_ENABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehzeProcRes->ProcResV11.dc_en      = FUNCTION_DISABLE;
                pAdehzeProcRes->ProcResV11.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV11.stManual.hist_setting.en)
                pAdehzeProcRes->ProcResV11.hist_en = FUNCTION_ENABLE;
            else
                pAdehzeProcRes->ProcResV11.hist_en = FUNCTION_DISABLE;
        }
    } else {
        LOGE_ADEHAZE("%s: Dehaze api in WRONG MODE!!!, dehaze by pass!!!\n", __FUNCTION__);
        pAdehzeProcRes->enable = false;
    }

    return pAdehzeProcRes->enable;
    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}
