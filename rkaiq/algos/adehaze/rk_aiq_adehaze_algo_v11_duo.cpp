/*
 * rk_aiq_adehaze_algo_v11_duo.cpp
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
#include "rk_aiq_adehaze_algo_v11_duo.h"
#include <string.h>
#include "xcam_log.h"

float LinearInterpV11Duo(const float* pX, const float* pY, float posx, int XSize) {
    int index;
    float yOut = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    return yOut;
}

int DehazeLinearInterpV11Duo(const float* pX, const float* pY, float posx, int BitInt, int BitFlaot,
                             int XSize) {
    int index;
    float yOut     = 0.0;
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFlaot + BitInt)) - 1);
    int yOutIntMin = 0;

    if (posx >= pX[XSize - 1]) {
        yOut = pY[XSize - 1];
    } else if (posx <= pX[0]) {
        yOut = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
               pY[index];
    }

    yOutInt = LIMIT_VALUE((int)(yOut * pow(2, BitFlaot)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

int LinearInterpEnableV11Duo(const float* pX, const unsigned char* pY, float posx, int XSize) {
    int index;
    float out;
    int yOut = 0;
    if (posx >= pX[XSize - 1]) {
        out = (float)pY[XSize - 1];
    } else if (posx <= pX[0]) {
        out = pY[0];
    } else {
        index = 0;
        while ((posx >= pX[index]) && (index < XSize)) {
            index++;
        }
        index -= 1;
        out = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index])) +
              pY[index];
    }
    yOut = out > 0.5 ? FUNCTION_ENABLE : FUNCTION_DISABLE;

    return yOut;
}

int ClipValueV11Duo(float posx, int BitInt, int BitFloat) {
    int yOutInt    = 0;
    int yOutIntMax = (int)(pow(2, (BitFloat + BitInt)) - 1);
    int yOutIntMin = 0;

    yOutInt = LIMIT_VALUE((int)(posx * pow(2, BitFloat)), yOutIntMax, yOutIntMin);

    return yOutInt;
}

void stManuGetDehazeParamsV11duo(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                                 int rawWidth, int rawHeight, unsigned int MDehazeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11duo.air_lc_en =
        pStManu->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV11duo.dc_min_th =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.dc_min_th, 8, 0);
    pProcRes->ProcResV11duo.dc_max_th =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.dc_max_th, 8, 0);
    pProcRes->ProcResV11duo.yhist_th =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.yhist_th, 8, 0);
    pProcRes->ProcResV11duo.yblk_th = int(pStManu->dehaze_setting.DehazeData.yblk_th *
                                          ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16));
    pProcRes->ProcResV11duo.dark_th =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.dark_th, 8, 0);
    pProcRes->ProcResV11duo.bright_min =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.bright_min, 8, 0);
    pProcRes->ProcResV11duo.bright_max =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.bright_max, 8, 0);
    pProcRes->ProcResV11duo.wt_max =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.wt_max, 0, 8);
    pProcRes->ProcResV11duo.air_min =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.air_min, 8, 0);
    pProcRes->ProcResV11duo.air_max =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.air_max, 8, 0);
    pProcRes->ProcResV11duo.tmax_base =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.tmax_base, 8, 0);
    pProcRes->ProcResV11duo.tmax_off =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.tmax_off, 0, 10);
    pProcRes->ProcResV11duo.tmax_max =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.tmax_max, 0, 10);
    pProcRes->ProcResV11duo.cfg_wt =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.cfg_wt, 0, 8);
    pProcRes->ProcResV11duo.cfg_air =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.cfg_air, 8, 0);
    pProcRes->ProcResV11duo.cfg_tmax =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.cfg_tmax, 0, 10);
    pProcRes->ProcResV11duo.range_sima =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.range_sigma, 0, 8);
    pProcRes->ProcResV11duo.space_sigma_cur =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.space_sigma_cur, 0, 8);
    pProcRes->ProcResV11duo.space_sigma_pre =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.space_sigma_pre, 0, 8);
    pProcRes->ProcResV11duo.bf_weight =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.bf_weight, 0, 8);
    pProcRes->ProcResV11duo.dc_weitcur =
        ClipValueV11Duo(pStManu->dehaze_setting.DehazeData.dc_weitcur, 0, 8);
    pProcRes->ProcResV11duo.stab_fnum    = ClipValueV11Duo(pStManu->dehaze_setting.stab_fnum, 5, 0);
    if (pStManu->dehaze_setting.sigma)
        pProcRes->ProcResV11duo.iir_sigma =
            LIMIT_VALUE(int(256.0f / pStManu->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV11duo.iir_sigma = 0x1;
    if (pStManu->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV11duo.iir_wt_sigma =
            LIMIT_VALUE(int(1024.0f / (8.0f * pStManu->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV11duo.iir_wt_sigma = 0x7ff;
    if (pStManu->dehaze_setting.air_sigma)
        pProcRes->ProcResV11duo.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pStManu->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV11duo.iir_air_sigma = 0x8;
    if (pStManu->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV11duo.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pStManu->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV11duo.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV11duo.iir_pre_wet =
        LIMIT_VALUE(int(pStManu->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV11duo.gaus_h0     = DEHAZE_GAUS_H4;
    pProcRes->ProcResV11duo.gaus_h1     = DEHAZE_GAUS_H1;
    pProcRes->ProcResV11duo.gaus_h2     = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV11duo.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff           = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11duo.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11duo.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV11duo.cfg_wt -= level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11duo.cfg_air -= level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11duo.cfg_tmax -= level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        }
        pProcRes->ProcResV11duo.cfg_wt =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_wt, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11duo.cfg_air =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_air, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11duo.cfg_tmax =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_tmax, BIT_10_MAX, BIT_MIN);
    }

    if (pProcRes->ProcResV11duo.dc_en && !(pProcRes->ProcResV11duo.enhance_en)) {
        if (pProcRes->ProcResV11duo.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         pProcRes->ProcResV11duo.cfg_air / 1.0f,
                         pProcRes->ProcResV11duo.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV11duo.cfg_wt / 255.0f);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV11duo.cfg_air,
                         pProcRes->ProcResV11duo.cfg_tmax, pProcRes->ProcResV11duo.cfg_wt);
        } else if (pProcRes->ProcResV11duo.cfg_alpha == 0) {
            LOGD_ADEHAZE(
                "%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__,
                pProcRes->ProcResV11duo.air_max / 1.0f, pProcRes->ProcResV11duo.air_min / 1.0f,
                pProcRes->ProcResV11duo.tmax_base / 1.0f, pProcRes->ProcResV11duo.wt_max / 255.0f);
            LOGD_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV11duo.air_max, pProcRes->ProcResV11duo.air_min,
                pProcRes->ProcResV11duo.tmax_base, pProcRes->ProcResV11duo.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV11duo(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes,
                                  unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool level_up           = false;
    unsigned int level_diff = 0;

    pProcRes->ProcResV11duo.enhance_value =
        ClipValueV11Duo(pStManu->enhance_setting.EnhanceData.enhance_value, 4, 10);
    pProcRes->ProcResV11duo.enhance_chroma =
        ClipValueV11Duo(pStManu->enhance_setting.EnhanceData.enhance_chroma, 4, 10);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11duo.enhance_value -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11duo.enhance_value =
            LIMIT_VALUE(pProcRes->ProcResV11duo.enhance_value, BIT_14_MAX, BIT_MIN);
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11duo.enhance_chroma -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11duo.enhance_chroma =
            LIMIT_VALUE(pProcRes->ProcResV11duo.enhance_chroma, BIT_14_MAX, BIT_MIN);
    }

    for (int i = 0; i < DHAZ_V11_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV11duo.enh_curve[i] = (int)(pStManu->enhance_setting.enhance_curve[i]);

    if (pProcRes->ProcResV11duo.dc_en && pProcRes->ProcResV11duo.enhance_en) {
        LOGD_ADEHAZE("%s enhance_value:%f enhance_chroma:%f\n", __func__,
                     pStManu->enhance_setting.EnhanceData.enhance_value / 1024.0f,
                     pStManu->enhance_setting.EnhanceData.enhance_chroma / 1024.0f);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV11duo.enhance_value, pProcRes->ProcResV11duo.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV11duo(mDehazeAttrV11_t* pStManu, RkAiqAdehazeProcResult_t* pProcRes) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11duo.hpara_en =
        pStManu->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV11duo.hpara_en =
        pProcRes->ProcResV11duo.dc_en ? pProcRes->ProcResV11duo.hpara_en : FUNCTION_ENABLE;

    pProcRes->ProcResV11duo.hist_gratio =
        ClipValueV11Duo(pStManu->hist_setting.HistData.hist_gratio, 0, 8);
    pProcRes->ProcResV11duo.hist_th_off =
        ClipValueV11Duo(pStManu->hist_setting.HistData.hist_th_off, 8, 0);
    pProcRes->ProcResV11duo.hist_k = ClipValueV11Duo(pStManu->hist_setting.HistData.hist_k, 3, 2);
    pProcRes->ProcResV11duo.hist_min =
        ClipValueV11Duo(pStManu->hist_setting.HistData.hist_min, 1, 8);
    pProcRes->ProcResV11duo.cfg_gratio =
        ClipValueV11Duo(pStManu->hist_setting.HistData.cfg_gratio, 5, 8);
    pProcRes->ProcResV11duo.hist_scale =
        ClipValueV11Duo(pStManu->hist_setting.HistData.hist_scale, 5, 8);

    if (pProcRes->ProcResV11duo.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV11duo.cfg_alpha / 255.0f, pProcRes->ProcResV11duo.hpara_en,
            pProcRes->ProcResV11duo.hist_gratio / 255.0f,
            pProcRes->ProcResV11duo.hist_th_off / 1.0f, pProcRes->ProcResV11duo.hist_k / 4.0f,
            pProcRes->ProcResV11duo.hist_min / 256.0f, pProcRes->ProcResV11duo.hist_scale / 256.0f,
            pProcRes->ProcResV11duo.cfg_gratio / 256.0f);
        LOGD_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV11duo.cfg_alpha, pProcRes->ProcResV11duo.hist_gratio,
            pProcRes->ProcResV11duo.hist_th_off, pProcRes->ProcResV11duo.hist_k,
            pProcRes->ProcResV11duo.hist_min, pProcRes->ProcResV11duo.hist_scale,
            pProcRes->ProcResV11duo.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV11duo(CalibDbDehazeV11_t* pCalibV11Duo, RkAiqAdehazeProcResult_t* pProcRes,
                           int rawWidth, int rawHeight, unsigned int MDehazeStrth,
                           float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11duo.air_lc_en =
        pCalibV11Duo->dehaze_setting.air_lc_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    pProcRes->ProcResV11duo.dc_min_th =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.dc_min_th, CtrlValue, 8, 0,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.dc_max_th =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.dc_max_th, CtrlValue, 8, 0,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.yhist_th = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.yhist_th, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.yblk_th =
        LinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                           pCalibV11Duo->dehaze_setting.DehazeData.yblk_th, CtrlValue,
                           DHAZ_CTRL_DATA_STEP_MAX) *
        ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16);
    pProcRes->ProcResV11duo.dark_th = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.dark_th, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.bright_min =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.bright_min, CtrlValue, 8,
                                 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.bright_max =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.bright_max, CtrlValue, 8,
                                 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.wt_max = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.wt_max, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.air_min = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.air_min, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.air_max = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.air_max, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.tmax_base =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.tmax_base, CtrlValue, 8, 0,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.tmax_off =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.tmax_off, CtrlValue, 0, 10,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.tmax_max =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.tmax_max, CtrlValue, 0, 10,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.cfg_wt = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.cfg_wt, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.cfg_air = DehazeLinearInterpV11Duo(
        pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
        pCalibV11Duo->dehaze_setting.DehazeData.cfg_air, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.cfg_tmax =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.cfg_tmax, CtrlValue, 0, 10,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.range_sima =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.range_sigma, CtrlValue, 0,
                                 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.space_sigma_cur =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.space_sigma_cur, CtrlValue,
                                 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.space_sigma_pre =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.space_sigma_pre, CtrlValue,
                                 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.bf_weight =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.bf_weight, CtrlValue, 0, 8,
                                 DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.dc_weitcur =
        DehazeLinearInterpV11Duo(pCalibV11Duo->dehaze_setting.DehazeData.CtrlData,
                                 pCalibV11Duo->dehaze_setting.DehazeData.dc_weitcur, CtrlValue, 0,
                                 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.stab_fnum =
        ClipValueV11Duo(pCalibV11Duo->dehaze_setting.stab_fnum, 5, 0);
    if (pCalibV11Duo->dehaze_setting.sigma)
        pProcRes->ProcResV11duo.iir_sigma =
            LIMIT_VALUE(int(256.0f / pCalibV11Duo->dehaze_setting.sigma), 255, 0);
    else
        pProcRes->ProcResV11duo.iir_sigma = 0x1;
    if (pCalibV11Duo->dehaze_setting.wt_sigma >= 0.0f)
        pProcRes->ProcResV11duo.iir_wt_sigma = LIMIT_VALUE(
            int(1024.0f / (8.0f * pCalibV11Duo->dehaze_setting.wt_sigma + 0.5f)), 0x7ff, 0);
    else
        pProcRes->ProcResV11duo.iir_wt_sigma = 0x7ff;
    if (pCalibV11Duo->dehaze_setting.air_sigma)
        pProcRes->ProcResV11duo.iir_air_sigma =
            LIMIT_VALUE(int(1024.0f / pCalibV11Duo->dehaze_setting.air_sigma), 255, 0);
    else
        pProcRes->ProcResV11duo.iir_air_sigma = 0x8;
    if (pCalibV11Duo->dehaze_setting.tmax_sigma)
        pProcRes->ProcResV11duo.iir_tmax_sigma =
            LIMIT_VALUE(int(1.0f / pCalibV11Duo->dehaze_setting.tmax_sigma), 0x7ff, 0);
    else
        pProcRes->ProcResV11duo.iir_tmax_sigma = 0x5f;
    pProcRes->ProcResV11duo.iir_pre_wet =
        LIMIT_VALUE(int(pCalibV11Duo->dehaze_setting.pre_wet - 1.0f), 15, 0);
    pProcRes->ProcResV11duo.gaus_h0 = DEHAZE_GAUS_H4;
    pProcRes->ProcResV11duo.gaus_h1 = DEHAZE_GAUS_H1;
    pProcRes->ProcResV11duo.gaus_h2 = DEHAZE_GAUS_H0;

    // add for rk_aiq_uapi2_setMDehazeStrth
    if (MDehazeStrth != DEHAZE_DEFAULT_LEVEL) {
        pProcRes->ProcResV11duo.cfg_alpha = BIT_8_MAX;
        unsigned int level_diff           = MDehazeStrth > DEHAZE_DEFAULT_LEVEL
                                      ? (MDehazeStrth - DEHAZE_DEFAULT_LEVEL)
                                      : (DEHAZE_DEFAULT_LEVEL - MDehazeStrth);
        bool level_up = MDehazeStrth > DEHAZE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.cfg_wt += level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11duo.cfg_air += level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11duo.cfg_tmax += level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        } else {
            pProcRes->ProcResV11duo.cfg_wt -= level_diff * DEHAZE_DEFAULT_CFG_WT_STEP;
            pProcRes->ProcResV11duo.cfg_air -= level_diff * DEHAZE_DEFAULT_CFG_AIR_STEP;
            pProcRes->ProcResV11duo.cfg_tmax -= level_diff * DEHAZE_DEFAULT_CFG_TMAX_STEP;
        }
        pProcRes->ProcResV11duo.cfg_wt =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_wt, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11duo.cfg_air =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_air, BIT_8_MAX, BIT_MIN);
        pProcRes->ProcResV11duo.cfg_tmax =
            LIMIT_VALUE(pProcRes->ProcResV11duo.cfg_tmax, BIT_10_MAX, BIT_MIN);
    }

    if (pProcRes->ProcResV11duo.dc_en && !(pProcRes->ProcResV11duo.enhance_en)) {
        if (pProcRes->ProcResV11duo.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 CtrlValue:%f cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__,
                         CtrlValue, pProcRes->ProcResV11duo.cfg_air / 1.0f,
                         pProcRes->ProcResV11duo.cfg_tmax / 1023.0f,
                         pProcRes->ProcResV11duo.cfg_wt / 255.0f);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n",
                         __func__, pProcRes->ProcResV11duo.cfg_air,
                         pProcRes->ProcResV11duo.cfg_tmax, pProcRes->ProcResV11duo.cfg_wt);
        } else if (pProcRes->ProcResV11duo.cfg_alpha == 0) {
            LOGD_ADEHAZE(
                "%s cfg_alpha:0 CtrlValue:%f air_max:%f air_min:%f tmax_base:%f wt_max:%f\n",
                __func__, CtrlValue, pProcRes->ProcResV11duo.air_max / 1.0f,
                pProcRes->ProcResV11duo.air_min / 1.0f, pProcRes->ProcResV11duo.tmax_base / 1.0f,
                pProcRes->ProcResV11duo.wt_max / 255.0f);
            LOGD_ADEHAZE(
                "%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n",
                __func__, pProcRes->ProcResV11duo.air_max, pProcRes->ProcResV11duo.air_min,
                pProcRes->ProcResV11duo.tmax_base, pProcRes->ProcResV11duo.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV11duo(CalibDbDehazeV11_t* pCalibV11Duo, RkAiqAdehazeProcResult_t* pProcRes,
                            unsigned int MEnhanceStrth, unsigned int MEnhanceChromeStrth,
                            float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    bool level_up           = false;
    unsigned int level_diff = 0;

    pProcRes->ProcResV11duo.enhance_value =
        DehazeLinearInterpV11Duo(pCalibV11Duo->enhance_setting.EnhanceData.CtrlData,
                                 pCalibV11Duo->enhance_setting.EnhanceData.enhance_value, CtrlValue,
                                 4, 10, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.enhance_chroma =
        DehazeLinearInterpV11Duo(pCalibV11Duo->enhance_setting.EnhanceData.CtrlData,
                                 pCalibV11Duo->enhance_setting.EnhanceData.enhance_chroma,
                                 CtrlValue, 4, 10, DHAZ_CTRL_DATA_STEP_MAX);

    // add for rk_aiq_uapi2_setMEnhanceStrth
    if (MEnhanceStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceStrth);
        level_up = MEnhanceStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.enhance_value += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11duo.enhance_value -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11duo.enhance_value =
            LIMIT_VALUE(pProcRes->ProcResV11duo.enhance_value, BIT_14_MAX, BIT_MIN);
    }

    // add for rk_aiq_uapi2_setMEnhanceChromeStrth
    if (MEnhanceChromeStrth != ENHANCE_DEFAULT_LEVEL) {
        level_diff = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL
                         ? (MEnhanceChromeStrth - ENHANCE_DEFAULT_LEVEL)
                         : (ENHANCE_DEFAULT_LEVEL - MEnhanceChromeStrth);
        level_up = MEnhanceChromeStrth > ENHANCE_DEFAULT_LEVEL;
        if (level_up) {
            pProcRes->ProcResV11duo.enhance_chroma += level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        } else {
            pProcRes->ProcResV11duo.enhance_chroma -= level_diff * ENHANCE_VALUE_DEFAULT_STEP;
        }
        pProcRes->ProcResV11duo.enhance_chroma =
            LIMIT_VALUE(pProcRes->ProcResV11duo.enhance_chroma, BIT_14_MAX, BIT_MIN);
    }

    for (int i = 0; i < DHAZ_V11_ENHANCE_CRUVE_NUM; i++)
        pProcRes->ProcResV11duo.enh_curve[i] =
            (int)(pCalibV11Duo->enhance_setting.enhance_curve[i]);

    if (pProcRes->ProcResV11duo.dc_en && pProcRes->ProcResV11duo.enhance_en) {
        LOGD_ADEHAZE("%s CtrlValue:%f enhance_value:%f enhance_chroma:%f\n", __func__, CtrlValue,
                     pProcRes->ProcResV11duo.enhance_value / 1024.0f,
                     pProcRes->ProcResV11duo.enhance_chroma / 1024.0f);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     pProcRes->ProcResV11duo.enhance_value, pProcRes->ProcResV11duo.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV11duo(CalibDbDehazeV11_t* pCalibV11Duo, RkAiqAdehazeProcResult_t* pProcRes,
                         float CtrlValue) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pProcRes->ProcResV11duo.hpara_en =
        pCalibV11Duo->hist_setting.hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    // clip hpara_en
    pProcRes->ProcResV11duo.hpara_en = pProcRes->ProcResV11duo.dc_en
                                           ? pProcRes->ProcResV11duo.hpara_en
                                           : FUNCTION_ENABLE;  //  dc en �رգ�hpara���迪

    pProcRes->ProcResV11duo.hist_gratio = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData,
        pCalibV11Duo->hist_setting.HistData.hist_gratio, CtrlValue, 0, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.hist_th_off = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData,
        pCalibV11Duo->hist_setting.HistData.hist_th_off, CtrlValue, 8, 0, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.hist_k = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData, pCalibV11Duo->hist_setting.HistData.hist_k,
        CtrlValue, 3, 2, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.hist_min = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData, pCalibV11Duo->hist_setting.HistData.hist_min,
        CtrlValue, 1, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.cfg_gratio = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData,
        pCalibV11Duo->hist_setting.HistData.cfg_gratio, CtrlValue, 5, 8, DHAZ_CTRL_DATA_STEP_MAX);
    pProcRes->ProcResV11duo.hist_scale = DehazeLinearInterpV11Duo(
        pCalibV11Duo->hist_setting.HistData.CtrlData,
        pCalibV11Duo->hist_setting.HistData.hist_scale, CtrlValue, 5, 8, DHAZ_CTRL_DATA_STEP_MAX);

    if (pProcRes->ProcResV11duo.hist_en) {
        LOGD_ADEHAZE(
            "%s cfg_alpha:%f CtrlValue:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f "
            "hist_min:%f hist_scale:%f cfg_gratio:%f\n",
            __func__, pProcRes->ProcResV11duo.cfg_alpha / 255.0f, CtrlValue,
            pProcRes->ProcResV11duo.hpara_en, pProcRes->ProcResV11duo.hist_gratio / 255.0f,
            pProcRes->ProcResV11duo.hist_th_off / 1.0f, pProcRes->ProcResV11duo.hist_k / 4.0f,
            pProcRes->ProcResV11duo.hist_min / 256.0f, pProcRes->ProcResV11duo.hist_scale / 256.0f,
            pProcRes->ProcResV11duo.cfg_gratio / 256.0f);
        LOGD_ADEHAZE(
            "%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x "
            "hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n",
            __func__, pProcRes->ProcResV11duo.cfg_alpha, pProcRes->ProcResV11duo.hist_gratio,
            pProcRes->ProcResV11duo.hist_th_off, pProcRes->ProcResV11duo.hist_k,
            pProcRes->ProcResV11duo.hist_min, pProcRes->ProcResV11duo.hist_scale,
            pProcRes->ProcResV11duo.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeHistDuoISPSettingV11(RkAiqAdehazeProcResult_t* pProcRes,
                                   rkisp_adehaze_stats_t* pStats, bool DuoCamera, int FrameID) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    // round_en
    pProcRes->ProcResV11duo.round_en = FUNCTION_ENABLE;

    // deahze duo setting
    if (DuoCamera) {
        pProcRes->ProcResV11duo.soft_wr_en = FUNCTION_ENABLE;
#if 1
        // support default value for kernel calc
        for (int i = 0; i < DHAZ_V11_HIST_WR_NUM; i++) {
            pProcRes->ProcResV11duo.hist_wr[i] = 16 * (i + 1);
            pProcRes->ProcResV11duo.hist_wr[i] = pProcRes->ProcResV11duo.hist_wr[i] > 1023
                                                     ? 1023
                                                     : pProcRes->ProcResV11duo.hist_wr[i];
        }
#else
        pProcRes->ProcResV11duo.adp_air_wr    = pStats->dehaze_stats_v11_duo.dhaz_adp_air_base;
        pProcRes->ProcResV11duo.adp_gratio_wr = pStats->dehaze_stats_v11_duo.dhaz_adp_gratio;
        pProcRes->ProcResV11duo.adp_tmax_wr   = pStats->dehaze_stats_v11_duo.dhaz_adp_tmax;
        pProcRes->ProcResV11duo.adp_wt_wr     = pStats->dehaze_stats_v11_duo.dhaz_adp_wt;

        static int hist_wr[64];
        if (!FrameID)
            for (int i = 0; i < 64; i++) {
                hist_wr[i]                         = 16 * (i + 1);
                hist_wr[i]                         = hist_wr[i] > 1023 ? 1023 : hist_wr[i];
                pProcRes->ProcResV11duo.hist_wr[i] = hist_wr[i];
            }
        else {
            int num = MIN(FrameID + 1, pProcRes->ProcResV11duo.stab_fnum);
            int tmp = 0;
            for (int i = 0; i < 64; i++) {
                tmp = (hist_wr[i] * (num - 1) + pStats->dehaze_stats_v11_duo.h_rgb_iir[i]) / num;
                pProcRes->ProcResV11duo.hist_wr[i] = tmp;
                hist_wr[i]                         = tmp;
            }
        }

        LOGD_ADEHAZE("%s adp_air_wr:0x%x adp_gratio_wr:0x%x adp_tmax_wr:0x%x adp_wt_wr:0x%x\n",
                     __func__, pProcRes->ProcResV11duo.adp_air_wr,
                     pProcRes->ProcResV11duo.adp_gratio_wr, pProcRes->ProcResV11duo.adp_tmax_wr,
                     pProcRes->ProcResV11duo.adp_wt_wr);

        LOGV_ADEHAZE("%s hist_wr:0x%x", __func__, pProcRes->ProcResV11duo.hist_wr[0]);
        for (int i = 1; i < 63; i++) LOGV_ADEHAZE(" 0x%x", pProcRes->ProcResV11duo.hist_wr[i]);
        LOGV_ADEHAZE(" 0x%x\n", pProcRes->ProcResV11duo.hist_wr[63]);
#endif
        LOGD_ADEHAZE("%s DuoCamera:%d soft_wr_en:%d\n", __func__, DuoCamera,
                     pProcRes->ProcResV11duo.soft_wr_en);
    } else
        pProcRes->ProcResV11duo.soft_wr_en = FUNCTION_DISABLE;

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

XCamReturn GetDehazeLocalGainSettingV11Duo(RkAiqAdehazeProcResult_t* pProcRes, float* sigma) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // get sigma_idx
    for (int i = 0; i < DHAZ_V11_SIGMA_IDX_NUM; i++)
        pProcRes->ProcResV11duo.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;

    // get sigma_lut
    float sigam_total = 0.0f;
    for (int i = 0; i < DHAZ_V11_SIGMA_IDX_NUM; i++) sigam_total += sigma[i];

    if (sigam_total < FLT_EPSILON) {
        for (int i = 0; i < DHAZ_V11_SIGMA_IDX_NUM; i++)
            pProcRes->ProcResV11duo.sigma_lut[i] = 0x200;
    } else {
        int tmp = 0;
        for (int i = 0; i < DHAZ_V11_SIGMA_LUT_NUM; i++) {
            tmp = LIMIT_VALUE(8.0f * sigma[i], BIT_10_MAX, BIT_MIN);
            pProcRes->ProcResV11duo.sigma_lut[i] = tmp;
        }
    }

    LOGV_ADEHAZE("%s(%d) ynr sigma(0~5): %f %f %f %f %f %f\n", __func__, __LINE__, sigma[0],
                 sigma[1], sigma[2], sigma[3], sigma[4], sigma[5]);
    LOGV_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
                 __LINE__, pProcRes->ProcResV11duo.sigma_idx[0],
                 pProcRes->ProcResV11duo.sigma_idx[1], pProcRes->ProcResV11duo.sigma_idx[2],
                 pProcRes->ProcResV11duo.sigma_idx[3], pProcRes->ProcResV11duo.sigma_idx[4],
                 pProcRes->ProcResV11duo.sigma_idx[5]);
    LOGV_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__,
                 __LINE__, pProcRes->ProcResV11duo.sigma_lut[0],
                 pProcRes->ProcResV11duo.sigma_lut[1], pProcRes->ProcResV11duo.sigma_lut[2],
                 pProcRes->ProcResV11duo.sigma_lut[3], pProcRes->ProcResV11duo.sigma_lut[4],
                 pProcRes->ProcResV11duo.sigma_lut[5]);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
    return ret;
}

void AdehazeGetStats(AdehazeHandle_t* pAdehazeCtx, rkisp_adehaze_stats_t* ROData) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_air_base =
        ROData->dehaze_stats_v11_duo.dhaz_adp_air_base;
    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_wt = ROData->dehaze_stats_v11_duo.dhaz_adp_wt;
    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_gratio =
        ROData->dehaze_stats_v11_duo.dhaz_adp_gratio;
    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_tmax =
        ROData->dehaze_stats_v11_duo.dhaz_adp_tmax;
    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_pic_sumh_left =
        ROData->dehaze_stats_v11_duo.dhaz_pic_sumh_left;
    pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_pic_sumh_right =
        ROData->dehaze_stats_v11_duo.dhaz_pic_sumh_right;
    for (int i = 0; i < DHAZ_V11_HIST_WR_NUM; i++)
        pAdehazeCtx->stats.dehaze_stats_v11_duo.h_rgb_iir[i] =
            ROData->dehaze_stats_v11_duo.h_rgb_iir[i];

    LOG1_ADEHAZE(
        "%s:  dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d dhaz_adp_tmax:%d "
        "dhaz_pic_sumh_left:%d dhaz_pic_sumh_right:%d\n",
        __FUNCTION__, pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_air_base,
        pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_wt,
        pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_gratio,
        pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_adp_tmax,
        pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_pic_sumh_left,
        pAdehazeCtx->stats.dehaze_stats_v11_duo.dhaz_pic_sumh_right);
    for (int i = 0; i < DHAZ_V11_HIST_WR_NUM; i++)
        LOG1_ADEHAZE("%s:  h_rgb_iir[%d]:%d:\n", __FUNCTION__, i,
                     pAdehazeCtx->stats.dehaze_stats_v11_duo.h_rgb_iir[i]);

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}
#ifdef RKAIQ_ENABLE_CAMGROUP
XCamReturn AdehazeGetCurrDataGroup(AdehazeHandle_t* pAdehazeCtx,
                                   rk_aiq_singlecam_3a_result_t* pCamgrpParams) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;

    // get ynr res
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++)
        pAdehazeCtx->YnrProcResV3_sigma[i] = pCamgrpParams->aynr_sigma._aynr_sigma_v3[i];

    if (pCamgrpParams) {
        // get EnvLv
        if (pCamgrpParams->aec._aePreRes) {
            RkAiqAlgoPreResAe* pAEPreRes =
                (RkAiqAlgoPreResAe*)pCamgrpParams->aec._aePreRes->map(pCamgrpParams->aec._aePreRes);

            if (pAEPreRes) {
                switch (pAdehazeCtx->FrameNumber) {
                    case LINEAR_NUM:
                        pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                        break;
                    case HDR_2X_NUM:
                        pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    case HDR_3X_NUM:
                        pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                        break;
                    default:
                        LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                        break;
                }

                // Normalize the current envLv for AEC
                pAdehazeCtx->CurrDataV11duo.EnvLv =
                    (pAdehazeCtx->CurrDataV11duo.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
                pAdehazeCtx->CurrDataV11duo.EnvLv =
                    LIMIT_VALUE(pAdehazeCtx->CurrDataV11duo.EnvLv, ENVLVMAX, ENVLVMIN);
            } else {
                pAdehazeCtx->CurrDataV11duo.EnvLv = ENVLVMIN;
                LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
            }
        } else {
            pAdehazeCtx->CurrDataV11duo.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:_aePreRes Res is NULL!\n", __FUNCTION__);
        }

        // get iso
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV11duo.ISO =
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain * ISOMIN;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV11duo.ISO =
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
                pCamgrpParams->aec._effAecExpInfo.HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV11duo.EnvLv = ENVLVMIN;
        pAdehazeCtx->CurrDataV11duo.ISO   = ISOMIN;
        LOGW_ADEHAZE("%s: camgroupParmasArray[0] Res is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}
#endif
XCamReturn AdehazeGetCurrData(AdehazeHandle_t* pAdehazeCtx, RkAiqAlgoProcAdhaz* pProcPara) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;

    // get ynr res
    for (int i = 0; i < YNR_V3_ISO_CURVE_POINT_NUM; i++)
        pAdehazeCtx->YnrProcResV3_sigma[i] = pProcPara->sigma_v3[i];

    // get EnvLv
    XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
    if (xCamAePreRes) {
        RkAiqAlgoPreResAe* pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);

        if (pAEPreRes) {
            switch (pAdehazeCtx->FrameNumber) {
                case LINEAR_NUM:
                    pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[0];
                    break;
                case HDR_2X_NUM:
                    pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                case HDR_3X_NUM:
                    pAdehazeCtx->CurrDataV11duo.EnvLv = pAEPreRes->ae_pre_res_rk.GlobalEnvLv[1];
                    break;
                default:
                    LOGE_ADEHAZE("%s:  Wrong frame number in HDR mode!!!\n", __FUNCTION__);
                    break;
            }
            // Normalize the current envLv for AEC
            pAdehazeCtx->CurrDataV11duo.EnvLv =
                (pAdehazeCtx->CurrDataV11duo.EnvLv - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
            pAdehazeCtx->CurrDataV11duo.EnvLv =
                LIMIT_VALUE(pAdehazeCtx->CurrDataV11duo.EnvLv, ENVLVMAX, ENVLVMIN);
        } else {
            pAdehazeCtx->CurrDataV11duo.EnvLv = ENVLVMIN;
            LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    } else {
        pAdehazeCtx->CurrDataV11duo.EnvLv = ENVLVMIN;
        LOGW_ADEHAZE("%s:PreResBuf is NULL!\n", __FUNCTION__);
    }

    // get ISO
    if (pProcPara->com.u.proc.curExp) {
        if (pAdehazeCtx->FrameNumber == LINEAR_NUM) {
            pAdehazeCtx->CurrDataV11duo.ISO =
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain * ISOMIN;
        } else if (pAdehazeCtx->FrameNumber == HDR_2X_NUM ||
                   pAdehazeCtx->FrameNumber == HDR_3X_NUM) {
            pAdehazeCtx->CurrDataV11duo.ISO =
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                pProcPara->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain * ISOMIN;
        }
    } else {
        pAdehazeCtx->CurrDataV11duo.ISO = ISOMIN;
        LOGW_ADEHAZE("%s:AE cur expo is NULL!\n", __FUNCTION__);
    }

    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
    return ret;
}

XCamReturn AdehazeInit(AdehazeHandle_t** pAdehazeCtx, CamCalibDbV2Context_t* pCalib) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret          = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* handle = (AdehazeHandle_t*)calloc(1, sizeof(AdehazeHandle_t));

    CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V11_duo =
        (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib));
    memcpy(&handle->AdehazeAtrrV11duo.stAuto, calibv2_adehaze_calib_V11_duo,
           sizeof(CalibDbV2_dehaze_v11_t));  // set defsult stAuto

    handle->PreDataV11duo.EnvLv   = 0.0;
    handle->PreDataV11duo.ApiMode = DEHAZE_API_AUTO;

    // set api default
    handle->AdehazeAtrrV11duo.mode                                               = DEHAZE_API_AUTO;

    handle->AdehazeAtrrV11duo.Info.ISO                 = ISOMIN;
    handle->AdehazeAtrrV11duo.Info.EnvLv               = ENVLVMIN;
    handle->AdehazeAtrrV11duo.Info.MDehazeStrth        = DEHAZE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV11duo.Info.MEnhanceStrth       = ENHANCE_DEFAULT_LEVEL;
    handle->AdehazeAtrrV11duo.Info.MEnhanceChromeStrth = ENHANCE_DEFAULT_LEVEL;

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

XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 pAdehazeCtx->ProcRes.enable,
                 (pAdehazeCtx->ProcRes.ProcResV11duo.dc_en & FUNCTION_ENABLE) &&
                     (!(pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en & FUNCTION_ENABLE)),
                 (pAdehazeCtx->ProcRes.ProcResV11duo.dc_en & FUNCTION_ENABLE) &&
                     (pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en & FUNCTION_ENABLE),
                 pAdehazeCtx->ProcRes.ProcResV11duo.hist_en);

    if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_AUTO) {
        float CtrlValue = pAdehazeCtx->CurrDataV11duo.EnvLv;
        if (pAdehazeCtx->CurrDataV11duo.CtrlDataType == CTRLDATATYPE_ISO)
            CtrlValue = pAdehazeCtx->CurrDataV11duo.ISO;

        // cfg setting
        pAdehazeCtx->ProcRes.ProcResV11duo.cfg_alpha =
            LIMIT_VALUE(SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.cfg_alpha),
                        BIT_8_MAX, BIT_MIN);

        // dehaze setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.enhance_setting.en ||
            (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.hist_setting.en &&
             !pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.hist_setting.hist_para_en))
            GetDehazeParamsV11duo(&pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara,
                                  &pAdehazeCtx->ProcRes, pAdehazeCtx->width, pAdehazeCtx->height,
                                  pAdehazeCtx->AdehazeAtrrV11duo.Info.MDehazeStrth, CtrlValue);

        // enhance setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.enhance_setting.en)
            GetEnhanceParamsV11duo(
                &pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara, &pAdehazeCtx->ProcRes,
                pAdehazeCtx->AdehazeAtrrV11duo.Info.MEnhanceStrth,
                pAdehazeCtx->AdehazeAtrrV11duo.Info.MEnhanceChromeStrth, CtrlValue);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.hist_setting.en)
            GetHistParamsV11duo(&pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara,
                                &pAdehazeCtx->ProcRes, CtrlValue);
    } else if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_MANUAL) {
        // cfg setting
        pAdehazeCtx->ProcRes.ProcResV11duo.cfg_alpha = LIMIT_VALUE(
            SHIFT8BIT(pAdehazeCtx->AdehazeAtrrV11duo.stManual.cfg_alpha), BIT_8_MAX, BIT_MIN);

        // dehaze setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.dehaze_setting.en ||
            pAdehazeCtx->AdehazeAtrrV11duo.stManual.enhance_setting.en ||
            (pAdehazeCtx->AdehazeAtrrV11duo.stManual.hist_setting.en &&
             !pAdehazeCtx->AdehazeAtrrV11duo.stManual.hist_setting.hist_para_en))
            stManuGetDehazeParamsV11duo(
                &pAdehazeCtx->AdehazeAtrrV11duo.stManual, &pAdehazeCtx->ProcRes, pAdehazeCtx->width,
                pAdehazeCtx->height, pAdehazeCtx->AdehazeAtrrV11duo.Info.MDehazeStrth);

        // enhance setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.enhance_setting.en)
            stManuGetEnhanceParamsV11duo(&pAdehazeCtx->AdehazeAtrrV11duo.stManual,
                                         &pAdehazeCtx->ProcRes,
                                         pAdehazeCtx->AdehazeAtrrV11duo.Info.MEnhanceStrth,
                                         pAdehazeCtx->AdehazeAtrrV11duo.Info.MEnhanceChromeStrth);

        // hist setting
        if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.hist_setting.en)
            stManuGetHistParamsV11duo(&pAdehazeCtx->AdehazeAtrrV11duo.stManual,
                                      &pAdehazeCtx->ProcRes);
    } else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    // get local gain setting
    ret = GetDehazeLocalGainSettingV11Duo(&pAdehazeCtx->ProcRes, pAdehazeCtx->YnrProcResV3_sigma);

    // get Duo cam setting
    GetDehazeHistDuoISPSettingV11(&pAdehazeCtx->ProcRes, &pAdehazeCtx->stats,
                                  pAdehazeCtx->is_multi_isp_mode, pAdehazeCtx->FrameID);

    // store pre data
    pAdehazeCtx->PreDataV11duo.EnvLv = pAdehazeCtx->CurrDataV11duo.EnvLv;
    pAdehazeCtx->PreDataV11duo.ISO     = pAdehazeCtx->CurrDataV11duo.ISO;
    pAdehazeCtx->PreDataV11duo.ApiMode = pAdehazeCtx->CurrDataV11duo.ApiMode;

    // store api info
    pAdehazeCtx->AdehazeAtrrV11duo.Info.ISO   = pAdehazeCtx->CurrDataV11duo.ISO;
    pAdehazeCtx->AdehazeAtrrV11duo.Info.EnvLv = pAdehazeCtx->CurrDataV11duo.EnvLv;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    pAdehazeCtx->CurrDataV11duo.CtrlDataType =
        pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.CtrlDataType;

    if (pAdehazeCtx->FrameID <= 2) pAdehazeCtx->byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV11duo.mode != pAdehazeCtx->PreDataV11duo.ApiMode)
        pAdehazeCtx->byPassProc = false;
    else if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_MANUAL)
        pAdehazeCtx->byPassProc = !pAdehazeCtx->ifReCalcStManual;
    else if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_AUTO) {
        float diff = 0.0f;
        if (pAdehazeCtx->CurrDataV11duo.CtrlDataType == CTRLDATATYPE_ENVLV) {
            diff = pAdehazeCtx->PreDataV11duo.EnvLv - pAdehazeCtx->CurrDataV11duo.EnvLv;
            if (pAdehazeCtx->PreDataV11duo.EnvLv == ENVLVMIN) {
                diff = pAdehazeCtx->CurrDataV11duo.EnvLv;
                if (diff == 0.0f)
                    pAdehazeCtx->byPassProc = true;
                else
                    pAdehazeCtx->byPassProc = false;
            } else {
                diff /= pAdehazeCtx->PreDataV11duo.EnvLv;
                if (diff >= pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.ByPassThr ||
                    diff <=
                        (0.0f - pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.ByPassThr))
                    pAdehazeCtx->byPassProc = false;
                else
                    pAdehazeCtx->byPassProc = true;
            }
        } else if (pAdehazeCtx->CurrDataV11duo.CtrlDataType == CTRLDATATYPE_ISO) {
            diff = pAdehazeCtx->PreDataV11duo.ISO - pAdehazeCtx->CurrDataV11duo.ISO;
            diff /= pAdehazeCtx->PreDataV11duo.ISO;
            if (diff >= pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.ByPassThr ||
                diff <= (0.0f - pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.ByPassThr))
                pAdehazeCtx->byPassProc = false;
            else
                pAdehazeCtx->byPassProc = true;
        }
        pAdehazeCtx->byPassProc = pAdehazeCtx->byPassProc && !pAdehazeCtx->ifReCalcStAuto;
    }

    LOGD_ADEHAZE(
        "%s:FrameID:%d DehazeApiMode:%d ifReCalcStAuto:%d ifReCalcStManual:%d CtrlDataType:%d "
        "EnvLv:%f ISO:%f byPassProc:%d\n",
        __func__, pAdehazeCtx->FrameID, pAdehazeCtx->AdehazeAtrrV11duo.mode,
        pAdehazeCtx->ifReCalcStAuto, pAdehazeCtx->ifReCalcStManual,
        pAdehazeCtx->CurrDataV11duo.CtrlDataType, pAdehazeCtx->CurrDataV11duo.EnvLv,
        pAdehazeCtx->CurrDataV11duo.ISO, pAdehazeCtx->byPassProc);

    pAdehazeCtx->ifReCalcStManual = false;
    pAdehazeCtx->ifReCalcStAuto   = false;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return pAdehazeCtx->byPassProc;
}
/******************************************************************************
 * DehazeEnableSetting()
 *
 *****************************************************************************/
bool DehazeEnableSetting(AdehazeHandle_t* pAdehazeCtx) {
    LOG1_ADEHAZE("%s:enter!\n", __FUNCTION__);

    if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_AUTO) {
        pAdehazeCtx->ProcRes.enable = pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.Enable;

        if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_DISABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV11duo.stAuto.DehazeTuningPara.hist_setting.en)
                pAdehazeCtx->ProcRes.ProcResV11duo.hist_en = FUNCTION_ENABLE;
            else
                pAdehazeCtx->ProcRes.ProcResV11duo.hist_en = FUNCTION_DISABLE;
        }
    } else if (pAdehazeCtx->AdehazeAtrrV11duo.mode == DEHAZE_API_MANUAL) {
        pAdehazeCtx->ProcRes.enable = pAdehazeCtx->AdehazeAtrrV11duo.stManual.Enable;

        if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.Enable) {
            if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.dehaze_setting.en &&
                pAdehazeCtx->AdehazeAtrrV11duo.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_ENABLE;
            } else if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.dehaze_setting.en &&
                       !pAdehazeCtx->AdehazeAtrrV11duo.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_DISABLE;
            } else if (!pAdehazeCtx->AdehazeAtrrV11duo.stManual.dehaze_setting.en &&
                       pAdehazeCtx->AdehazeAtrrV11duo.stManual.enhance_setting.en) {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_ENABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_ENABLE;
            } else {
                pAdehazeCtx->ProcRes.ProcResV11duo.dc_en      = FUNCTION_DISABLE;
                pAdehazeCtx->ProcRes.ProcResV11duo.enhance_en = FUNCTION_DISABLE;
            }

            if (pAdehazeCtx->AdehazeAtrrV11duo.stManual.hist_setting.en)
                pAdehazeCtx->ProcRes.ProcResV11duo.hist_en = FUNCTION_ENABLE;
            else
                pAdehazeCtx->ProcRes.ProcResV11duo.hist_en = FUNCTION_DISABLE;
        }
    } else {
        LOGE_ADEHAZE("%s: Dehaze api in WRONG MODE!!!, dehaze by pass!!!\n", __FUNCTION__);
        pAdehazeCtx->ProcRes.enable = false;
    }

    return pAdehazeCtx->ProcRes.enable;
    LOG1_ADEHAZE("%s:exit!\n", __FUNCTION__);
}
