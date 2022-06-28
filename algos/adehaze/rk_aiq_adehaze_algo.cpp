/*
 * rk_aiq_adehaze_algo.cpp
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
#include <string.h>
#include "rk_aiq_adehaze_algo.h"
#include "xcam_log.h"

float  LinearInterp(const float *pX, const float *pY, float posx, int XSize)
{
    int index;
    float yOut = 0;

    if (posx >= pX[XSize - 1])
    {
        yOut = pY[XSize - 1];
    }
    else if (posx <= pX[0])
    {
        yOut = pY[0];
    }
    else
    {
        index = 0;
        while((posx >= pX[index]) && (index < XSize))
        {
            index++;
        }
        index -= 1;
        yOut = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index]))
               + pY[index];
    }

    return yOut;
}

int LinearInterpEnable(const float *pX, const unsigned char *pY, float posx, int XSize)
{
    int index;
    float out;
    float yOut = 0;
    if (posx >= pX[XSize - 1])
    {
        out = (float)pY[XSize - 1];
    }
    else if (posx <= pX[0])
    {
        out = pY[0];
    }
    else
    {
        index = 0;
        while((posx >= pX[index]) && (index < XSize))
        {
            index++;
        }
        index -= 1;
        out = ((pY[index + 1] - pY[index]) / (pX[index + 1] - pX[index]) * (posx - pX[index]))
              + pY[index];
    }
    yOut = out > 0.5 ? 1 : 0;

    return yOut;
}

void EnableSetting(CalibDbV2_dehaze_V20_t* pAdehazeCtx, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    ProcRes->ProcResV20.enable = true;

    bool dehaze_enable = false;
    bool enhance_enable = false;

    if(pAdehazeCtx->DehazeTuningPara.Enable) {
        if(pAdehazeCtx->DehazeTuningPara.dehaze_setting.en && pAdehazeCtx->DehazeTuningPara.enhance_setting.en)
        {
            ProcRes->ProcResV20.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV20.enhance_en = FUNCTION_ENABLE;
        }
        else if(pAdehazeCtx->DehazeTuningPara.dehaze_setting.en && !pAdehazeCtx->DehazeTuningPara.enhance_setting.en)
        {
            ProcRes->ProcResV20.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV20.enhance_en = FUNCTION_DISABLE;
        }
        else if(!pAdehazeCtx->DehazeTuningPara.dehaze_setting.en && pAdehazeCtx->DehazeTuningPara.enhance_setting.en)
        {
            ProcRes->ProcResV20.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV20.enhance_en = FUNCTION_ENABLE;
        }
        else
        {
            ProcRes->ProcResV20.dc_en = FUNCTION_DISABLE;
            ProcRes->ProcResV20.enhance_en = FUNCTION_DISABLE;
        }

        if(pAdehazeCtx->DehazeTuningPara.hist_setting.en)
            ProcRes->ProcResV20.hist_en = FUNCTION_ENABLE;
        else
            ProcRes->ProcResV20.hist_en = FUNCTION_DISABLE;
    }
    else {
        ProcRes->ProcResV20.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV20.enhance_en = FUNCTION_DISABLE;
        ProcRes->ProcResV20.hist_en = FUNCTION_DISABLE;
    }

    dehaze_enable = (ProcRes->ProcResV20.dc_en & FUNCTION_ENABLE) && (!(ProcRes->ProcResV20.enhance_en & FUNCTION_ENABLE));
    enhance_enable = (ProcRes->ProcResV20.dc_en & FUNCTION_ENABLE) && (ProcRes->ProcResV20.enhance_en & FUNCTION_ENABLE);

    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__, ProcRes->ProcResV20.enable,
                 dehaze_enable, enhance_enable, ProcRes->ProcResV20.hist_en);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void EnableSettingV21(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    ProcRes->ProcResV21.enable = pCalibV21->Enable;

    bool dehaze_enable = false;
    bool enhance_enable = false;
    if(pCalibV21->Enable) {
        if(pCalibV21->dehaze_setting.en && pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_ENABLE;
        }
        else if(pCalibV21->dehaze_setting.en && !pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        }
        else if(!pCalibV21->dehaze_setting.en && pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_ENABLE;
        }
        else
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_DISABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        }

        if(pCalibV21->hist_setting.en)
            ProcRes->ProcResV21.hist_en = FUNCTION_ENABLE;
        else
            ProcRes->ProcResV21.hist_en = FUNCTION_DISABLE;
    }
    else {
        ProcRes->ProcResV21.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        ProcRes->ProcResV21.hist_en = FUNCTION_DISABLE;
    }
    dehaze_enable = (ProcRes->ProcResV21.dc_en & FUNCTION_ENABLE) && (!(ProcRes->ProcResV21.enhance_en & FUNCTION_ENABLE));
    enhance_enable = (ProcRes->ProcResV21.dc_en & FUNCTION_ENABLE) && (ProcRes->ProcResV21.enhance_en & FUNCTION_ENABLE);

    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 ProcRes->ProcResV21.enable, dehaze_enable, enhance_enable, ProcRes->ProcResV21.hist_en);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void EnableSettingV30(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    ProcRes->ProcResV30.enable = pCalibV21->Enable;

    bool dehaze_enable = false;
    bool enhance_enable = false;
    if(pCalibV21->Enable) {
        if(pCalibV21->dehaze_setting.en && pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_ENABLE;
        }
        else if(pCalibV21->dehaze_setting.en && !pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        }
        else if(!pCalibV21->dehaze_setting.en && pCalibV21->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_ENABLE;
        }
        else
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_DISABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        }

        if(pCalibV21->hist_setting.en)
            ProcRes->ProcResV30.hist_en = FUNCTION_ENABLE;
        else
            ProcRes->ProcResV30.hist_en = FUNCTION_DISABLE;
    }
    else {
        ProcRes->ProcResV30.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        ProcRes->ProcResV30.hist_en = FUNCTION_DISABLE;
    }
    dehaze_enable = (ProcRes->ProcResV30.dc_en & FUNCTION_ENABLE) && (!(ProcRes->ProcResV30.enhance_en & FUNCTION_ENABLE));
    enhance_enable = (ProcRes->ProcResV30.dc_en & FUNCTION_ENABLE) && (ProcRes->ProcResV30.enhance_en & FUNCTION_ENABLE);

    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 ProcRes->ProcResV30.enable, dehaze_enable, enhance_enable, ProcRes->ProcResV30.hist_en);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuEnableSettingV21(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    ProcRes->ProcResV21.enable = pStManu->Enable;

    bool dehaze_enable = false;
    bool enhance_enable = false;
    if(pStManu->Enable) {
        if(pStManu->dehaze_setting.en && pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_ENABLE;
        }
        else if(pStManu->dehaze_setting.en && !pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        }
        else if(!pStManu->dehaze_setting.en && pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_ENABLE;
        }
        else
        {
            ProcRes->ProcResV21.dc_en = FUNCTION_DISABLE;
            ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        }

        if(pStManu->hist_setting.en)
            ProcRes->ProcResV21.hist_en = FUNCTION_ENABLE;
        else
            ProcRes->ProcResV21.hist_en = FUNCTION_DISABLE;
    }
    else {
        ProcRes->ProcResV21.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV21.enhance_en = FUNCTION_DISABLE;
        ProcRes->ProcResV21.hist_en = FUNCTION_DISABLE;
    }
    dehaze_enable = (ProcRes->ProcResV21.dc_en & FUNCTION_ENABLE) && (!(ProcRes->ProcResV21.enhance_en & FUNCTION_ENABLE));
    enhance_enable = (ProcRes->ProcResV21.dc_en & FUNCTION_ENABLE) && (ProcRes->ProcResV21.enhance_en & FUNCTION_ENABLE);

    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 ProcRes->ProcResV21.enable, dehaze_enable, enhance_enable, ProcRes->ProcResV21.hist_en);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuEnableSettingV30(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    ProcRes->ProcResV30.enable = pStManu->Enable;

    bool dehaze_enable = false;
    bool enhance_enable = false;
    if(pStManu->Enable) {
        if(pStManu->dehaze_setting.en && pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_ENABLE;
        }
        else if(pStManu->dehaze_setting.en && !pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        }
        else if(!pStManu->dehaze_setting.en && pStManu->enhance_setting.en)
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_ENABLE;
        }
        else
        {
            ProcRes->ProcResV30.dc_en = FUNCTION_DISABLE;
            ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        }

        if(pStManu->hist_setting.en)
            ProcRes->ProcResV30.hist_en = FUNCTION_ENABLE;
        else
            ProcRes->ProcResV30.hist_en = FUNCTION_DISABLE;
    }
    else {
        ProcRes->ProcResV30.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
        ProcRes->ProcResV30.hist_en = FUNCTION_DISABLE;
    }
    dehaze_enable = (ProcRes->ProcResV30.dc_en & FUNCTION_ENABLE) && (!(ProcRes->ProcResV30.enhance_en & FUNCTION_ENABLE));
    enhance_enable = (ProcRes->ProcResV30.dc_en & FUNCTION_ENABLE) && (ProcRes->ProcResV30.enhance_en & FUNCTION_ENABLE);

    LOGD_ADEHAZE(" %s: Dehaze module en:%d Dehaze en:%d, Enhance en:%d, Hist en:%d\n", __func__,
                 ProcRes->ProcResV30.enable, dehaze_enable, enhance_enable, ProcRes->ProcResV30.hist_en);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetDehazeParamsV21(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    bool air_lc_en = pStManu->dehaze_setting.air_lc_en;

    // dehaze_self_adp[7]
    float dc_min_th = pStManu->dehaze_setting.DehazeData.dc_min_th;
    float dc_max_th = pStManu->dehaze_setting.DehazeData.dc_max_th;
    float yhist_th = pStManu->dehaze_setting.DehazeData.yhist_th;
    float yblk_th = pStManu->dehaze_setting.DehazeData.yblk_th;
    float dark_th = pStManu->dehaze_setting.DehazeData.dark_th;
    float bright_min = pStManu->dehaze_setting.DehazeData.bright_min;
    float bright_max = pStManu->dehaze_setting.DehazeData.bright_max;

    // dehaze_range_adj[6]
    float wt_max = pStManu->dehaze_setting.DehazeData.wt_max;
    float air_max = pStManu->dehaze_setting.DehazeData.air_max;
    float air_min = pStManu->dehaze_setting.DehazeData.air_min;
    float tmax_base = pStManu->dehaze_setting.DehazeData.tmax_base;
    float tmax_off = pStManu->dehaze_setting.DehazeData.tmax_off;
    float tmax_max = pStManu->dehaze_setting.DehazeData.tmax_max;

    // dehaze_iir_control[5]
    float stab_fnum = pStManu->dehaze_setting.stab_fnum;
    float sigma = pStManu->dehaze_setting.sigma;
    float wt_sigma = pStManu->dehaze_setting.wt_sigma;
    float air_sigma = pStManu->dehaze_setting.air_sigma;
    float tmax_sigma = pStManu->dehaze_setting.tmax_sigma;
    float pre_wet = pStManu->dehaze_setting.pre_wet;

    float cfg_wt = pStManu->dehaze_setting.DehazeData.cfg_wt;
    float cfg_air = pStManu->dehaze_setting.DehazeData.cfg_air;
    float cfg_tmax = pStManu->dehaze_setting.DehazeData.cfg_tmax;

    float range_sigma = pStManu->dehaze_setting.DehazeData.range_sigma;
    float space_sigma_cur = pStManu->dehaze_setting.DehazeData.space_sigma_cur;
    float space_sigma_pre = pStManu->dehaze_setting.DehazeData.space_sigma_pre;

    // dehaze_bi_pAdehazeCtx[4]
    float bf_weight = pStManu->dehaze_setting.DehazeData.bf_weight;
    float dc_weitcur = pStManu->dehaze_setting.DehazeData.dc_weitcur;

    // dehaze_air_bf_h[9],dehaze_gaus_h[9]
    float gaus_h[9] = {2.0000, 4.0000, 2.0000,
                       4.0000, 8.0000, 4.0000,
                       2.0000, 4.0000, 2.0000
                      };

    int rawWidth = 1920;
    int rawHeight = 1080;
    ProcRes->ProcResV21.air_lc_en    = air_lc_en ? 1 : 0; // air_lc_en
    ProcRes->ProcResV21.dc_min_th    = int(dc_min_th); //0~255, (8bit) dc_min_th
    ProcRes->ProcResV21.dc_max_th    = int(dc_max_th);  //0~255, (8bit) dc_max_th
    ProcRes->ProcResV21.yhist_th    = int(yhist_th);  //0~255, (8bit) yhist_th
    ProcRes->ProcResV21.yblk_th    = int(yblk_th * ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16)); //default:28,(9bit) yblk_th
    ProcRes->ProcResV21.dark_th    = int(dark_th);  //0~255, (8bit) dark_th
    ProcRes->ProcResV21.bright_min   = int(bright_min);  //0~255, (8bit) bright_min
    ProcRes->ProcResV21.bright_max   = int(bright_max);  //0~255, (8bit) bright_max
    ProcRes->ProcResV21.wt_max   = int(wt_max * 256); //0~255, (8bit) wt_max
    ProcRes->ProcResV21.air_min   = int(air_min);  //0~255, (8bit) air_min
    ProcRes->ProcResV21.air_max   = int(air_max);  //0~256, (8bit) air_max
    ProcRes->ProcResV21.tmax_base   = int(tmax_base);  //0~255, (8bit) tmax_base
    ProcRes->ProcResV21.tmax_off   = int(tmax_off * 1024); //0~1024,(10bit) tmax_off
    ProcRes->ProcResV21.tmax_max   = int(tmax_max * 1024); //0~1024,(10bit) tmax_max
    ProcRes->ProcResV21.stab_fnum = int(stab_fnum);  //1~31,  (5bit) stab_fnum
    ProcRes->ProcResV21.iir_sigma = int(sigma);  //0~255, (8bit) sigma
    ProcRes->ProcResV21.iir_wt_sigma = int(wt_sigma * 8 + 0.5); //       (11bit),8bit+3bit, wt_sigma
    ProcRes->ProcResV21.iir_air_sigma = int(air_sigma);  //       (8bit) air_sigma
    ProcRes->ProcResV21.iir_tmax_sigma = int(tmax_sigma * 1024 + 0.5);  //       (11bit) tmax_sigma
    ProcRes->ProcResV21.iir_pre_wet = int(pre_wet * 15 + 0.5);  //       (4bit) iir_pre_wet
    ProcRes->ProcResV21.cfg_wt = int(cfg_wt * 256); //0~256, (9bit) cfg_wt
    ProcRes->ProcResV21.cfg_air = int(cfg_air);  //0~255, (8bit) cfg_air
    ProcRes->ProcResV21.cfg_tmax = int(cfg_tmax * 1024); //0~1024,(11bit) cfg_tmax
    ProcRes->ProcResV21.range_sima = int(range_sigma * 512); //0~512,(9bit) range_sima
    ProcRes->ProcResV21.space_sigma_cur = int(space_sigma_cur * 256); //0~256,(8bit) space_sigma_cur
    ProcRes->ProcResV21.space_sigma_pre = int(space_sigma_pre * 256); //0~256,(8bit) space_sigma_pre
    ProcRes->ProcResV21.bf_weight      = int(bf_weight * 256); //0~512, (8bit) dc_thed
    ProcRes->ProcResV21.dc_weitcur       = int(dc_weitcur * 256 + 0.5); //0~256, (9bit) dc_weitcur
    ProcRes->ProcResV21.gaus_h0    = int(gaus_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV21.gaus_h1    = int(gaus_h[1]);
    ProcRes->ProcResV21.gaus_h2    = int(gaus_h[0]);

    if(ProcRes->ProcResV21.dc_en && !(ProcRes->ProcResV21.enhance_en)) {
        if(ProcRes->ProcResV21.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:255 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__, cfg_air, cfg_tmax, cfg_wt);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n", __func__, ProcRes->ProcResV21.cfg_air, ProcRes->ProcResV21.cfg_tmax,
                         ProcRes->ProcResV21.cfg_wt);
        }
        else if(ProcRes->ProcResV21.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__, air_max, air_min, tmax_base, wt_max);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n", __func__, ProcRes->ProcResV21.air_max, ProcRes->ProcResV21.air_min,
                         ProcRes->ProcResV21.tmax_base, ProcRes->ProcResV21.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetDehazeParamsV30(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    bool air_lc_en = pStManu->dehaze_setting.air_lc_en;

    // dehaze_self_adp[7]
    float dc_min_th = pStManu->dehaze_setting.DehazeData.dc_min_th;
    float dc_max_th = pStManu->dehaze_setting.DehazeData.dc_max_th;
    float yhist_th = pStManu->dehaze_setting.DehazeData.yhist_th;
    float yblk_th = pStManu->dehaze_setting.DehazeData.yblk_th;
    float dark_th = pStManu->dehaze_setting.DehazeData.dark_th;
    float bright_min = pStManu->dehaze_setting.DehazeData.bright_min;
    float bright_max = pStManu->dehaze_setting.DehazeData.bright_max;

    // dehaze_range_adj[6]
    float wt_max = pStManu->dehaze_setting.DehazeData.wt_max;
    float air_max = pStManu->dehaze_setting.DehazeData.air_max;
    float air_min = pStManu->dehaze_setting.DehazeData.air_min;
    float tmax_base = pStManu->dehaze_setting.DehazeData.tmax_base;
    float tmax_off = pStManu->dehaze_setting.DehazeData.tmax_off;
    float tmax_max = pStManu->dehaze_setting.DehazeData.tmax_max;

    // dehaze_iir_control[5]
    float stab_fnum = pStManu->dehaze_setting.stab_fnum;
    float sigma = pStManu->dehaze_setting.sigma;
    float wt_sigma = pStManu->dehaze_setting.wt_sigma;
    float air_sigma = pStManu->dehaze_setting.air_sigma;
    float tmax_sigma = pStManu->dehaze_setting.tmax_sigma;
    float pre_wet = pStManu->dehaze_setting.pre_wet;

    float cfg_wt = pStManu->dehaze_setting.DehazeData.cfg_wt;
    float cfg_air = pStManu->dehaze_setting.DehazeData.cfg_air;
    float cfg_tmax = pStManu->dehaze_setting.DehazeData.cfg_tmax;

    float range_sigma = pStManu->dehaze_setting.DehazeData.range_sigma;
    float space_sigma_cur = pStManu->dehaze_setting.DehazeData.space_sigma_cur;
    float space_sigma_pre = pStManu->dehaze_setting.DehazeData.space_sigma_pre;

    // dehaze_bi_pAdehazeCtx[4]
    float bf_weight = pStManu->dehaze_setting.DehazeData.bf_weight;
    float dc_weitcur = pStManu->dehaze_setting.DehazeData.dc_weitcur;

    // dehaze_air_bf_h[9],dehaze_gaus_h[9]
    float gaus_h[9] = {2.0000, 4.0000, 2.0000,
                       4.0000, 8.0000, 4.0000,
                       2.0000, 4.0000, 2.0000
                      };

    int rawWidth = 1920;
    int rawHeight = 1080;
    ProcRes->ProcResV30.air_lc_en    = air_lc_en ? 1 : 0; // air_lc_en
    ProcRes->ProcResV30.dc_min_th    = int(dc_min_th); //0~255, (8bit) dc_min_th
    ProcRes->ProcResV30.dc_max_th    = int(dc_max_th);  //0~255, (8bit) dc_max_th
    ProcRes->ProcResV30.yhist_th    = int(yhist_th);  //0~255, (8bit) yhist_th
    ProcRes->ProcResV30.yblk_th    = int(yblk_th * ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16)); //default:28,(9bit) yblk_th
    ProcRes->ProcResV30.dark_th    = int(dark_th);  //0~255, (8bit) dark_th
    ProcRes->ProcResV30.bright_min   = int(bright_min);  //0~255, (8bit) bright_min
    ProcRes->ProcResV30.bright_max   = int(bright_max);  //0~255, (8bit) bright_max
    ProcRes->ProcResV30.wt_max   = int(wt_max * 256); //0~255, (8bit) wt_max
    ProcRes->ProcResV30.air_min   = int(air_min);  //0~255, (8bit) air_min
    ProcRes->ProcResV30.air_max   = int(air_max);  //0~256, (8bit) air_max
    ProcRes->ProcResV30.tmax_base   = int(tmax_base);  //0~255, (8bit) tmax_base
    ProcRes->ProcResV30.tmax_off   = int(tmax_off * 1024); //0~1024,(10bit) tmax_off
    ProcRes->ProcResV30.tmax_max   = int(tmax_max * 1024); //0~1024,(10bit) tmax_max
    ProcRes->ProcResV30.stab_fnum = int(stab_fnum);  //1~31,  (5bit) stab_fnum
    ProcRes->ProcResV30.iir_sigma = int(sigma);  //0~255, (8bit) sigma
    ProcRes->ProcResV30.iir_wt_sigma = int(wt_sigma * 8 + 0.5); //       (11bit),8bit+3bit, wt_sigma
    ProcRes->ProcResV30.iir_air_sigma = int(air_sigma);  //       (8bit) air_sigma
    ProcRes->ProcResV30.iir_tmax_sigma = int(tmax_sigma * 1024 + 0.5);  //       (11bit) tmax_sigma
    ProcRes->ProcResV30.iir_pre_wet = int(pre_wet * 15 + 0.5);  //       (4bit) iir_pre_wet
    ProcRes->ProcResV30.cfg_wt = int(cfg_wt * 256); //0~256, (9bit) cfg_wt
    ProcRes->ProcResV30.cfg_air = int(cfg_air);  //0~255, (8bit) cfg_air
    ProcRes->ProcResV30.cfg_tmax = int(cfg_tmax * 1024); //0~1024,(11bit) cfg_tmax
    ProcRes->ProcResV30.range_sima = int(range_sigma * 512); //0~512,(9bit) range_sima
    ProcRes->ProcResV30.space_sigma_cur = int(space_sigma_cur * 256); //0~256,(8bit) space_sigma_cur
    ProcRes->ProcResV30.space_sigma_pre = int(space_sigma_pre * 256); //0~256,(8bit) space_sigma_pre
    ProcRes->ProcResV30.bf_weight      = int(bf_weight * 256); //0~512, (8bit) dc_thed
    ProcRes->ProcResV30.dc_weitcur       = int(dc_weitcur * 256 + 0.5); //0~256, (9bit) dc_weitcur
    ProcRes->ProcResV30.gaus_h0    = int(gaus_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV30.gaus_h1    = int(gaus_h[1]);
    ProcRes->ProcResV30.gaus_h2    = int(gaus_h[0]);

    if(ProcRes->ProcResV30.dc_en && !(ProcRes->ProcResV30.enhance_en)) {
        if(ProcRes->ProcResV30.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:255 cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__, cfg_air, cfg_tmax, cfg_wt);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n", __func__, ProcRes->ProcResV30.cfg_air, ProcRes->ProcResV30.cfg_tmax,
                         ProcRes->ProcResV30.cfg_wt);
        }
        else if(ProcRes->ProcResV30.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__, air_max, air_min, tmax_base, wt_max);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n", __func__, ProcRes->ProcResV30.air_max, ProcRes->ProcResV30.air_min,
                         ProcRes->ProcResV30.tmax_base, ProcRes->ProcResV30.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV21(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    float enhance_value = pStManu->enhance_setting.EnhanceData.enhance_value;
    float enhance_chroma = pStManu->enhance_setting.EnhanceData.enhance_chroma;

    ProcRes->ProcResV21.enhance_value = int(enhance_value * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value
    ProcRes->ProcResV21.enhance_chroma = int(enhance_chroma * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value

    for(int i = 0; i < ISP21_DHAZ_ENH_CURVE_NUM; i++)
        ProcRes->ProcResV21.enh_curve[i] = (int)(pStManu->enhance_setting.enhance_curve[i]);

    if(ProcRes->ProcResV21.dc_en && ProcRes->ProcResV21.enhance_en) {
        LOGD_ADEHAZE("%s enhance_value:%f enhance_chroma:%f\n", __func__,
                     enhance_value, enhance_chroma);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     ProcRes->ProcResV21.enhance_value, ProcRes->ProcResV21.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetEnhanceParamsV30(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    float enhance_value = pStManu->enhance_setting.EnhanceData.enhance_value;
    float enhance_chroma = pStManu->enhance_setting.EnhanceData.enhance_chroma;

    ProcRes->ProcResV30.enhance_value = int(enhance_value * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value
    ProcRes->ProcResV30.enhance_chroma = int(enhance_chroma * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value

    for(int i = 0; i < ISP21_DHAZ_ENH_CURVE_NUM; i++)
        ProcRes->ProcResV30.enh_curve[i] = (int)(pStManu->enhance_setting.enhance_curve[i]);

    if(ProcRes->ProcResV30.dc_en && ProcRes->ProcResV30.enhance_en) {
        LOGD_ADEHAZE("%s enhance_value:%f enhance_chroma:%f\n", __func__,
                     enhance_value, enhance_chroma);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     ProcRes->ProcResV30.enhance_value, ProcRes->ProcResV30.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV21(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    bool hist_para_en = pStManu->hist_setting.hist_para_en;
    float hist_gratio = pStManu->hist_setting.HistData.hist_gratio;
    float hist_th_off = pStManu->hist_setting.HistData.hist_th_off;
    float hist_k = pStManu->hist_setting.HistData.hist_k;
    float hist_min = pStManu->hist_setting.HistData.hist_min;
    float hist_scale = pStManu->hist_setting.HistData.hist_scale;
    float cfg_gratio = pStManu->hist_setting.HistData.cfg_gratio;

    ProcRes->ProcResV21.hpara_en = hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE; //  hist_para_en
    //clip hpara_en
    ProcRes->ProcResV21.hpara_en = ProcRes->ProcResV21.dc_en ? ProcRes->ProcResV21.hpara_en : FUNCTION_ENABLE; //  dc en 关闭，hpara必需开

    ProcRes->ProcResV21.hist_gratio   = int(hist_gratio * 8); //       (8bit) hist_gratio
    ProcRes->ProcResV21.hist_th_off   = int(hist_th_off);  //       (8bit) hist_th_off
    ProcRes->ProcResV21.hist_k   = int(hist_k * 4 + 0.5); //0~7    (5bit),3bit+2bit, hist_k
    ProcRes->ProcResV21.hist_min   = int(hist_min * 256); //       (9bit) hist_min
    ProcRes->ProcResV21.cfg_gratio = int(cfg_gratio * 256); //       (13bit),5bit+8bit, cfg_gratio
    ProcRes->ProcResV21.hist_scale       = int(hist_scale *  256 + 0.5 );  //       (13bit),5bit + 8bit, sw_hist_scale

    if(ProcRes->ProcResV21.hist_en) {
        LOGD_ADEHAZE("%s cfg_alpha:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f hist_min:%f hist_scale:%f cfg_gratio:%f\n", __func__,
                     ProcRes->ProcResV21.cfg_alpha / 255.0, ProcRes->ProcResV21.hpara_en, hist_gratio, hist_th_off, hist_k, hist_min, hist_scale, cfg_gratio);
        LOGD_ADEHAZE("%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n", __func__,
                     ProcRes->ProcResV21.cfg_alpha, ProcRes->ProcResV21.hist_gratio, ProcRes->ProcResV21.hist_th_off, ProcRes->ProcResV21.hist_k,
                     ProcRes->ProcResV21.hist_min, ProcRes->ProcResV21.hist_scale, ProcRes->ProcResV21.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void stManuGetHistParamsV30(mDehazeAttr_t* pStManu, RkAiqAdehazeProcResult_t* ProcRes)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    bool hist_para_en = pStManu->hist_setting.hist_para_en;
    float hist_gratio = pStManu->hist_setting.HistData.hist_gratio;
    float hist_th_off = pStManu->hist_setting.HistData.hist_th_off;
    float hist_k = pStManu->hist_setting.HistData.hist_k;
    float hist_min = pStManu->hist_setting.HistData.hist_min;
    float hist_scale = pStManu->hist_setting.HistData.hist_scale;
    float cfg_gratio = pStManu->hist_setting.HistData.cfg_gratio;

    ProcRes->ProcResV30.hpara_en = hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE; //  hist_para_en
    //clip hpara_en
    ProcRes->ProcResV30.hpara_en = ProcRes->ProcResV30.dc_en ? ProcRes->ProcResV30.hpara_en : FUNCTION_ENABLE; //  dc en 关闭，hpara必需开

    ProcRes->ProcResV30.hist_gratio   = int(hist_gratio * 8); //       (8bit) hist_gratio
    ProcRes->ProcResV30.hist_th_off   = int(hist_th_off);  //       (8bit) hist_th_off
    ProcRes->ProcResV30.hist_k   = int(hist_k * 4 + 0.5); //0~7    (5bit),3bit+2bit, hist_k
    ProcRes->ProcResV30.hist_min   = int(hist_min * 256); //       (9bit) hist_min
    ProcRes->ProcResV30.cfg_gratio = int(cfg_gratio * 256); //       (13bit),5bit+8bit, cfg_gratio
    ProcRes->ProcResV30.hist_scale       = int(hist_scale *  256 + 0.5 );  //       (13bit),5bit + 8bit, sw_hist_scale

    if(ProcRes->ProcResV30.hist_en) {
        LOGD_ADEHAZE("%s cfg_alpha:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f hist_min:%f hist_scale:%f cfg_gratio:%f\n", __func__,
                     ProcRes->ProcResV30.cfg_alpha / 255.0, ProcRes->ProcResV30.hpara_en, hist_gratio, hist_th_off, hist_k, hist_min, hist_scale, cfg_gratio);
        LOGD_ADEHAZE("%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n", __func__,
                     ProcRes->ProcResV30.cfg_alpha, ProcRes->ProcResV30.hist_gratio, ProcRes->ProcResV30.hist_th_off, ProcRes->ProcResV30.hist_k,
                     ProcRes->ProcResV30.hist_min, ProcRes->ProcResV30.hist_scale, ProcRes->ProcResV30.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParams(CalibDbV2_dehaze_V20_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int iso_len = pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
    // dehaze_self_adp[7]
    float dc_min_th = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th, CtrlValue, iso_len);
    float dc_max_th = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th, CtrlValue, iso_len);
    float yhist_th = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th, CtrlValue,  iso_len);
    float yblk_th = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th, CtrlValue, iso_len);
    float dark_th = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.dark_th, CtrlValue, iso_len);
    float bright_min = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.bright_min, CtrlValue, iso_len);
    float bright_max = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.bright_max, CtrlValue, iso_len);

    // dehaze_range_adj[6]
    float wt_max = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.wt_max, CtrlValue, iso_len);
    float air_max = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.air_max, CtrlValue, iso_len);
    float air_min = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.air_min, CtrlValue, iso_len);
    float tmax_base = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base, CtrlValue, iso_len);
    float tmax_off = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off, CtrlValue, iso_len);
    float tmax_max = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max, CtrlValue, iso_len);

    // dehaze_iir_control[5]
    float stab_fnum = pCalib->DehazeTuningPara.dehaze_setting.stab_fnum;
    float sigma = pCalib->DehazeTuningPara.dehaze_setting.sigma;
    float wt_sigma = pCalib->DehazeTuningPara.dehaze_setting.wt_sigma;
    float air_sigma = pCalib->DehazeTuningPara.dehaze_setting.air_sigma;
    float tmax_sigma = pCalib->DehazeTuningPara.dehaze_setting.tmax_sigma;

    float cfg_wt = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt, CtrlValue, iso_len);
    float cfg_air = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air, CtrlValue, iso_len);
    float cfg_tmax = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax, CtrlValue, iso_len);

    // dehaze_bi_pAdehazeCtx[4]
    float dc_thed = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed, CtrlValue, iso_len);
    float dc_weitcur = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur, CtrlValue, iso_len);
    float air_thed = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.air_thed, CtrlValue, iso_len);
    float air_weitcur = LinearInterp(pCalib->DehazeTuningPara.dehaze_setting.DehazeData.ISO, pCalib->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur, CtrlValue, iso_len);

    // dehaze_dc_bf_h[25]
    float dc_bf_h[25] = {12.0000, 17.0000, 19.0000, 17.0000, 12.0000,
                         17.0000, 25.0000, 28.0000, 25.0000, 17.0000,
                         19.0000, 28.0000, 32.0000, 28.0000, 19.0000,
                         17.0000, 25.0000, 28.0000, 25.0000, 17.0000,
                         12.0000, 17.0000, 19.0000, 17.0000, 12.0000
                        };

    // dehaze_air_bf_h[9],dehaze_gaus_h[9]
    float air_bf_h[9] = {25.0000, 28.0000, 25.0000,
                         28.0000, 32.0000, 28.0000,
                         25.0000, 28.0000, 25.0000
                        };
    float gaus_h[9] = {2.0000, 4.0000, 2.0000,
                       4.0000, 8.0000, 4.0000,
                       2.0000, 4.0000, 2.0000
                      };

    LOGD_ADEHAZE("%s dc_min_th:%f dc_max_th:%f yhist_th:%f yblk_th:%f dark_th:%f bright_min:%f bright_max:%f\n", __func__, dc_min_th, dc_max_th, yhist_th, yblk_th, dark_th, bright_min, bright_max);
    LOGD_ADEHAZE("%s wt_max:%f air_max:%f air_min:%f tmax_base:%f tmax_off:%f tmax_max:%f\n", __func__, wt_max, air_max, air_min, tmax_base, tmax_off, tmax_max);
    LOGD_ADEHAZE("%s stab_fnum:%f sigma:%f wt_sigma:%f air_sigma:%f tmax_sigma:%f\n", __func__, stab_fnum, sigma, wt_sigma, air_sigma, tmax_sigma);
    LOGD_ADEHAZE("%s  cfg_wt:%f cfg_air:%f cfg_tmax:%f\n", __func__, cfg_wt, cfg_air, cfg_tmax);
    LOGD_ADEHAZE("%s dc_thed:%f dc_weitcur:%f air_thed:%f air_weitcur:%f\n", __func__, dc_thed, dc_weitcur, air_thed, air_weitcur);

    int rawWidth = 1920;
    int rawHeight = 1080;
    ProcRes->ProcResV20.dc_min_th    = int(dc_min_th); //0~255, (8bit) dc_min_th
    ProcRes->ProcResV20.dc_max_th    = int(dc_max_th);  //0~255, (8bit) dc_max_th
    ProcRes->ProcResV20.yhist_th    = int(yhist_th);  //0~255, (8bit) yhist_th
    ProcRes->ProcResV20.yblk_th    = int(yblk_th * ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16)); //default:28,(9bit) yblk_th
    ProcRes->ProcResV20.dark_th    = int(dark_th);  //0~255, (8bit) dark_th
    ProcRes->ProcResV20.bright_min   = int(bright_min);  //0~255, (8bit) bright_min
    ProcRes->ProcResV20.bright_max   = int(bright_max);  //0~255, (8bit) bright_max


    ProcRes->ProcResV20.wt_max   = int(wt_max * 256); //0~255, (9bit) wt_max
    ProcRes->ProcResV20.air_min   = int(air_min);  //0~255, (8bit) air_min
    ProcRes->ProcResV20.air_max   = int(air_max);  //0~256, (8bit) air_max
    ProcRes->ProcResV20.tmax_base   = int(tmax_base);  //0~255, (8bit) tmax_base
    ProcRes->ProcResV20.tmax_off   = int(tmax_off * 1024); //0~1024,(10bit) tmax_off
    ProcRes->ProcResV20.tmax_max   = int(tmax_max * 1024); //0~1024,(10bit) tmax_max

    ProcRes->ProcResV20.stab_fnum = int(stab_fnum);  //1~31,  (5bit) stab_fnum
    ProcRes->ProcResV20.iir_sigma = int(sigma);  //0~255, (8bit) sigma
    ProcRes->ProcResV20.iir_wt_sigma = int(wt_sigma * 8 + 0.5); //       (11bit),8bit+3bit, wt_sigma
    ProcRes->ProcResV20.iir_air_sigma = int(air_sigma);  //       (8bit) air_sigma
    ProcRes->ProcResV20.iir_tmax_sigma = int(tmax_sigma * 1024 + 0.5);  //       (11bit) tmax_sigma

    ProcRes->ProcResV20.cfg_wt = int(cfg_wt * 256); //0~256, (9bit) cfg_wt
    ProcRes->ProcResV20.cfg_air = int(cfg_air);  //0~255, (8bit) cfg_air
    ProcRes->ProcResV20.cfg_tmax = int(cfg_tmax * 1024); //0~1024,(11bit) cfg_tmax

    ProcRes->ProcResV20.dc_thed      = int(dc_thed);  //0~255, (8bit) dc_thed
    ProcRes->ProcResV20.dc_weitcur       = int(dc_weitcur * 256 + 0.5); //0~256, (9bit) dc_weitcur
    ProcRes->ProcResV20.air_thed     = int(air_thed);  //0~255, (8bit) air_thed
    ProcRes->ProcResV20.air_weitcur      = int(air_weitcur * 256 + 0.5);  //0~256, (9bit) air_weitcur

    ProcRes->ProcResV20.gaus_h0    = int(gaus_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV20.gaus_h1    = int(gaus_h[1]);
    ProcRes->ProcResV20.gaus_h2    = int(gaus_h[0]);
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h0   = int(dc_bf_h[12]);//h0~h5  浠庡ぇ鍒板皬
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h1   = int(dc_bf_h[7]);
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h2   = int(dc_bf_h[6]);
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h3   = int(dc_bf_h[2]);
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h4   = int(dc_bf_h[1]);
    ProcRes->ProcResV20.sw_dhaz_dc_bf_h5   = int(dc_bf_h[0]);
    ProcRes->ProcResV20.air_bf_h0  = int(air_bf_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV20.air_bf_h1  = int(air_bf_h[1]);
    ProcRes->ProcResV20.air_bf_h2  = int(air_bf_h[0]);

    if(ProcRes->ProcResV20.dc_en && !(ProcRes->ProcResV20.enhance_en)) {
        LOGD_ADEHAZE("%s dc_min_th:%d dc_max_th:%d yhist_th:%d yblk_th:%d dark_th:%d bright_min:%d bright_max:%d\n", __func__, ProcRes->ProcResV20.dc_min_th,
                     ProcRes->ProcResV20.dc_max_th, ProcRes->ProcResV20.yhist_th, ProcRes->ProcResV20.yblk_th, ProcRes->ProcResV20.dark_th,
                     ProcRes->ProcResV20.bright_min, ProcRes->ProcResV20.bright_max);
        LOGD_ADEHAZE("%s wt_max:%d air_max:%d air_min:%d tmax_base:%d tmax_off:%d tmax_max:%d\n", __func__, ProcRes->ProcResV20.wt_max,
                     ProcRes->ProcResV20.air_max, ProcRes->ProcResV20.air_min, ProcRes->ProcResV20.tmax_base,
                     ProcRes->ProcResV20.tmax_off, ProcRes->ProcResV20.tmax_max);
        LOGD_ADEHAZE("%s stab_fnum:%d sigma:%d wt_sigma:%d air_sigma:%d tmax_sigma:%d\n", __func__, ProcRes->ProcResV20.stab_fnum,
                     ProcRes->ProcResV20.iir_sigma, ProcRes->ProcResV20.iir_wt_sigma, ProcRes->ProcResV20.iir_air_sigma,
                     ProcRes->ProcResV20.iir_tmax_sigma);
        LOGD_ADEHAZE("%s  cfg_wt:%d cfg_air:%d cfg_tmax:%d\n", __func__, ProcRes->ProcResV20.cfg_wt, ProcRes->ProcResV20.cfg_air, ProcRes->ProcResV20.cfg_tmax);
        LOGD_ADEHAZE("%s dc_thed:%d dc_weitcur:%d air_thed:%d air_weitcur:%d\n", __func__, ProcRes->ProcResV20.dc_thed,
                     ProcRes->ProcResV20.dc_weitcur, ProcRes->ProcResV20.air_thed, ProcRes->ProcResV20.air_weitcur);
    }


    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV21(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->dehaze_setting.DehazeData.EnvLv_len;
    bool air_lc_en = pCalibV21->dehaze_setting.air_lc_en;

    // dehaze_self_adp[7]
    float dc_min_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_min_th, CtrlValue, EnvLv_len);
    float dc_max_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_max_th, CtrlValue, EnvLv_len);
    float yhist_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.yhist_th, CtrlValue,  EnvLv_len);
    float yblk_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.yblk_th, CtrlValue, EnvLv_len);
    float dark_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dark_th, CtrlValue, EnvLv_len);
    float bright_min = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bright_min, CtrlValue, EnvLv_len);
    float bright_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bright_max, CtrlValue, EnvLv_len);

    // dehaze_range_adj[6]
    float wt_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.wt_max, CtrlValue, EnvLv_len);
    float air_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.air_max, CtrlValue, EnvLv_len);
    float air_min = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.air_min, CtrlValue, EnvLv_len);
    float tmax_base = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_base, CtrlValue, EnvLv_len);
    float tmax_off = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_off, CtrlValue, EnvLv_len);
    float tmax_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_max, CtrlValue, EnvLv_len);

    // dehaze_iir_control[5]
    float stab_fnum = pCalibV21->dehaze_setting.stab_fnum;
    float sigma = pCalibV21->dehaze_setting.sigma;
    float wt_sigma = pCalibV21->dehaze_setting.wt_sigma;
    float air_sigma = pCalibV21->dehaze_setting.air_sigma;
    float tmax_sigma = pCalibV21->dehaze_setting.tmax_sigma;
    float pre_wet = pCalibV21->dehaze_setting.pre_wet;

    float cfg_wt = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_wt, CtrlValue, EnvLv_len);
    float cfg_air = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_air, CtrlValue, EnvLv_len);
    float cfg_tmax = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_tmax, CtrlValue, EnvLv_len);

    float range_sigma = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.range_sigma, CtrlValue, EnvLv_len);
    float space_sigma_cur = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.space_sigma_cur, CtrlValue, EnvLv_len);
    float space_sigma_pre = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.space_sigma_pre, CtrlValue, EnvLv_len);

    // dehaze_bi_pAdehazeCtx[4]
    float bf_weight = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bf_weight, CtrlValue, EnvLv_len);
    float dc_weitcur = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_weitcur, CtrlValue, EnvLv_len);

    // dehaze_air_bf_h[9],dehaze_gaus_h[9]
    float gaus_h[9] = {2.0000, 4.0000, 2.0000,
                       4.0000, 8.0000, 4.0000,
                       2.0000, 4.0000, 2.0000
                      };

    int rawWidth = 1920;
    int rawHeight = 1080;
    ProcRes->ProcResV21.air_lc_en    = air_lc_en ? 1 : 0; // air_lc_en
    ProcRes->ProcResV21.dc_min_th    = int(dc_min_th); //0~255, (8bit) dc_min_th
    ProcRes->ProcResV21.dc_max_th    = int(dc_max_th);  //0~255, (8bit) dc_max_th
    ProcRes->ProcResV21.yhist_th    = int(yhist_th);  //0~255, (8bit) yhist_th
    ProcRes->ProcResV21.yblk_th    = int(yblk_th * ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16)); //default:28,(9bit) yblk_th
    ProcRes->ProcResV21.dark_th    = int(dark_th);  //0~255, (8bit) dark_th
    ProcRes->ProcResV21.bright_min   = int(bright_min);  //0~255, (8bit) bright_min
    ProcRes->ProcResV21.bright_max   = int(bright_max);  //0~255, (8bit) bright_max
    ProcRes->ProcResV21.wt_max   = int(wt_max * 256); //0~255, (8bit) wt_max
    ProcRes->ProcResV21.air_min   = int(air_min);  //0~255, (8bit) air_min
    ProcRes->ProcResV21.air_max   = int(air_max);  //0~256, (8bit) air_max
    ProcRes->ProcResV21.tmax_base   = int(tmax_base);  //0~255, (8bit) tmax_base
    ProcRes->ProcResV21.tmax_off   = int(tmax_off * 1024); //0~1024,(10bit) tmax_off
    ProcRes->ProcResV21.tmax_max   = int(tmax_max * 1024); //0~1024,(10bit) tmax_max
    ProcRes->ProcResV21.stab_fnum = int(stab_fnum);  //1~31,  (5bit) stab_fnum
    ProcRes->ProcResV21.iir_sigma = int(sigma);  //0~255, (8bit) sigma
    ProcRes->ProcResV21.iir_wt_sigma = int(wt_sigma * 8 + 0.5); //       (11bit),8bit+3bit, wt_sigma
    ProcRes->ProcResV21.iir_air_sigma = int(air_sigma);  //       (8bit) air_sigma
    ProcRes->ProcResV21.iir_tmax_sigma = int(tmax_sigma * 1024 + 0.5);  //       (11bit) tmax_sigma
    ProcRes->ProcResV21.iir_pre_wet = int(pre_wet * 15 + 0.5);  //       (4bit) iir_pre_wet
    ProcRes->ProcResV21.cfg_wt = int(cfg_wt * 256); //0~256, (9bit) cfg_wt
    ProcRes->ProcResV21.cfg_air = int(cfg_air);  //0~255, (8bit) cfg_air
    ProcRes->ProcResV21.cfg_tmax = int(cfg_tmax * 1024); //0~1024,(11bit) cfg_tmax
    ProcRes->ProcResV21.range_sima = int(range_sigma * 512); //0~512,(9bit) range_sima
    ProcRes->ProcResV21.space_sigma_cur = int(space_sigma_cur * 256); //0~256,(8bit) space_sigma_cur
    ProcRes->ProcResV21.space_sigma_pre = int(space_sigma_pre * 256); //0~256,(8bit) space_sigma_pre
    ProcRes->ProcResV21.bf_weight      = int(bf_weight * 256); //0~512, (8bit) dc_thed
    ProcRes->ProcResV21.dc_weitcur       = int(dc_weitcur * 256 + 0.5); //0~256, (9bit) dc_weitcur
    ProcRes->ProcResV21.gaus_h0    = int(gaus_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV21.gaus_h1    = int(gaus_h[1]);
    ProcRes->ProcResV21.gaus_h2    = int(gaus_h[0]);

    if(ProcRes->ProcResV21.dc_en && !(ProcRes->ProcResV21.enhance_en)) {
        if(ProcRes->ProcResV21.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:255 EnvLv:%f cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__, CtrlValue, cfg_air, cfg_tmax, cfg_wt);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n", __func__, ProcRes->ProcResV21.cfg_air, ProcRes->ProcResV21.cfg_tmax,
                         ProcRes->ProcResV21.cfg_wt);
        }
        else if(ProcRes->ProcResV21.cfg_alpha == 0) {
            LOGD_ADEHAZE("%s cfg_alpha:0 EnvLv:%f air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__, CtrlValue, air_max, air_min, tmax_base, wt_max);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n", __func__, ProcRes->ProcResV21.air_max, ProcRes->ProcResV21.air_min,
                         ProcRes->ProcResV21.tmax_base, ProcRes->ProcResV21.wt_max);
        }
    }


    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeParamsV30(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->dehaze_setting.DehazeData.EnvLv_len;
    bool air_lc_en = pCalibV21->dehaze_setting.air_lc_en;

    // dehaze_self_adp[7]
    float dc_min_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_min_th, CtrlValue, EnvLv_len);
    float dc_max_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_max_th, CtrlValue, EnvLv_len);
    float yhist_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.yhist_th, CtrlValue,  EnvLv_len);
    float yblk_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.yblk_th, CtrlValue, EnvLv_len);
    float dark_th = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dark_th, CtrlValue, EnvLv_len);
    float bright_min = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bright_min, CtrlValue, EnvLv_len);
    float bright_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bright_max, CtrlValue, EnvLv_len);

    // dehaze_range_adj[6]
    float wt_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.wt_max, CtrlValue, EnvLv_len);
    float air_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.air_max, CtrlValue, EnvLv_len);
    float air_min = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.air_min, CtrlValue, EnvLv_len);
    float tmax_base = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_base, CtrlValue, EnvLv_len);
    float tmax_off = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_off, CtrlValue, EnvLv_len);
    float tmax_max = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.tmax_max, CtrlValue, EnvLv_len);

    // dehaze_iir_control[5]
    float stab_fnum = pCalibV21->dehaze_setting.stab_fnum;
    float sigma = pCalibV21->dehaze_setting.sigma;
    float wt_sigma = pCalibV21->dehaze_setting.wt_sigma;
    float air_sigma = pCalibV21->dehaze_setting.air_sigma;
    float tmax_sigma = pCalibV21->dehaze_setting.tmax_sigma;
    float pre_wet = pCalibV21->dehaze_setting.pre_wet;

    float cfg_wt = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_wt, CtrlValue, EnvLv_len);
    float cfg_air = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_air, CtrlValue, EnvLv_len);
    float cfg_tmax = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.cfg_tmax, CtrlValue, EnvLv_len);

    float range_sigma = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.range_sigma, CtrlValue, EnvLv_len);
    float space_sigma_cur = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.space_sigma_cur, CtrlValue, EnvLv_len);
    float space_sigma_pre = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.space_sigma_pre, CtrlValue, EnvLv_len);

    // dehaze_bi_pAdehazeCtx[4]
    float bf_weight = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.bf_weight, CtrlValue, EnvLv_len);
    float dc_weitcur = LinearInterp(pCalibV21->dehaze_setting.DehazeData.EnvLv, pCalibV21->dehaze_setting.DehazeData.dc_weitcur, CtrlValue, EnvLv_len);

    // dehaze_air_bf_h[9],dehaze_gaus_h[9]
    float gaus_h[9] = {2.0000, 4.0000, 2.0000,
                       4.0000, 8.0000, 4.0000,
                       2.0000, 4.0000, 2.0000
                      };

    int rawWidth = 1920;
    int rawHeight = 1080;
    ProcRes->ProcResV30.air_lc_en    = air_lc_en ? 1 : 0; // air_lc_en
    ProcRes->ProcResV30.dc_min_th    = int(dc_min_th); //0~255, (8bit) dc_min_th
    ProcRes->ProcResV30.dc_max_th    = int(dc_max_th);  //0~255, (8bit) dc_max_th
    ProcRes->ProcResV30.yhist_th    = int(yhist_th);  //0~255, (8bit) yhist_th
    ProcRes->ProcResV30.yblk_th    = int(yblk_th * ((rawWidth + 15) / 16) * ((rawHeight + 15) / 16)); //default:28,(9bit) yblk_th
    ProcRes->ProcResV30.dark_th    = int(dark_th);  //0~255, (8bit) dark_th
    ProcRes->ProcResV30.bright_min   = int(bright_min);  //0~255, (8bit) bright_min
    ProcRes->ProcResV30.bright_max   = int(bright_max);  //0~255, (8bit) bright_max
    ProcRes->ProcResV30.wt_max   = int(wt_max * 256); //0~255, (8bit) wt_max
    ProcRes->ProcResV30.air_min   = int(air_min);  //0~255, (8bit) air_min
    ProcRes->ProcResV30.air_max   = int(air_max);  //0~256, (8bit) air_max
    ProcRes->ProcResV30.tmax_base   = int(tmax_base);  //0~255, (8bit) tmax_base
    ProcRes->ProcResV30.tmax_off   = int(tmax_off * 1024); //0~1024,(10bit) tmax_off
    ProcRes->ProcResV30.tmax_max   = int(tmax_max * 1024); //0~1024,(10bit) tmax_max
    ProcRes->ProcResV30.stab_fnum = int(stab_fnum);  //1~31,  (5bit) stab_fnum
    ProcRes->ProcResV30.iir_sigma = int(sigma);  //0~255, (8bit) sigma
    ProcRes->ProcResV30.iir_wt_sigma = int(wt_sigma * 8 + 0.5); //       (11bit),8bit+3bit, wt_sigma
    ProcRes->ProcResV30.iir_air_sigma = int(air_sigma);  //       (8bit) air_sigma
    ProcRes->ProcResV30.iir_tmax_sigma = int(tmax_sigma * 1024 + 0.5);  //       (11bit) tmax_sigma
    ProcRes->ProcResV30.iir_pre_wet = int(pre_wet * 15 + 0.5);  //       (4bit) iir_pre_wet
    ProcRes->ProcResV30.cfg_wt = int(cfg_wt * 256); //0~256, (9bit) cfg_wt
    ProcRes->ProcResV30.cfg_air = int(cfg_air);  //0~255, (8bit) cfg_air
    ProcRes->ProcResV30.cfg_tmax = int(cfg_tmax * 1024); //0~1024,(11bit) cfg_tmax
    ProcRes->ProcResV30.range_sima = int(range_sigma * 512); //0~512,(9bit) range_sima
    ProcRes->ProcResV30.space_sigma_cur = int(space_sigma_cur * 256); //0~256,(8bit) space_sigma_cur
    ProcRes->ProcResV30.space_sigma_pre = int(space_sigma_pre * 256); //0~256,(8bit) space_sigma_pre
    ProcRes->ProcResV30.bf_weight      = int(bf_weight * 256); //0~512, (8bit) dc_thed
    ProcRes->ProcResV30.dc_weitcur       = int(dc_weitcur * 256 + 0.5); //0~256, (9bit) dc_weitcur
    ProcRes->ProcResV30.gaus_h0    = int(gaus_h[4]);//h0~h2  浠庡ぇ鍒板皬
    ProcRes->ProcResV30.gaus_h1    = int(gaus_h[1]);
    ProcRes->ProcResV30.gaus_h2    = int(gaus_h[0]);

    if(ProcRes->ProcResV30.dc_en && !(ProcRes->ProcResV30.enhance_en)) {
        if(ProcRes->ProcResV30.cfg_alpha == 255) {
            LOGD_ADEHAZE("%s cfg_alpha:1 EnvLv:%f cfg_air:%f cfg_tmax:%f cfg_wt:%f\n", __func__, CtrlValue, cfg_air, cfg_tmax, cfg_wt);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x255 cfg_air:0x%x cfg_tmax:0x%x cfg_wt:0x%x\n", __func__,
                         ProcRes->ProcResV30.cfg_air, ProcRes->ProcResV30.cfg_tmax, ProcRes->ProcResV30.cfg_wt);
        }
        else if(ProcRes->ProcResV30.cfg_alpha == 0) {

            LOGD_ADEHAZE("%s cfg_alpha:0 EnvLv:%f air_max:%f air_min:%f tmax_base:%f wt_max:%f\n", __func__, CtrlValue, air_max, air_min, tmax_base, wt_max);
            LOGD_ADEHAZE("%s cfg_alpha_reg:0x0 air_max:0x%x air_min:0x%x tmax_base:0x%x wt_max:0x%x\n", __func__, ProcRes->ProcResV30.air_max, ProcRes->ProcResV30.air_min,
                         ProcRes->ProcResV30.tmax_base, ProcRes->ProcResV30.wt_max);
        }
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParams(CalibDbV2_dehaze_V20_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int iso_len = pCalib->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len;
    float enhance_value = LinearInterp(pCalib->DehazeTuningPara.enhance_setting.EnhanceData.ISO, pCalib->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value, CtrlValue, iso_len);
    ProcRes->ProcResV20.enhance_value = int(enhance_value * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value

    if(ProcRes->ProcResV20.dc_en && ProcRes->ProcResV20.enhance_en)
        LOGD_ADEHAZE("%s enhance_value:%f enhance_value:0x%x\n", __func__, enhance_value, ProcRes->ProcResV20.enhance_value);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV21(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->enhance_setting.EnhanceData.EnvLv_len;
    float enhance_value = LinearInterp(pCalibV21->enhance_setting.EnhanceData.EnvLv, pCalibV21->enhance_setting.EnhanceData.enhance_value, CtrlValue, EnvLv_len);
    float enhance_chroma = LinearInterp(pCalibV21->enhance_setting.EnhanceData.EnvLv, pCalibV21->enhance_setting.EnhanceData.enhance_chroma, CtrlValue, EnvLv_len);

    ProcRes->ProcResV21.enhance_value = int(enhance_value * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value
    ProcRes->ProcResV21.enhance_chroma = int(enhance_chroma * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value

    for(int i = 0; i < ISP21_DHAZ_ENH_CURVE_NUM; i++)
        ProcRes->ProcResV21.enh_curve[i] = (int)(pCalibV21->enhance_setting.enhance_curve[i]);

    if(ProcRes->ProcResV21.dc_en && ProcRes->ProcResV21.enhance_en) {
        LOGD_ADEHAZE("%s EnvLv:%f enhance_value:%f enhance_chroma:%f\n", __func__,
                     CtrlValue, enhance_value, enhance_chroma);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     ProcRes->ProcResV21.enhance_value, ProcRes->ProcResV21.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetEnhanceParamsV30(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->enhance_setting.EnhanceData.EnvLv_len;
    float enhance_value = LinearInterp(pCalibV21->enhance_setting.EnhanceData.EnvLv, pCalibV21->enhance_setting.EnhanceData.enhance_value, CtrlValue, EnvLv_len);
    float enhance_chroma = LinearInterp(pCalibV21->enhance_setting.EnhanceData.EnvLv, pCalibV21->enhance_setting.EnhanceData.enhance_chroma, CtrlValue, EnvLv_len);

    ProcRes->ProcResV30.enhance_value = int(enhance_value * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value
    ProcRes->ProcResV30.enhance_chroma = int(enhance_chroma * 1024 + 0.5); //       (14bit),4bit + 10bit, enhance_value

    for(int i = 0; i < ISP3X_DHAZ_ENH_CURVE_NUM; i++)
        ProcRes->ProcResV30.enh_curve[i] = (int)(pCalibV21->enhance_setting.enhance_curve[i]);

    if(ProcRes->ProcResV30.dc_en && ProcRes->ProcResV30.enhance_en) {
        LOGD_ADEHAZE("%s EnvLv:%f enhance_value:%f enhance_chroma:%f\n", __func__,
                     CtrlValue, enhance_value, enhance_chroma);
        LOGD_ADEHAZE("%s enhance_value_reg:0x%x enhance_chroma_reg:0x%x\n", __func__,
                     ProcRes->ProcResV30.enhance_value, ProcRes->ProcResV30.enhance_chroma);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParams(CalibDbV2_dehaze_V20_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int iso_len = pCalib->DehazeTuningPara.hist_setting.HistData.ISO_len;
    bool hist_channel = pCalib->DehazeTuningPara.hist_setting.hist_channel;
    bool hist_para_en = pCalib->DehazeTuningPara.hist_setting.hist_para_en;
    float hist_gratio = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.hist_gratio, CtrlValue, iso_len);
    float hist_th_off = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.hist_th_off, CtrlValue, iso_len);
    float hist_k = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.hist_k, CtrlValue, iso_len);
    float hist_min = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.hist_min, CtrlValue, iso_len);
    float hist_scale = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.hist_scale, CtrlValue, iso_len);
    float cfg_gratio = LinearInterp(pCalib->DehazeTuningPara.hist_setting.HistData.ISO, pCalib->DehazeTuningPara.hist_setting.HistData.cfg_gratio, CtrlValue, iso_len);

    // dehaze_hist_t0[6],dehaze_hist_t1[6],dehaze_hist_t2[6]
    float hist_conv_t0[ISP2X_DHAZ_CONV_COEFF_NUM] = {1.0000, 2.0000, 1.0000, -1.0000, -2.0000, -1.0000};
    float hist_conv_t1[ISP2X_DHAZ_CONV_COEFF_NUM] = {1.0000, 0.0000, -1.0000, 2.0000, 0.0000, -2.0000};
    float hist_conv_t2[ISP2X_DHAZ_CONV_COEFF_NUM] = {1.0000, -2.0000, 1.0000, 2.0000, -4.0000, 2.0000};

    ProcRes->ProcResV20.hist_chn = hist_channel ? 1 : 0; //  hist_para_en
    ProcRes->ProcResV20.hpara_en = hist_para_en ? 1 : 0; //  hist_para_en
    ProcRes->ProcResV20.hist_gratio   = int(hist_gratio * 8); //       (8bit) hist_gratio
    ProcRes->ProcResV20.hist_th_off   = int(hist_th_off);  //       (8bit) hist_th_off
    ProcRes->ProcResV20.hist_k   = int(hist_k * 4 + 0.5); //0~7    (5bit),3bit+2bit, hist_k
    ProcRes->ProcResV20.hist_min   = int(hist_min * 256); //       (9bit) hist_min
    ProcRes->ProcResV20.cfg_gratio = int(cfg_gratio * 256); //       (13bit),5bit+8bit, cfg_gratio
    ProcRes->ProcResV20.hist_scale       = int(hist_scale *  256 + 0.5 );  //       (13bit),5bit + 8bit, sw_hist_scale

    for (int i = 0; i < ISP2X_DHAZ_CONV_COEFF_NUM; i++)
    {
        ProcRes->ProcResV20.conv_t0[i]     = int(hist_conv_t0[i]);
        ProcRes->ProcResV20.conv_t1[i]     = int(hist_conv_t1[i]);
        ProcRes->ProcResV20.conv_t2[i]     = int(hist_conv_t2[i]);
    }

    if(ProcRes->ProcResV20.hist_en) {
        LOGD_ADEHAZE("%s hist_channel:%d hist_prar_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f hist_min:%f hist_scale:%f cfg_gratio:%f\n", __func__,
                     ProcRes->ProcResV20.hist_chn, ProcRes->ProcResV20.hpara_en, hist_gratio, hist_th_off, hist_k, hist_min, hist_scale, cfg_gratio);
        LOGD_ADEHAZE("%s hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n", __func__,
                     ProcRes->ProcResV20.hist_gratio, ProcRes->ProcResV20.hist_th_off, ProcRes->ProcResV20.hist_k,
                     ProcRes->ProcResV20.hist_min, ProcRes->ProcResV20.hist_scale, ProcRes->ProcResV20.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV21(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->hist_setting.HistData.EnvLv_len;
    bool hist_para_en = pCalibV21->hist_setting.hist_para_en;
    float hist_gratio = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_gratio, CtrlValue, EnvLv_len);
    float hist_th_off = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_th_off, CtrlValue, EnvLv_len);
    float hist_k = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_k, CtrlValue, EnvLv_len);
    float hist_min = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_min, CtrlValue, EnvLv_len);
    float hist_scale = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_scale, CtrlValue, EnvLv_len);
    float cfg_gratio = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.cfg_gratio, CtrlValue, EnvLv_len);

    ProcRes->ProcResV21.hpara_en = hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE; //  hist_para_en
    //clip hpara_en
    ProcRes->ProcResV21.hpara_en = ProcRes->ProcResV21.dc_en ? ProcRes->ProcResV21.hpara_en : FUNCTION_ENABLE; //  dc en 关闭，hpara必需开

    ProcRes->ProcResV21.hist_gratio   = int(hist_gratio * 8); //       (8bit) hist_gratio
    ProcRes->ProcResV21.hist_th_off   = int(hist_th_off);  //       (8bit) hist_th_off
    ProcRes->ProcResV21.hist_k   = int(hist_k * 4 + 0.5); //0~7    (5bit),3bit+2bit, hist_k
    ProcRes->ProcResV21.hist_min   = int(hist_min * 256); //       (9bit) hist_min
    ProcRes->ProcResV21.cfg_gratio = int(cfg_gratio * 256); //       (13bit),5bit+8bit, cfg_gratio
    ProcRes->ProcResV21.hist_scale       = int(hist_scale *  256 + 0.5 );  //       (13bit),5bit + 8bit, sw_hist_scale

    if(ProcRes->ProcResV21.hist_en) {
        LOGD_ADEHAZE("%s cfg_alpha:%f EnvLv:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f hist_min:%f hist_scale:%f cfg_gratio:%f\n", __func__,
                     ProcRes->ProcResV21.cfg_alpha / 255.0, CtrlValue, ProcRes->ProcResV21.hpara_en, hist_gratio, hist_th_off, hist_k, hist_min, hist_scale, cfg_gratio);
        LOGD_ADEHAZE("%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n", __func__,
                     ProcRes->ProcResV21.cfg_alpha, ProcRes->ProcResV21.hist_gratio, ProcRes->ProcResV21.hist_th_off, ProcRes->ProcResV21.hist_k,
                     ProcRes->ProcResV21.hist_min, ProcRes->ProcResV21.hist_scale, ProcRes->ProcResV21.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetHistParamsV30(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    int EnvLv_len = pCalibV21->hist_setting.HistData.EnvLv_len;
    bool hist_para_en = pCalibV21->hist_setting.hist_para_en;
    float hist_gratio = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_gratio, CtrlValue, EnvLv_len);
    float hist_th_off = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_th_off, CtrlValue, EnvLv_len);
    float hist_k = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_k, CtrlValue, EnvLv_len);
    float hist_min = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_min, CtrlValue, EnvLv_len);
    float hist_scale = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.hist_scale, CtrlValue, EnvLv_len);
    float cfg_gratio = LinearInterp(pCalibV21->hist_setting.HistData.EnvLv, pCalibV21->hist_setting.HistData.cfg_gratio, CtrlValue, EnvLv_len);

    ProcRes->ProcResV30.hpara_en = hist_para_en ? FUNCTION_ENABLE : FUNCTION_DISABLE; //  hist_para_en
    //clip hpara_en
    ProcRes->ProcResV30.hpara_en = ProcRes->ProcResV30.dc_en ? ProcRes->ProcResV30.hpara_en : FUNCTION_ENABLE; //  dc en 关闭，hpara必需开

    ProcRes->ProcResV30.hist_gratio   = int(hist_gratio * 8); //       (8bit) hist_gratio
    ProcRes->ProcResV30.hist_th_off   = int(hist_th_off);  //       (8bit) hist_th_off
    ProcRes->ProcResV30.hist_k   = int(hist_k * 4 + 0.5); //0~7    (5bit),3bit+2bit, hist_k
    ProcRes->ProcResV30.hist_min   = int(hist_min * 256); //       (9bit) hist_min
    ProcRes->ProcResV30.cfg_gratio = int(cfg_gratio * 256); //       (13bit),5bit+8bit, cfg_gratio
    ProcRes->ProcResV30.hist_scale       = int(hist_scale *  256 + 0.5 );  //       (13bit),5bit + 8bit, sw_hist_scale

    if(ProcRes->ProcResV30.hist_en) {
        LOGD_ADEHAZE("%s cfg_alpha:%f EnvLv:%f hist_para_en:%d hist_gratio:%f hist_th_off:%f hist_k:%f hist_min:%f hist_scale:%f cfg_gratio:%f\n", __func__,
                     ProcRes->ProcResV30.cfg_alpha / 255.0, CtrlValue, ProcRes->ProcResV30.hpara_en, hist_gratio, hist_th_off, hist_k, hist_min, hist_scale, cfg_gratio);
        LOGD_ADEHAZE("%s cfg_alpha_reg:0x%x hist_gratio_reg:0x%x hist_th_off_reg:0x%x hist_k_reg:0x%x hist_min_reg:0x%x hist_scale_reg:0x%x cfg_gratio_reg:0x%x\n", __func__,
                     ProcRes->ProcResV30.cfg_alpha, ProcRes->ProcResV30.hist_gratio, ProcRes->ProcResV30.hist_th_off, ProcRes->ProcResV30.hist_k,
                     ProcRes->ProcResV30.hist_min, ProcRes->ProcResV30.hist_scale, ProcRes->ProcResV30.cfg_gratio);
    }

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeHistDuoISPSettingV30(RkAiqAdehazeProcResult_t* ProcRes, rkisp_adehaze_stats_t* pStats, bool DuoCamera, int FrameID)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    //round_en
    ProcRes->ProcResV30.round_en = FUNCTION_ENABLE;

    //deahze duo setting
    if(DuoCamera) {
        ProcRes->ProcResV30.soft_wr_en = FUNCTION_ENABLE;
#if 1
        //support default value for kernel calc
        for(int i = 0; i < ISP3X_DHAZ_HIST_WR_NUM; i++) {
            ProcRes->ProcResV30.hist_wr[i] = 16 * (i + 1);
            ProcRes->ProcResV30.hist_wr[i] = ProcRes->ProcResV30.hist_wr[i] > 1023 ? 1023 : ProcRes->ProcResV30.hist_wr[i];
        }
#else
        ProcRes->ProcResV30.adp_air_wr = pStats->dehaze_stats_v30.dhaz_adp_air_base;
        ProcRes->ProcResV30.adp_gratio_wr = pStats->dehaze_stats_v30.dhaz_adp_gratio;
        ProcRes->ProcResV30.adp_tmax_wr = pStats->dehaze_stats_v30.dhaz_adp_tmax;
        ProcRes->ProcResV30.adp_wt_wr = pStats->dehaze_stats_v30.dhaz_adp_wt;

        static int hist_wr[64];
        if(!FrameID)
            for(int i = 0; i < 64; i++) {
                hist_wr[i] = 16 * (i + 1);
                hist_wr[i] = hist_wr[i] > 1023 ? 1023 : hist_wr[i];
                ProcRes->ProcResV30.hist_wr[i] = hist_wr[i];
            }
        else {
            int num = MIN(FrameID + 1, ProcRes->ProcResV30.stab_fnum);
            int tmp = 0;
            for(int i = 0; i < 64; i++) {
                tmp = (hist_wr[i] * (num - 1) + pStats->dehaze_stats_v30.h_rgb_iir[i]) / num;
                ProcRes->ProcResV30.hist_wr[i] = tmp;
                hist_wr[i] = tmp;
            }
        }

        LOGD_ADEHAZE("%s adp_air_wr:0x%x adp_gratio_wr:0x%x adp_tmax_wr:0x%x adp_wt_wr:0x%x\n", __func__, ProcRes->ProcResV30.adp_air_wr, ProcRes->ProcResV30.adp_gratio_wr,
                     ProcRes->ProcResV30.adp_tmax_wr, ProcRes->ProcResV30.adp_wt_wr);

        LOGV_ADEHAZE("%s hist_wr:0x%x", __func__, ProcRes->ProcResV30.hist_wr[0]);
        for(int i = 1; i < 63; i++)
            LOGV_ADEHAZE(" 0x%x", ProcRes->ProcResV30.hist_wr[i]);
        LOGV_ADEHAZE(" 0x%x\n", ProcRes->ProcResV30.hist_wr[63]);
#endif
        LOGD_ADEHAZE("%s DuoCamera:%d soft_wr_en:%d\n", __func__, DuoCamera, ProcRes->ProcResV30.soft_wr_en);
    }
    else
        ProcRes->ProcResV30.soft_wr_en = FUNCTION_DISABLE;

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetDehazeLocalGainSettingV30(RkAiqAdehazeProcResult_t* pProcRes, CalibDbV2_YnrV3_CalibPara_t* pYnrCalib, float ISO, YnrSnrMode_t SnrMode)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if(pYnrCalib->Setting_len >= 1 && SnrMode <= pYnrCalib->Setting_len) {
        if(pYnrCalib->Setting[SnrMode].Calib_ISO_len >= 1) {
            //get iso knots and ratio
            float ISO_lo = 50.0;
            float ISO_hi = 50.0;
            float ratio = 1.0;
            int ISO_knot_lo = 0;
            int ISO_knot_hi = 0;
            for(int i = 0; i < pYnrCalib->Setting[SnrMode].Calib_ISO_len - 1; i++) {
                if(ISO >= pYnrCalib->Setting[SnrMode].Calib_ISO[i].iso && ISO <= pYnrCalib->Setting[SnrMode].Calib_ISO[i + 1].iso) {
                    ISO_knot_lo = i;
                    ISO_knot_hi = i + 1;
                    ISO_lo = pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].iso;
                    ISO_hi = pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].iso;

                    if((ISO_hi - ISO_lo) != 0)
                        ratio = (ISO - ISO_lo ) / (ISO_hi - ISO_lo);
                    else
                        LOGE_ADEHAZE("Dehaze zero in %s(%d) \n", __func__, __LINE__);
                    break;
                }
                else
                    continue;
            }

            //get noiseSigma
            float* noiseSigma_lo = (float*)malloc(sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            memset(noiseSigma_lo, 0x0, sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            float* noiseSigma_hi = (float*)malloc(sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            memset(noiseSigma_hi, 0x0, sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            float* noiseSigma = (float*)malloc(sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            memset(noiseSigma, 0x0, sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
            for(int i = 0; i < ISP3X_DHAZ_SIGMA_LUT_NUM; i++) {
                float ave1, ave2, ave3, ave4;
                if(i == (ISP3X_DHAZ_SIGMA_LUT_NUM - 1))
                    ave1 = (float)YNR_ISO_CURVE_SECT_VALUE1;
                else
                    ave1 = (float)(i * YNR_ISO_CURVE_SECT_VALUE);

                ave2 = ave1 * ave1;
                ave3 = ave2 * ave1;
                ave4 = ave3 * ave1;
                noiseSigma_lo[i] = pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].sigma_curve[0] * ave4
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].sigma_curve[1] * ave3
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].sigma_curve[2] * ave2
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].sigma_curve[3] * ave1
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_lo].sigma_curve[4];

                if(noiseSigma_lo[i] < 0)
                    noiseSigma_lo[i] = 0;

                noiseSigma_hi[i] = pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].sigma_curve[0] * ave4
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].sigma_curve[1] * ave3
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].sigma_curve[2] * ave2
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].sigma_curve[3] * ave1
                                   + pYnrCalib->Setting[SnrMode].Calib_ISO[ISO_knot_hi].sigma_curve[4];

                if(noiseSigma_hi[i] < 0)
                    noiseSigma_hi[i] = 0;

                noiseSigma[i] = ratio * (noiseSigma_hi[i] - noiseSigma_lo[i]) + noiseSigma_lo[i];
            }

            //get proc res
            //get sigma_idx
            for(int i = 0; i < ISP3X_DHAZ_SIGMA_IDX_NUM; i++)
                pProcRes->ProcResV30.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;

            //get sigma_lut
            int tmp = 0;
            for(int i = 0; i < ISP3X_DHAZ_SIGMA_LUT_NUM; i++) {
                tmp = LIMIT_VALUE(noiseSigma[i], ADHZ10BITMAX, ADHZ10BITMIN);
                pProcRes->ProcResV30.sigma_lut[i] = tmp;
            }

            free(noiseSigma);
            free(noiseSigma_lo);
            free(noiseSigma_hi);

#if 0
            LOGE_ADEHAZE("%s(%d) ISO:%f SnrMode:%d ISO_lo:%f ISO_hi:%f\n", __func__, __LINE__, ISO, SnrMode, ISO_lo, ISO_hi);
            LOGE_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV30.sigma_idx[0], pProcRes->ProcResV30.sigma_idx[1],
                         pProcRes->ProcResV30.sigma_idx[2], pProcRes->ProcResV30.sigma_idx[3], pProcRes->ProcResV30.sigma_idx[4], pProcRes->ProcResV30.sigma_idx[5]);
            LOGE_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV30.sigma_lut[0], pProcRes->ProcResV30.sigma_lut[1],
                         pProcRes->ProcResV30.sigma_lut[2], pProcRes->ProcResV30.sigma_lut[3], pProcRes->ProcResV30.sigma_lut[4], pProcRes->ProcResV30.sigma_lut[5]);
#endif
        }
        else
            LOGE_ADEHAZE("%s(%d) Ynr calib setting ISO length is under 1!!!\n", __func__, __LINE__);
    }
    else
        LOGE_ADEHAZE("%s(%d) Ynr calib setting length is under 1!!!\n", __func__, __LINE__);

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void GetManuDehazeLocalGainSettingV30(RkAiqAdehazeProcResult_t* pProcRes, mDehazeAttr_t* pstManu) {
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    // get noiseSigma
    float* noiseSigma = (float*)malloc(sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));
    memset(noiseSigma, 0x0, sizeof(float) * (ISP3X_DHAZ_SIGMA_LUT_NUM));

    for (int i = 0; i < ISP3X_DHAZ_SIGMA_LUT_NUM; i++) {
        float ave1, ave2, ave3, ave4;
        if (i == (ISP3X_DHAZ_SIGMA_LUT_NUM - 1))
            ave1 = (float)YNR_ISO_CURVE_SECT_VALUE1;
        else
            ave1 = (float)(i * YNR_ISO_CURVE_SECT_VALUE);

        ave2 = ave1 * ave1;
        ave3 = ave2 * ave1;
        ave4 = ave3 * ave1;

        noiseSigma[i] = pstManu->sigma_curve[0] * ave4 + pstManu->sigma_curve[1] * ave3 +
                        pstManu->sigma_curve[2] * ave2 + pstManu->sigma_curve[3] * ave1 +
                        pstManu->sigma_curve[4];
    }

    // get proc res
    // get sigma_idx
    for (int i = 0; i < ISP3X_DHAZ_SIGMA_IDX_NUM; i++)
        pProcRes->ProcResV30.sigma_idx[i] = (i + 1) * YNR_CURVE_STEP;

    // get sigma_lut
    int tmp = 0;
    for (int i = 0; i < ISP3X_DHAZ_SIGMA_LUT_NUM; i++) {
        tmp                               = LIMIT_VALUE(noiseSigma[i], ADHZ10BITMAX, ADHZ10BITMIN);
        pProcRes->ProcResV30.sigma_lut[i] = tmp;
    }

    free(noiseSigma);
#if 0
            LOGE_ADEHAZE("%s(%d) dehaze local gain IDX(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV30.sigma_idx[0], pProcRes->ProcResV30.sigma_idx[1],
                         pProcRes->ProcResV30.sigma_idx[2], pProcRes->ProcResV30.sigma_idx[3], pProcRes->ProcResV30.sigma_idx[4], pProcRes->ProcResV30.sigma_idx[5]);
            LOGE_ADEHAZE("%s(%d) dehaze local gain LUT(0~5): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, pProcRes->ProcResV30.sigma_lut[0], pProcRes->ProcResV30.sigma_lut[1],
                         pProcRes->ProcResV30.sigma_lut[2], pProcRes->ProcResV30.sigma_lut[3], pProcRes->ProcResV30.sigma_lut[4], pProcRes->ProcResV30.sigma_lut[5]);
#endif

    LOG1_ADEHAZE("EIXT: %s \n", __func__);
}

void AdehazeApiToolProcess(CalibDbV2_dehaze_V20_t* pStool, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE("%s: Adehaze in api TOOL !!! \n", __func__);

    //cfg setting
    ProcRes->ProcResV20.cfg_alpha = (int)LIMIT_VALUE((pStool->DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
    LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV20.cfg_alpha);

    //fuction enable
    EnableSetting(pStool, ProcRes);

    //dehaze setting
    GetDehazeParams(pStool, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParams(pStool, ProcRes, CtrlValue);

    //hist setting
    GetHistParams(pStool, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeApiManuV21Process(RkAiqAdehazeProcResult_t* pProcRes, mDehazeAttr_t* pStManu)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api stManual!!!\n", __func__);

    //cfg setting
    pProcRes->ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pStManu->cfg_alpha * 256.0), 255, 0);

    //enable setting
    stManuEnableSettingV21(pStManu, pProcRes);

    //dehaze setting
    stManuGetDehazeParamsV21(pStManu, pProcRes);

    //enhance setting
    stManuGetEnhanceParamsV21(pStManu, pProcRes);

    //hist setting
    stManuGetHistParamsV21(pStManu, pProcRes);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeApiManuV30Process(RkAiqAdehazeProcResult_t* pProcRes, mDehazeAttr_t* pStManu)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api stManual!!!\n", __func__);

    //cfg setting
    pProcRes->ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pStManu->cfg_alpha * 256.0), 255, 0);

    //enable setting
    stManuEnableSettingV30(pStManu, pProcRes);

    //dehaze setting
    stManuGetDehazeParamsV30(pStManu, pProcRes);

    //enhance setting
    stManuGetEnhanceParamsV30(pStManu, pProcRes);

    //hist setting
    stManuGetHistParamsV30(pStManu, pProcRes);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeEnhanceApiBypassProcess(CalibDbV2_dehaze_V20_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //cfg setting
    ProcRes->ProcResV20.cfg_alpha = (int)LIMIT_VALUE((pCalib->DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV20.cfg_alpha);

    //enable setting
    EnableSetting(pCalib, ProcRes);

    //dehaze setting
    GetDehazeParams(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParams(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParams(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeEnhanceApiBypassV21Process(CalibDbV2_dehaze_V21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //cfg setting
    ProcRes->ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pCalibV21->DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV20.cfg_alpha);

    //enable setting
    EnableSettingV21(&pCalibV21->DehazeTuningPara, ProcRes);

    //dehaze setting
    GetDehazeParamsV21(&pCalibV21->DehazeTuningPara, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV21(&pCalibV21->DehazeTuningPara, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV21(&pCalibV21->DehazeTuningPara, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeEnhanceApiBypassV30Process(CalibDbDehazeV21_t* pCalibV21, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //cfg setting
    ProcRes->ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pCalibV21->cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV30.cfg_alpha);

    //enable setting
    EnableSettingV30(pCalibV21, ProcRes);

    //dehaze setting
    GetDehazeParamsV30(pCalibV21, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV30(pCalibV21, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV30(pCalibV21, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void AdehazeEnhanceApiOffProcess(CalibDbV2_dehaze_V20_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //enable setting
    ProcRes->ProcResV20.enable = true;
    if(pCalib->DehazeTuningPara.enhance_setting.en) {
        ProcRes->ProcResV20.dc_en = true;
        ProcRes->ProcResV20.enhance_en = true;
    }
    else {
        ProcRes->ProcResV20.dc_en = false;
        ProcRes->ProcResV20.enhance_en = false;
    }

    //cfg setting
    ProcRes->ProcResV20.cfg_alpha = (int)LIMIT_VALUE((pCalib->DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV20.cfg_alpha);

    //dehaze setting
    GetDehazeParams(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParams(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParams(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void ApiDehazeOffProcessV21(CalibDbDehazeV21_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //enable setting
    ProcRes->ProcResV21.enable = true;
    if(pCalib->enhance_setting.en) {
        ProcRes->ProcResV21.dc_en = true;
        ProcRes->ProcResV21.enhance_en = true;
    }
    else {
        ProcRes->ProcResV21.dc_en = false;
        ProcRes->ProcResV21.enhance_en = false;
    }

    //hist en
    ProcRes->ProcResV21.hist_en = pCalib->hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n",
                 ProcRes->ProcResV21.dc_en || ProcRes->ProcResV21.enhance_en || ProcRes->ProcResV21.hist_en, FUNCTION_DISABLE, pCalib->enhance_setting.en, ProcRes->ProcResV21.hist_en );

    //cfg setting
    ProcRes->ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pCalib->cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV21.cfg_alpha);

    //dehaze setting
    GetDehazeParamsV21(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV21(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV21(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void ApiDehazeOffProcessV30(CalibDbDehazeV21_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    //enable setting
    ProcRes->ProcResV30.enable = true;
    if(pCalib->enhance_setting.en) {
        ProcRes->ProcResV30.dc_en = FUNCTION_ENABLE;
        ProcRes->ProcResV30.enhance_en = FUNCTION_ENABLE;
    }
    else {
        ProcRes->ProcResV30.dc_en = FUNCTION_DISABLE;
        ProcRes->ProcResV30.enhance_en = FUNCTION_DISABLE;
    }

    //hist en
    ProcRes->ProcResV30.hist_en = pCalib->hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);
    LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n",
                 ProcRes->ProcResV30.dc_en || ProcRes->ProcResV30.enhance_en || ProcRes->ProcResV30.hist_en, FUNCTION_DISABLE, pCalib->enhance_setting.en, ProcRes->ProcResV30.hist_en );
    //cfg setting
    ProcRes->ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pCalib->cfg_alpha * 256.0), 255, 0);
    //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, ProcRes->ProcResV30.cfg_alpha);

    //dehaze setting
    GetDehazeParamsV30(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV30(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV30(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void ApiEnhanceOffProcessV21(CalibDbDehazeV21_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //enable setting
    ProcRes->ProcResV21.enable = true;
    if(pCalib->dehaze_setting.en) {
        ProcRes->ProcResV21.dc_en = true;
        ProcRes->ProcResV21.enhance_en = false;
    }
    else {
        ProcRes->ProcResV21.dc_en = false;
        ProcRes->ProcResV21.enhance_en = false;
    }

    //hist en
    ProcRes->ProcResV21.hist_en = pCalib->hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n",
                 ProcRes->ProcResV21.dc_en || ProcRes->ProcResV21.enhance_en || ProcRes->ProcResV21.hist_en, FUNCTION_DISABLE, pCalib->enhance_setting.en, ProcRes->ProcResV21.hist_en );

    //cfg setting
    ProcRes->ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pCalib->cfg_alpha * 256.0), 255, 0);

    //dehaze setting
    GetDehazeParamsV21(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV21(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV21(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

void ApiEnhanceOffProcessV30(CalibDbDehazeV21_t* pCalib, RkAiqAdehazeProcResult_t* ProcRes, float CtrlValue)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    LOGD_ADEHAZE(" %s: Adehaze Api off!!!\n", __func__);

    //enable setting
    ProcRes->ProcResV30.enable = true;
    if(pCalib->dehaze_setting.en) {
        ProcRes->ProcResV30.dc_en = true;
        ProcRes->ProcResV30.enhance_en = false;
    }
    else {
        ProcRes->ProcResV30.dc_en = false;
        ProcRes->ProcResV30.enhance_en = false;
    }

    //hist en
    ProcRes->ProcResV30.hist_en = pCalib->hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
    LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n",
                 ProcRes->ProcResV30.dc_en || ProcRes->ProcResV30.enhance_en || ProcRes->ProcResV30.hist_en, FUNCTION_DISABLE, pCalib->enhance_setting.en, ProcRes->ProcResV30.hist_en );

    //cfg setting
    ProcRes->ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pCalib->cfg_alpha * 256.0), 255, 0);

    //dehaze setting
    GetDehazeParamsV30(pCalib, ProcRes, CtrlValue);

    //enhance setting
    GetEnhanceParamsV30(pCalib, ProcRes, CtrlValue);

    //hist setting
    GetHistParamsV30(pCalib, ProcRes, CtrlValue);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
}

XCamReturn AdehazeProcessV21(AdehazeHandle_t* pAdehazeCtx, float CtrlValue)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_BYPASS)
        AdehazeEnhanceApiBypassV21Process(&pAdehazeCtx->Calib.Dehaze_v21, &pAdehazeCtx->ProcRes, CtrlValue);
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_MANUAL)
        AdehazeApiManuV21Process(&pAdehazeCtx->ProcRes, &pAdehazeCtx->AdehazeAtrr.stManual);
    else if(pAdehazeCtx->AdehazeAtrr.mode > DEHAZE_API_MANUAL && pAdehazeCtx->AdehazeAtrr.mode < DEHAZE_API_DEHAZE_OFF) {
        pAdehazeCtx->ProcRes.ProcResV21.enable = true;
        pAdehazeCtx->ProcRes.ProcResV21.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV21.enhance_en = FUNCTION_DISABLE;
        //LOGD_ADEHAZE(" Dehaze fuction en:%d, Dehaze en:%d, Enhance en:%d,", 1, 1, 0 );

        if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_AUTO)
            pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_MANUAL)
            pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha = 255;
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha);

        //hist en setting
        if(pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.hist_setting.en)
            pAdehazeCtx->ProcRes.ProcResV21.hist_en = FUNCTION_ENABLE;
        else
            pAdehazeCtx->ProcRes.ProcResV21.hist_en = FUNCTION_DISABLE;

        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_ENABLE, FUNCTION_DISABLE, pAdehazeCtx->ProcRes.ProcResV21.hist_en );

        GetDehazeParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_MANUAL)
        {
            float level = (float)(pAdehazeCtx->AdehazeAtrr.stDehazeManu.level);
            float level_diff = (float)(level - DEHAZE_API_MANUAL_DEFAULT_LEVEL);

            //sw_dhaz_cfg_wt
            float sw_dhaz_cfg_wt = (float)pAdehazeCtx->ProcRes.ProcResV21.cfg_wt;
            sw_dhaz_cfg_wt += level_diff * 0.005;
            sw_dhaz_cfg_wt = LIMIT_VALUE(sw_dhaz_cfg_wt, 0.99, 0.01);
            pAdehazeCtx->ProcRes.ProcResV21.cfg_wt = (int)sw_dhaz_cfg_wt;

            //sw_dhaz_cfg_air
            float sw_dhaz_cfg_air = (float)pAdehazeCtx->ProcRes.ProcResV21.cfg_air;
            sw_dhaz_cfg_air += level_diff * 0.5;
            sw_dhaz_cfg_air = LIMIT_VALUE(sw_dhaz_cfg_air, 255, 0.01);
            pAdehazeCtx->ProcRes.ProcResV21.cfg_air = (int)sw_dhaz_cfg_air;

            //sw_dhaz_cfg_tmax
            float sw_dhaz_cfg_tmax = (float)pAdehazeCtx->ProcRes.ProcResV21.cfg_tmax;
            sw_dhaz_cfg_tmax += level_diff * 0.005;
            sw_dhaz_cfg_tmax = LIMIT_VALUE(sw_dhaz_cfg_tmax, 0.99, 0.01);
            pAdehazeCtx->ProcRes.ProcResV21.cfg_tmax = (int)sw_dhaz_cfg_tmax;

            LOGD_ADEHAZE(" %s: Adehaze munual level:%f level_diff:%f\n", __func__, level, level_diff);
            LOGD_ADEHAZE(" %s: After manual api sw_dhaz_cfg_wt:%f sw_dhaz_cfg_air:%f sw_dhaz_cfg_tmax:%f\n", __func__, sw_dhaz_cfg_wt,
                         sw_dhaz_cfg_air, sw_dhaz_cfg_tmax);
        }
        //hist setting
        GetHistParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_OFF)
        ApiDehazeOffProcessV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_MANUAL) {
        pAdehazeCtx->ProcRes.ProcResV21.enable = true;
        pAdehazeCtx->ProcRes.ProcResV21.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV21.enhance_en = FUNCTION_ENABLE;

        //cfg setting
        pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha);

        //hist en
        pAdehazeCtx->ProcRes.ProcResV21.hist_en = pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_DISABLE, FUNCTION_ENABLE, pAdehazeCtx->ProcRes.ProcResV21.hist_en );

        //dehaze setting
        GetDehazeParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //enhance setting
        GetEnhanceParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        float level = (float)(pAdehazeCtx->AdehazeAtrr.stEnhanceManu.level);
        level -= DEHAZE_API_ENHANCE_MANUAL_DEFAULT_LEVEL;
        float step = (float)(pAdehazeCtx->ProcRes.ProcResV21.enhance_value - 1024);
        step = LIMIT_VALUE(step, 30.9, 0);
        step /= DEHAZE_API_ENHANCE_MANUAL_DEFAULT_LEVEL;
        pAdehazeCtx->ProcRes.ProcResV21.enhance_value =  pAdehazeCtx->ProcRes.ProcResV21.enhance_value + (int)(step * level);

        LOGD_ADEHAZE("%s After enhance api enhance_value:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV21.enhance_value);

        //hist setting
        GetHistParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_AUTO) {
        pAdehazeCtx->ProcRes.ProcResV21.enable = true;
        pAdehazeCtx->ProcRes.ProcResV21.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV21.enhance_en = FUNCTION_ENABLE;

        //cfg setting
        pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV21.cfg_alpha);

        //hist en
        pAdehazeCtx->ProcRes.ProcResV21.hist_en = pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_DISABLE, FUNCTION_ENABLE, pAdehazeCtx->ProcRes.ProcResV21.hist_en );

        //dehaze setting
        GetDehazeParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //enhance setting
        GetEnhanceParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //hist setting
        GetHistParamsV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_OFF)
        ApiEnhanceOffProcessV21(&pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

XCamReturn AdehazeProcessV30(AdehazeHandle_t* pAdehazeCtx, float CtrlValue)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_BYPASS)
        AdehazeEnhanceApiBypassV30Process(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_MANUAL)
        AdehazeApiManuV30Process(&pAdehazeCtx->ProcRes, &pAdehazeCtx->AdehazeAtrr.stManual);
    else if(pAdehazeCtx->AdehazeAtrr.mode > DEHAZE_API_MANUAL && pAdehazeCtx->AdehazeAtrr.mode < DEHAZE_API_DEHAZE_OFF) {
        pAdehazeCtx->ProcRes.ProcResV30.enable = true;
        pAdehazeCtx->ProcRes.ProcResV30.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV30.enhance_en = FUNCTION_DISABLE;
        //LOGD_ADEHAZE(" Dehaze fuction en:%d, Dehaze en:%d, Enhance en:%d,", 1, 1, 0 );

        if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_AUTO)
            pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_MANUAL)
            pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha = 255;
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha);

        //hist en setting
        if(pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.hist_setting.en)
            pAdehazeCtx->ProcRes.ProcResV30.hist_en = FUNCTION_ENABLE;
        else
            pAdehazeCtx->ProcRes.ProcResV30.hist_en = FUNCTION_DISABLE;

        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_ENABLE, FUNCTION_DISABLE, pAdehazeCtx->ProcRes.ProcResV30.hist_en );

        GetDehazeParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_MANUAL)
        {
            float level = (float)(pAdehazeCtx->AdehazeAtrr.stDehazeManu.level);
            float level_diff = (float)(level - DEHAZE_API_MANUAL_DEFAULT_LEVEL);

            //sw_dhaz_cfg_wt
            float sw_dhaz_cfg_wt = (float)pAdehazeCtx->ProcRes.ProcResV30.cfg_wt;
            sw_dhaz_cfg_wt += level_diff * 0.005;
            sw_dhaz_cfg_wt = LIMIT_VALUE(sw_dhaz_cfg_wt, 0.99, 0.01);
            pAdehazeCtx->ProcRes.ProcResV30.cfg_wt = (int)sw_dhaz_cfg_wt;

            //sw_dhaz_cfg_air
            float sw_dhaz_cfg_air = (float)pAdehazeCtx->ProcRes.ProcResV30.cfg_air;
            sw_dhaz_cfg_air += level_diff * 0.5;
            sw_dhaz_cfg_air = LIMIT_VALUE(sw_dhaz_cfg_air, 255, 0.01);
            pAdehazeCtx->ProcRes.ProcResV30.cfg_air = (int)sw_dhaz_cfg_air;

            //sw_dhaz_cfg_tmax
            float sw_dhaz_cfg_tmax = (float)pAdehazeCtx->ProcRes.ProcResV30.cfg_tmax;
            sw_dhaz_cfg_tmax += level_diff * 0.005;
            sw_dhaz_cfg_tmax = LIMIT_VALUE(sw_dhaz_cfg_tmax, 0.99, 0.01);
            pAdehazeCtx->ProcRes.ProcResV30.cfg_tmax = (int)sw_dhaz_cfg_tmax;

            LOGD_ADEHAZE(" %s: Adehaze munual level:%f level_diff:%f\n", __func__, level, level_diff);
            LOGD_ADEHAZE(" %s: After manual api sw_dhaz_cfg_wt:%f sw_dhaz_cfg_air:%f sw_dhaz_cfg_tmax:%f\n", __func__, sw_dhaz_cfg_wt,
                         sw_dhaz_cfg_air, sw_dhaz_cfg_tmax);
        }

        //hist setting
        GetHistParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_DEHAZE_OFF)
        ApiDehazeOffProcessV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_MANUAL) {
        pAdehazeCtx->ProcRes.ProcResV30.enable = true;
        pAdehazeCtx->ProcRes.ProcResV30.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV30.enhance_en = FUNCTION_ENABLE;

        //cfg setting
        pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha);

        //hist en
        pAdehazeCtx->ProcRes.ProcResV30.hist_en = pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;
        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_DISABLE, FUNCTION_ENABLE, pAdehazeCtx->ProcRes.ProcResV30.hist_en );

        //hist setting
        GetHistParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //dehaze setting
        GetDehazeParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //enhance setting
        GetEnhanceParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        float level = (float)(pAdehazeCtx->AdehazeAtrr.stEnhanceManu.level);
        level -= DEHAZE_API_ENHANCE_MANUAL_DEFAULT_LEVEL;
        float step = (float)(pAdehazeCtx->ProcRes.ProcResV30.enhance_value - 1024);
        step = LIMIT_VALUE(step, 30.9, 0);
        step /= DEHAZE_API_ENHANCE_MANUAL_DEFAULT_LEVEL;
        pAdehazeCtx->ProcRes.ProcResV30.enhance_value =  pAdehazeCtx->ProcRes.ProcResV30.enhance_value + (int)(step * level);

        LOGD_ADEHAZE("%s After enhance api enhance_value:0x%x\n", __func__, pAdehazeCtx->ProcRes.ProcResV30.enhance_value);
    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_AUTO) {
        pAdehazeCtx->ProcRes.ProcResV30.enable = true;
        pAdehazeCtx->ProcRes.ProcResV30.dc_en = FUNCTION_ENABLE;
        pAdehazeCtx->ProcRes.ProcResV30.enhance_en = FUNCTION_ENABLE;

        //cfg setting
        pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha = (int)LIMIT_VALUE((pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.cfg_alpha * 256.0), 255, 0);
        //LOGD_ADEHAZE("%s Config Alpha:%d\n", __func__, pAdehazeCtx->ProcRes.ProcResV30.cfg_alpha);

        //hist en
        pAdehazeCtx->ProcRes.ProcResV30.hist_en = pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.hist_setting.en ? FUNCTION_ENABLE : FUNCTION_DISABLE;

        LOGD_ADEHAZE(" Dehaze module en:%d, Dehaze en:%d, Enhance en:%d, Hist en:%d\n", FUNCTION_ENABLE,
                     FUNCTION_DISABLE, FUNCTION_ENABLE, pAdehazeCtx->ProcRes.ProcResV30.hist_en );

        //hist setting
        GetHistParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //dehaze setting
        GetDehazeParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);

        //enhance setting
        GetEnhanceParamsV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    }
    else if(pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_ENHANCE_OFF)
        ApiEnhanceOffProcessV30(&pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara, &pAdehazeCtx->ProcRes, CtrlValue);
    else
        LOGE_ADEHAZE("%s:Wrong Adehaze API mode!!! \n", __func__);

    //get Duo cam setting
    GetDehazeHistDuoISPSettingV30(&pAdehazeCtx->ProcRes, &pAdehazeCtx->stats, pAdehazeCtx->is_multi_isp_mode, pAdehazeCtx->FrameID);

    //get local gain setting
    if (pAdehazeCtx->AdehazeAtrr.mode == DEHAZE_API_MANUAL)
        GetManuDehazeLocalGainSettingV30(&pAdehazeCtx->ProcRes, &pAdehazeCtx->AdehazeAtrr.stManual);
    else
        GetDehazeLocalGainSettingV30(
            &pAdehazeCtx->ProcRes, &pAdehazeCtx->Calib.Dehaze_v30.YnrCalibPara,
            pAdehazeCtx->CurrData.V30.ISO, pAdehazeCtx->CurrData.V30.SnrMode);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

void AdehazeGetStats
(
    AdehazeHandle_t*           pAdehazeCtx,
    rkisp_adehaze_stats_t*         ROData
) {
    LOG1_ADEHAZE( "%s:enter!\n", __FUNCTION__);
    LOGV_ADEHAZE("%s: Adehaze RO data from register:\n", __FUNCTION__);

    if(CHECK_ISP_HW_V20()) {
        pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_air_base = ROData->dehaze_stats_v20.dhaz_adp_air_base;
        pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_wt = ROData->dehaze_stats_v20.dhaz_adp_wt;
        pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_gratio = ROData->dehaze_stats_v20.dhaz_adp_gratio;
        pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_tmax = ROData->dehaze_stats_v20.dhaz_adp_tmax;
        for(int i = 0; i < ISP2X_DHAZ_HIST_IIR_NUM; i++) {
            pAdehazeCtx->stats.dehaze_stats_v20.h_b_iir[i] = ROData->dehaze_stats_v20.h_b_iir[i];
            pAdehazeCtx->stats.dehaze_stats_v20.h_g_iir[i] = ROData->dehaze_stats_v20.h_g_iir[i];
            pAdehazeCtx->stats.dehaze_stats_v20.h_r_iir[i] = ROData->dehaze_stats_v20.h_r_iir[i];
        }

        LOGV_ADEHAZE("%s:  dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d dhaz_adp_tmax:%d\n", __FUNCTION__,
                     pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_air_base, pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_wt,
                     pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_gratio, pAdehazeCtx->stats.dehaze_stats_v20.dhaz_adp_tmax);
        for(int i = 0; i < ISP2X_DHAZ_HIST_IIR_NUM; i++) {
            LOGV_ADEHAZE("%s:  h_b_iir[%d]:%d:\n", __FUNCTION__, i, pAdehazeCtx->stats.dehaze_stats_v20.h_b_iir[i]);
            LOGV_ADEHAZE("%s:  h_g_iir[%d]:%d:\n", __FUNCTION__, i, pAdehazeCtx->stats.dehaze_stats_v20.h_g_iir[i]);
            LOGV_ADEHAZE("%s:  h_r_iir[%d]:%d:\n", __FUNCTION__, i, pAdehazeCtx->stats.dehaze_stats_v20.h_r_iir[i]);
        }
    }
    else if(CHECK_ISP_HW_V21()) {
        pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_air_base = ROData->dehaze_stats_v21.dhaz_adp_air_base;
        pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_wt = ROData->dehaze_stats_v21.dhaz_adp_wt;
        pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_gratio = ROData->dehaze_stats_v21.dhaz_adp_gratio;
        pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_tmax = ROData->dehaze_stats_v21.dhaz_adp_tmax;
        for(int i = 0; i < ISP21_DHAZ_HIST_IIR_NUM; i++)
            pAdehazeCtx->stats.dehaze_stats_v21.h_rgb_iir[i] = ROData->dehaze_stats_v21.h_rgb_iir[i];

        LOGV_ADEHAZE("%s:  dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d dhaz_adp_tmax:%d\n", __FUNCTION__,
                     pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_air_base, pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_wt,
                     pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_gratio, pAdehazeCtx->stats.dehaze_stats_v21.dhaz_adp_tmax);
        for(int i = 0; i < ISP21_DHAZ_HIST_IIR_NUM; i++)
            LOGV_ADEHAZE("%s:  h_rgb_iir[%d]:%d:\n", __FUNCTION__, i, pAdehazeCtx->stats.dehaze_stats_v21.h_rgb_iir[i]);
    }
    else if(CHECK_ISP_HW_V30()) {
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_air_base = ROData->dehaze_stats_v30.dhaz_adp_air_base;
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_wt = ROData->dehaze_stats_v30.dhaz_adp_wt;
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_gratio = ROData->dehaze_stats_v30.dhaz_adp_gratio;
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_tmax = ROData->dehaze_stats_v30.dhaz_adp_tmax;
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_pic_sumh_left = ROData->dehaze_stats_v30.dhaz_pic_sumh_left;
        pAdehazeCtx->stats.dehaze_stats_v30.dhaz_pic_sumh_right = ROData->dehaze_stats_v30.dhaz_pic_sumh_right;
        for(int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++)
            pAdehazeCtx->stats.dehaze_stats_v30.h_rgb_iir[i] = ROData->dehaze_stats_v30.h_rgb_iir[i];

        LOGV_ADEHAZE("%s:  dhaz_adp_air_base:%d dhaz_adp_wt:%d dhaz_adp_gratio:%d dhaz_adp_tmax:%d dhaz_pic_sumh_left:%d dhaz_pic_sumh_right:%d\n", __FUNCTION__,
                     pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_air_base, pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_wt,
                     pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_gratio, pAdehazeCtx->stats.dehaze_stats_v30.dhaz_adp_tmax,
                     pAdehazeCtx->stats.dehaze_stats_v30.dhaz_pic_sumh_left, pAdehazeCtx->stats.dehaze_stats_v30.dhaz_pic_sumh_right);
        for(int i = 0; i < ISP3X_DHAZ_HIST_IIR_NUM; i++)
            LOGV_ADEHAZE("%s:  h_rgb_iir[%d]:%d:\n", __FUNCTION__, i, pAdehazeCtx->stats.dehaze_stats_v30.h_rgb_iir[i]);
    }

    //get other stats from stats
    for(int i = 0; i < 225; i++)
    {
        pAdehazeCtx->stats.other_stats.short_luma[i] = ROData->other_stats.short_luma[i];
        pAdehazeCtx->stats.other_stats.long_luma[i] = ROData->other_stats.long_luma[i];
        pAdehazeCtx->stats.other_stats.tmo_luma[i] = ROData->other_stats.tmo_luma[i];
    }

    if(pAdehazeCtx->FrameNumber == HDR_3X_NUM)
    {
        for(int i = 0; i < 25; i++)
            pAdehazeCtx->stats.other_stats.middle_luma[i] = ROData->other_stats.middle_luma[i];
    }

    LOG1_ADEHAZE( "%s:exit!\n", __FUNCTION__);
}

void AdehazeGetEnvLvISO
(
    AdehazeHandle_t*           pAdehazeCtx,
    RkAiqAlgoPreResAe*         pAePreRes
) {
    LOG1_ADEHAZE( "%s:enter!\n", __FUNCTION__);

    if(pAePreRes == NULL) {
        LOGE_ADEHAZE( "%s:Ae Pre Res is NULL!\n", __FUNCTION__);
        pAdehazeCtx->CurrData.V21.EnvLv = 0.0;
        return;
    }

    if(CHECK_ISP_HW_V21()) {
        pAdehazeCtx->CurrData.V21.EnvLv = pAePreRes->ae_pre_res_rk.GlobalEnvLv[pAePreRes->ae_pre_res_rk.NormalIndex];

        //Normalize the current envLv for AEC
        pAdehazeCtx->CurrData.V21.EnvLv = (pAdehazeCtx->CurrData.V21.EnvLv  - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
        pAdehazeCtx->CurrData.V21.EnvLv = LIMIT_VALUE(pAdehazeCtx->CurrData.V21.EnvLv, ENVLVMAX, ENVLVMIN);

        //get iso
        if(pAdehazeCtx->FrameNumber == LINEAR_NUM)
            pAdehazeCtx->CurrData.V21.ISO = pAePreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain *
                                            pAePreRes->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain * 50.0;
        else if(pAdehazeCtx->FrameNumber == HDR_2X_NUM || pAdehazeCtx->FrameNumber == HDR_3X_NUM)
            pAdehazeCtx->CurrData.V21.ISO = pAePreRes->ae_pre_res_rk.HdrExp[1].exp_real_params.analog_gain *
                                            pAePreRes->ae_pre_res_rk.HdrExp[1].exp_real_params.digital_gain * 50.0;
    }
    else if(CHECK_ISP_HW_V30()) {
        pAdehazeCtx->CurrData.V30.EnvLv = pAePreRes->ae_pre_res_rk.GlobalEnvLv[pAePreRes->ae_pre_res_rk.NormalIndex];

        //Normalize the current envLv for AEC
        pAdehazeCtx->CurrData.V30.EnvLv = (pAdehazeCtx->CurrData.V30.EnvLv  - MIN_ENV_LV) / (MAX_ENV_LV - MIN_ENV_LV);
        pAdehazeCtx->CurrData.V30.EnvLv = LIMIT_VALUE(pAdehazeCtx->CurrData.V30.EnvLv, ENVLVMAX, ENVLVMIN);

        //get iso
        if(pAdehazeCtx->FrameNumber == LINEAR_NUM)
            pAdehazeCtx->CurrData.V30.ISO = pAePreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain *
                                            pAePreRes->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain * 50.0;
        else if(pAdehazeCtx->FrameNumber == HDR_2X_NUM || pAdehazeCtx->FrameNumber == HDR_3X_NUM)
            pAdehazeCtx->CurrData.V30.ISO = pAePreRes->ae_pre_res_rk.HdrExp[1].exp_real_params.analog_gain *
                                            pAePreRes->ae_pre_res_rk.HdrExp[1].exp_real_params.digital_gain * 50.0;
    }

    LOG1_ADEHAZE( "%s:exit!\n", __FUNCTION__);
}

void AdehazeGetCurrDataGroup
(
    AdehazeHandle_t*           pAdehazeCtx,
    RKAiqAecExpInfo_t*         pAeEffExpo,
    XCamVideoBuffer*           pAePreRes
) {
    LOG1_ADEHAZE( "%s:enter!\n", __FUNCTION__);

    if(CHECK_ISP_HW_V20()) {
        int iso = 50;
        AdehazeExpInfo_t stExpInfo;
        memset(&stExpInfo, 0x00, sizeof(AdehazeExpInfo_t));

        stExpInfo.hdr_mode = 0;
        for(int i = 0; i < 3; i++) {
            stExpInfo.arIso[i] = 50;
            stExpInfo.arAGain[i] = 1.0;
            stExpInfo.arDGain[i] = 1.0;
            stExpInfo.arTime[i] = 0.01;
        }

        if(pAdehazeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdehazeCtx->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            stExpInfo.hdr_mode = 1;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdehazeCtx->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            stExpInfo.hdr_mode = 2;
        }

        if(pAeEffExpo != NULL) {
            if(pAdehazeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                stExpInfo.arAGain[0] = pAeEffExpo->LinearExp.exp_real_params.analog_gain;
                stExpInfo.arDGain[0] = pAeEffExpo->LinearExp.exp_real_params.digital_gain;
                stExpInfo.arTime[0] = pAeEffExpo->LinearExp.exp_real_params.integration_time;
                stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
            } else {
                for(int i = 0; i < 3; i++) {
                    stExpInfo.arAGain[i] = pAeEffExpo->HdrExp[i].exp_real_params.analog_gain;
                    stExpInfo.arDGain[i] = pAeEffExpo->HdrExp[i].exp_real_params.digital_gain;
                    stExpInfo.arTime[i] = pAeEffExpo->HdrExp[i].exp_real_params.integration_time;
                    stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

                    LOGD_ADEHAZE("index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                                 i,
                                 stExpInfo.arAGain[i],
                                 stExpInfo.arDGain[i],
                                 stExpInfo.arTime[i],
                                 stExpInfo.arIso[i],
                                 stExpInfo.hdr_mode);
                }
            }
        } else {
            LOGE_ADEHAZE("%s:%d pAEPreRes is NULL, so use default instead \n", __FUNCTION__, __LINE__);
        }

        iso = stExpInfo.arIso[stExpInfo.hdr_mode];
        pAdehazeCtx->CurrData.V20.ISO = (float)iso;
    }
    else if(CHECK_ISP_HW_V21()) {
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (pAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)pAePreRes->map(pAePreRes);
            AdehazeGetEnvLvISO(pAdehazeCtx, pAEPreRes);
        }
        else {
            pAdehazeCtx->CurrData.V21.EnvLv = 0.0;
            LOGW_ADEHAZE( "%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (pAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)pAePreRes->map(pAePreRes);
            AdehazeGetEnvLvISO(pAdehazeCtx, pAEPreRes);
        }
        else {
            pAdehazeCtx->CurrData.V30.EnvLv = 0.0;
            LOGE_ADEHAZE( "%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    }

    LOG1_ADEHAZE( "%s:exit!\n", __FUNCTION__);
}

void AdehazeGetCurrData
(
    AdehazeHandle_t*           pAdehazeCtx,
    RkAiqAlgoProcAdhaz*         pProcPara
) {
    LOG1_ADEHAZE( "%s:enter!\n", __FUNCTION__);

    if(CHECK_ISP_HW_V20()) {
        int iso = 50;
        AdehazeExpInfo_t stExpInfo;
        memset(&stExpInfo, 0x00, sizeof(AdehazeExpInfo_t));

        stExpInfo.hdr_mode = 0;
        for(int i = 0; i < 3; i++) {
            stExpInfo.arIso[i] = 50;
            stExpInfo.arAGain[i] = 1.0;
            stExpInfo.arDGain[i] = 1.0;
            stExpInfo.arTime[i] = 0.01;
        }

        if(pAdehazeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            stExpInfo.hdr_mode = 0;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdehazeCtx->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            stExpInfo.hdr_mode = 1;
        } else if(RK_AIQ_HDR_GET_WORKING_MODE(pAdehazeCtx->working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            stExpInfo.hdr_mode = 2;
        }

        XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            if(pAdehazeCtx->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                stExpInfo.arAGain[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.analog_gain;
                stExpInfo.arDGain[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.digital_gain;
                stExpInfo.arTime[0] = pAEPreRes->ae_pre_res_rk.LinearExp.exp_real_params.integration_time;
                stExpInfo.arIso[0] = stExpInfo.arAGain[0] * stExpInfo.arDGain[0] * 50;
            } else {
                for(int i = 0; i < 3; i++) {
                    stExpInfo.arAGain[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.analog_gain;
                    stExpInfo.arDGain[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.digital_gain;
                    stExpInfo.arTime[i] = pAEPreRes->ae_pre_res_rk.HdrExp[i].exp_real_params.integration_time;
                    stExpInfo.arIso[i] = stExpInfo.arAGain[i] * stExpInfo.arDGain[i] * 50;

                    LOGD_ADEHAZE("index:%d again:%f dgain:%f time:%f iso:%d hdr_mode:%d\n",
                                 i,
                                 stExpInfo.arAGain[i],
                                 stExpInfo.arDGain[i],
                                 stExpInfo.arTime[i],
                                 stExpInfo.arIso[i],
                                 stExpInfo.hdr_mode);
                }
            }
        } else {
            LOGE_ADEHAZE("%s:%d pAEPreRes is NULL, so use default instead \n", __FUNCTION__, __LINE__);
        }

        iso = stExpInfo.arIso[stExpInfo.hdr_mode];
        pAdehazeCtx->CurrData.V20.ISO = (float)iso;
    }
    else if(CHECK_ISP_HW_V21()) {
        XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            AdehazeGetEnvLvISO(pAdehazeCtx, pAEPreRes);
        }
        else {
            pAdehazeCtx->CurrData.V21.EnvLv = 0.0;
            pAdehazeCtx->CurrData.V30.ISO = 50.0;
            LOGW_ADEHAZE( "%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        XCamVideoBuffer* xCamAePreRes = pProcPara->com.u.proc.res_comb->ae_pre_res;
        RkAiqAlgoPreResAe* pAEPreRes = NULL;
        if (xCamAePreRes) {
            pAEPreRes = (RkAiqAlgoPreResAe*)xCamAePreRes->map(xCamAePreRes);
            AdehazeGetEnvLvISO(pAdehazeCtx, pAEPreRes);
        }
        else {
            pAdehazeCtx->CurrData.V30.EnvLv = 0.0;
            pAdehazeCtx->CurrData.V30.ISO = 50.0;
            LOGE_ADEHAZE( "%s:PreResBuf is NULL!\n", __FUNCTION__);
        }
    }

    LOG1_ADEHAZE( "%s:exit!\n", __FUNCTION__);
}

XCamReturn AdehazeInit(AdehazeHandle_t** pAdehazeCtx, CamCalibDbV2Context_t* pCalib)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t *handle = (AdehazeHandle_t*)malloc(sizeof(AdehazeHandle_t));
    if (NULL == handle)
        return XCAM_RETURN_ERROR_MEM;
    memset(handle, 0, sizeof(AdehazeHandle_t));

    if(CHECK_ISP_HW_V20())
        handle->HWversion = ADEHAZE_ISP20;
    else if(CHECK_ISP_HW_V21())
        handle->HWversion = ADEHAZE_ISP21;
    else if(CHECK_ISP_HW_V30())
        handle->HWversion = ADEHAZE_ISP30;

    if(handle->HWversion == ADEHAZE_ISP20) {
        //isp20
        CalibDbV2_dehaze_V20_t* calibv2_adehaze_calib_V20 =
            (CalibDbV2_dehaze_V20_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib_v20));
        memcpy(&handle->Calib.Dehaze_v20, calibv2_adehaze_calib_V20, sizeof(CalibDbV2_dehaze_V20_t));

        handle->PreData.V20.ISO = 50.0;
        handle->PreData.V20.ApiMode = DEHAZE_API_BYPASS;
    } else if(handle->HWversion == ADEHAZE_ISP21) {
        //isp21
        CalibDbV2_dehaze_V21_t* calibv2_adehaze_calib_V21 =
            (CalibDbV2_dehaze_V21_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib_v21));
        memcpy(&handle->Calib.Dehaze_v21, calibv2_adehaze_calib_V21, sizeof(CalibDbV2_dehaze_V21_t));

        handle->PreData.V21.EnvLv = 0.0;
        handle->PreData.V21.ApiMode = DEHAZE_API_BYPASS;
    } else if(handle->HWversion == ADEHAZE_ISP30) {
        //isp30
        CalibDbV2_dehaze_V30_t* calibv2_adehaze_calib_V30 =
            (CalibDbV2_dehaze_V30_t*)(CALIBDBV2_GET_MODULE_PTR(pCalib, adehaze_calib_v30));
        memcpy(&handle->Calib.Dehaze_v30.DehazeTuningPara, &calibv2_adehaze_calib_V30->DehazeTuningPara, sizeof(CalibDbDehazeV21_t));

        handle->PreData.V30.EnvLv = 0.0;
        handle->PreData.V30.ApiMode = DEHAZE_API_BYPASS;

        //dehaze local gain
        CalibDbV2_YnrV3_t*  calibv2_Ynr =
            (CalibDbV2_YnrV3_t *)(CALIBDBV2_GET_MODULE_PTR(pCalib, ynr_v3));
        memcpy(&handle->Calib.Dehaze_v30.YnrCalibPara, &calibv2_Ynr->CalibPara, sizeof(CalibDbV2_YnrV3_CalibPara_t));
    }

    //set api default
    handle->AdehazeAtrr.mode = DEHAZE_API_BYPASS;
    handle->AdehazeAtrr.stManual.Enable = true;
    handle->AdehazeAtrr.stManual.cfg_alpha = 1.0;
    handle->AdehazeAtrr.stManual.dehaze_setting.en = false;
    handle->AdehazeAtrr.stManual.dehaze_setting.air_lc_en = true;
    handle->AdehazeAtrr.stManual.dehaze_setting.stab_fnum = 8;
    handle->AdehazeAtrr.stManual.dehaze_setting.sigma = 6;
    handle->AdehazeAtrr.stManual.dehaze_setting.wt_sigma = 8;
    handle->AdehazeAtrr.stManual.dehaze_setting.air_sigma = 120;
    handle->AdehazeAtrr.stManual.dehaze_setting.tmax_sigma = 0.01;
    handle->AdehazeAtrr.stManual.dehaze_setting.pre_wet = 0.01;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.dc_min_th = 64;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.dc_max_th = 192;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.yhist_th = 249;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.yblk_th = 0.002;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.dark_th = 250;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.bright_min = 180;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.bright_max = 240;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.wt_max = 0.9;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.air_min = 200;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.air_max = 250;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.tmax_base = 125;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.tmax_off = 0.1;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.tmax_max = 0.8;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.cfg_wt = 0.8;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.cfg_air = 210;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.cfg_tmax = 0.2;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.dc_weitcur = 1;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.bf_weight = 0.5;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.range_sigma = 0.14;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.space_sigma_pre = 0.14;
    handle->AdehazeAtrr.stManual.dehaze_setting.DehazeData.space_sigma_cur = 0.14;

    handle->AdehazeAtrr.stManual.enhance_setting.en = true;
    handle->AdehazeAtrr.stManual.enhance_setting.EnhanceData.enhance_value = 1.0;
    handle->AdehazeAtrr.stManual.enhance_setting.EnhanceData.enhance_chroma = 1.0;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[0] = 0;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[1] = 64;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[2] = 128;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[3] = 192;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[4] = 256;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[5] = 320;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[6] = 384;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[7] = 448;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[8] = 512;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[9] = 576;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[10] = 640;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[11] = 704;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[12] = 768;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[13] = 832;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[14] = 896;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[15] = 960;
    handle->AdehazeAtrr.stManual.enhance_setting.enhance_curve[16] = 1023;

    handle->AdehazeAtrr.stManual.hist_setting.en = false;
    handle->AdehazeAtrr.stManual.hist_setting.hist_para_en = true;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.hist_gratio = 2;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.hist_th_off = 64;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.hist_k = 2;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.hist_min = 0.015;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.hist_scale = 0.09;
    handle->AdehazeAtrr.stManual.hist_setting.HistData.cfg_gratio = 2;

    handle->AdehazeAtrr.stDehazeManu.level = 50;
    handle->AdehazeAtrr.stEnhanceManu.level = 50;

    *pAdehazeCtx = handle;
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return(ret);
}

XCamReturn AdehazeRelease(AdehazeHandle_t* pAdehazeCtx)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (pAdehazeCtx)
        free(pAdehazeCtx);
    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return(ret);
}

XCamReturn AdehazeProcess(AdehazeHandle_t* pAdehazeCtx, AdehazeVersion_t version)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOG1_ADEHAZE("ENTER: %s \n", __func__);

    float CtrlValue = 0.0;

    if(version == ADEHAZE_ISP20) {
        //todo
    }
    else if(version == ADEHAZE_ISP21) {
        CtrlValue = pAdehazeCtx->CurrData.V21.EnvLv;
        AdehazeProcessV21(pAdehazeCtx, CtrlValue);
    }
    else if(version == ADEHAZE_ISP30) {
        CtrlValue = pAdehazeCtx->CurrData.V30.EnvLv;
        AdehazeProcessV30(pAdehazeCtx, CtrlValue);
    }
    else
        LOGE_ADEHAZE(" %s:Wrong hardware version!! \n", __func__);

    //store pre data
    if(version == ADEHAZE_ISP20)
        pAdehazeCtx->PreData.V20.ISO = pAdehazeCtx->CurrData.V20.ISO;
    else if(version == ADEHAZE_ISP21)
        pAdehazeCtx->PreData.V21.EnvLv = pAdehazeCtx->CurrData.V21.EnvLv;
    else if(version == ADEHAZE_ISP30)
        pAdehazeCtx->PreData.V30.EnvLv = pAdehazeCtx->CurrData.V30.EnvLv;

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

bool AdehazeByPassProcessing(AdehazeHandle_t* pAdehazeCtx)
{
    LOG1_ADEHAZE("ENTER: %s \n", __func__);
    bool ret = false;
    float diff = 0.0;

    if(pAdehazeCtx->FrameID <= 2)
        pAdehazeCtx->byPassProc = false;
    else if(CHECK_ISP_HW_V20()) {
        if(pAdehazeCtx->AdehazeAtrr.mode > DEHAZE_API_BYPASS)
            pAdehazeCtx->byPassProc = false;
        else if(pAdehazeCtx->AdehazeAtrr.mode != pAdehazeCtx->PreData.V20.ApiMode)
            pAdehazeCtx->byPassProc = false;
        else {
            diff = (pAdehazeCtx->PreData.V20.ISO - pAdehazeCtx->CurrData.V20.ISO) / pAdehazeCtx->PreData.V20.ISO;
            if(diff > pAdehazeCtx->Calib.Dehaze_v20.DehazeTuningPara.ByPassThr || diff < (0 - pAdehazeCtx->Calib.Dehaze_v20.DehazeTuningPara.ByPassThr))
                pAdehazeCtx->byPassProc = false;
            else
                pAdehazeCtx->byPassProc = true;
        }
    }
    else if(CHECK_ISP_HW_V21()) {
        if(pAdehazeCtx->AdehazeAtrr.mode > DEHAZE_API_BYPASS)
            pAdehazeCtx->byPassProc = false;
        else if(pAdehazeCtx->AdehazeAtrr.mode != pAdehazeCtx->PreData.V21.ApiMode)
            pAdehazeCtx->byPassProc = false;
        else {
            diff = pAdehazeCtx->PreData.V21.EnvLv - pAdehazeCtx->CurrData.V21.EnvLv;
            if(pAdehazeCtx->PreData.V21.EnvLv == 0.0) {
                diff = pAdehazeCtx->CurrData.V21.EnvLv;
                if(diff == 0.0)
                    pAdehazeCtx->byPassProc = true;
                else
                    pAdehazeCtx->byPassProc = false;
            }
            else {
                diff /= pAdehazeCtx->PreData.V21.EnvLv;
                if(diff >= pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.ByPassThr || diff <= (0 - pAdehazeCtx->Calib.Dehaze_v21.DehazeTuningPara.ByPassThr))
                    pAdehazeCtx->byPassProc = false;
                else
                    pAdehazeCtx->byPassProc = true;

            }
        }
    }
    else if(CHECK_ISP_HW_V30()) {
        if(pAdehazeCtx->AdehazeAtrr.mode > DEHAZE_API_BYPASS)
            pAdehazeCtx->byPassProc = false;
        else if(pAdehazeCtx->AdehazeAtrr.mode != pAdehazeCtx->PreData.V30.ApiMode)
            pAdehazeCtx->byPassProc = false;
        else {
            diff = pAdehazeCtx->PreData.V30.EnvLv - pAdehazeCtx->CurrData.V30.EnvLv;
            if(pAdehazeCtx->PreData.V30.EnvLv == 0.0) {
                diff = pAdehazeCtx->CurrData.V30.EnvLv;
                if(diff == 0.0)
                    pAdehazeCtx->byPassProc = true;
                else
                    pAdehazeCtx->byPassProc = false;
            }
            else {
                diff /= pAdehazeCtx->PreData.V30.EnvLv;
                if(diff >= pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.ByPassThr || diff <= (0 - pAdehazeCtx->Calib.Dehaze_v30.DehazeTuningPara.ByPassThr))
                    pAdehazeCtx->byPassProc = false;
                else
                    pAdehazeCtx->byPassProc = true;
            }
        }
    }
    else
        LOGE_ADEHAZE(" %s:Wrong hardware version!! \n", __func__);

    ret = pAdehazeCtx->byPassProc;

    LOGD_ADEHAZE("%s:FrameID:%d byPassProc:%d ISO:%f\n", __func__, pAdehazeCtx->FrameID, ret, pAdehazeCtx->CurrData.V30.ISO);

    LOG1_ADEHAZE("EXIT: %s \n", __func__);
    return ret;
}

