/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */
#include "rk_aiq_user_api_adehaze.h"

#include "algo_handlers/RkAiqAdehazeHandle.h"
#include "uAPI2/rk_aiq_user_api2_adehaze.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

/*
void
DehazeTransferSetToolData(CalibDbV2_dehaze_v10_t* DehazeV2, CalibDbDehaze_t *DehazeV1)
{
    //len
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur_len = 9;
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed_len = 9;

    DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len = 9;
    DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len = 9;

    DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale_len = 9;
    DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len = 9;

    //malloc
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO = (float *) malloc(sizeof(float) * 9);
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th = (float *) malloc(sizeof(float)
* 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th = (float *)
malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th = (float *)
malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th = (float *)
malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th = (float *)
malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max = (float
*) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base
= (float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off
= (float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed =
(float *) malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur
= (float *) malloc(sizeof(float) * 9);
    DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur = (float *)
malloc(sizeof(float) * 9); DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed = (float *)
malloc(sizeof(float) * 9);

    DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO = (float *) malloc(sizeof(float) *
9); DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value = (float *)
malloc(sizeof(float) * 9);

    DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO = (float *) malloc(sizeof(float) * 9);
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio = (float *) malloc(sizeof(float) *
9); DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off = (float *) malloc(sizeof(float) *
9); DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k = (float *) malloc(sizeof(float) * 9);
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min = (float *) malloc(sizeof(float) * 9);
    DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale = (float *) malloc(sizeof(float) *
9); DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio = (float *) malloc(sizeof(float) *
9);

    DehazeV2->DehazeTuningPara.Enable = DehazeV1->en ? true : false;
    DehazeV2->DehazeTuningPara.cfg_alpha = DehazeV1->cfg_alpha_normal;
    //dehaze
    DehazeV2->DehazeTuningPara.dehaze_setting.en = DehazeV1->dehaze_setting[0].en ? true : false;
    for(int i = 0; i < 9; i++) {
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO[i] =
DehazeV1->dehaze_setting[0].iso[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[i] =
DehazeV1->dehaze_setting[0].dc_min_th[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[i] =
DehazeV1->dehaze_setting[0].dc_max_th[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[i] =
DehazeV1->dehaze_setting[0].yhist_th[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[i] =
DehazeV1->dehaze_setting[0].yblk_th[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[i] =
DehazeV1->dehaze_setting[0].dark_th[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[i] =
DehazeV1->dehaze_setting[0].bright_max[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[i] =
DehazeV1->dehaze_setting[0].bright_min[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[i] =
DehazeV1->dehaze_setting[0].wt_max[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max[i] =
DehazeV1->dehaze_setting[0].air_max[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min[i] =
DehazeV1->dehaze_setting[0].air_min[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[i] =
DehazeV1->dehaze_setting[0].tmax_base[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[i] =
DehazeV1->dehaze_setting[0].tmax_off[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[i] =
DehazeV1->dehaze_setting[0].tmax_max[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i] =
DehazeV1->dehaze_setting[0].cfg_wt[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i] =
DehazeV1->dehaze_setting[0].cfg_air[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i] =
DehazeV1->dehaze_setting[0].cfg_tmax[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed[i] =
DehazeV1->dehaze_setting[0].dc_thed[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[i] =
DehazeV1->dehaze_setting[0].dc_weitcur[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed[i] =
DehazeV1->dehaze_setting[0].air_thed[i];
        DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur[i] =
DehazeV1->dehaze_setting[0].air_weitcur[i];
    }
    DehazeV2->DehazeTuningPara.dehaze_setting.stab_fnum =
DehazeV1->dehaze_setting[0].IIR_setting.stab_fnum; DehazeV2->DehazeTuningPara.dehaze_setting.sigma =
DehazeV1->dehaze_setting[0].IIR_setting.sigma; DehazeV2->DehazeTuningPara.dehaze_setting.wt_sigma =
DehazeV1->dehaze_setting[0].IIR_setting.wt_sigma;
    DehazeV2->DehazeTuningPara.dehaze_setting.air_sigma =
DehazeV1->dehaze_setting[0].IIR_setting.air_sigma;
    DehazeV2->DehazeTuningPara.dehaze_setting.tmax_sigma =
DehazeV1->dehaze_setting[0].IIR_setting.tmax_sigma;
    //enhance
    DehazeV2->DehazeTuningPara.enhance_setting.en = DehazeV1->enhance_setting[0].en ? true : false;
    for(int i = 0; i < 9; i++) {
        DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO[i] =
DehazeV1->enhance_setting[0].iso[i];
        DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[i] =
DehazeV1->enhance_setting[0].enhance_value[i];
    }
    //hist
    DehazeV2->DehazeTuningPara.hist_setting.en = DehazeV1->hist_setting[0].en ? true : false;
    DehazeV2->DehazeTuningPara.hist_setting.hist_para_en = DehazeV1->hist_setting[0].hist_para_en[0]
? true : false; DehazeV2->DehazeTuningPara.hist_setting.hist_channel =
DehazeV1->hist_setting[0].hist_channel[0] ? true : false; for(int i = 0; i < 9; i++) {
        DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO[i] = DehazeV1->hist_setting[0].iso[i];
        DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio[i] =
DehazeV1->hist_setting[0].hist_gratio[i];
        DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off[i] =
DehazeV1->hist_setting[0].hist_th_off[i]; DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k[i]
= DehazeV1->hist_setting[0].hist_k[i]; DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min[i]
= DehazeV1->hist_setting[0].hist_min[i];
        DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale[i] =
DehazeV1->hist_setting[0].hist_scale[i];
        DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio[i] =
DehazeV1->hist_setting[0].cfg_gratio[i];
    }
}

void
DehazeTransferGetToolData(CalibDbV2_dehaze_v10_t* DehazeV2, CalibDbDehaze_t *DehazeV1)
{

    DehazeV1->en = DehazeV2->DehazeTuningPara.Enable ? 1 : 0;
    DehazeV1->cfg_alpha_normal = DehazeV2->DehazeTuningPara.cfg_alpha;
    DehazeV1->cfg_alpha_hdr = DehazeV2->DehazeTuningPara.cfg_alpha;
    DehazeV1->cfg_alpha_night = DehazeV2->DehazeTuningPara.cfg_alpha;

    //dehaze
    if(DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len >= 9)
        for(int j = 0; j < 5; j++ ) {
            DehazeV1->dehaze_setting[j].en = DehazeV2->DehazeTuningPara.dehaze_setting.en ? 1 : 0;
            for(int i = 0; i < 9; i++) {
                DehazeV1->dehaze_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO[i];
                DehazeV1->dehaze_setting[j].dc_min_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[i] ;
                DehazeV1->dehaze_setting[j].dc_max_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[i];
                DehazeV1->dehaze_setting[j].yhist_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[i];
                DehazeV1->dehaze_setting[j].yblk_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[i];
                DehazeV1->dehaze_setting[j].dark_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[i];
                DehazeV1->dehaze_setting[j].bright_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[i];
                DehazeV1->dehaze_setting[j].bright_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[i];
                DehazeV1->dehaze_setting[j].wt_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[i];
                DehazeV1->dehaze_setting[j].air_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max[i];
                DehazeV1->dehaze_setting[j].air_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min[i];
                DehazeV1->dehaze_setting[j].tmax_base[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[i];
                DehazeV1->dehaze_setting[j].tmax_off[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[i];
                DehazeV1->dehaze_setting[j].tmax_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[i];
                DehazeV1->dehaze_setting[j].cfg_wt[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i];
                DehazeV1->dehaze_setting[j].cfg_air[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i];
                DehazeV1->dehaze_setting[j].cfg_tmax[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i];
                DehazeV1->dehaze_setting[j].dc_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed[i];
                DehazeV1->dehaze_setting[j].dc_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[i];
                DehazeV1->dehaze_setting[j].air_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed[i];
                DehazeV1->dehaze_setting[j].air_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur[i];
            }
            DehazeV1->dehaze_setting[j].IIR_setting.stab_fnum =
DehazeV2->DehazeTuningPara.dehaze_setting.stab_fnum; DehazeV1->dehaze_setting[j].IIR_setting.sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.sigma; DehazeV1->dehaze_setting[j].IIR_setting.wt_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.wt_sigma;
            DehazeV1->dehaze_setting[j].IIR_setting.air_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.air_sigma ;
            DehazeV1->dehaze_setting[j].IIR_setting.tmax_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.tmax_sigma;
        }
    else if(DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len >= 1 &&
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len < 9) { for(int j = 0; j < 5; j++ ) {
            //dehaze
            DehazeV1->dehaze_setting[j].en = DehazeV2->DehazeTuningPara.dehaze_setting.en ? 1 : 0;
            for(int i = 0; i < DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len; i++) {
                DehazeV1->dehaze_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO[i];
                DehazeV1->dehaze_setting[j].dc_min_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[i] ;
                DehazeV1->dehaze_setting[j].dc_max_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[i];
                DehazeV1->dehaze_setting[j].yhist_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[i];
                DehazeV1->dehaze_setting[j].yblk_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[i];
                DehazeV1->dehaze_setting[j].dark_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[i];
                DehazeV1->dehaze_setting[j].bright_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[i];
                DehazeV1->dehaze_setting[j].bright_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[i];
                DehazeV1->dehaze_setting[j].wt_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[i];
                DehazeV1->dehaze_setting[j].air_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max[i];
                DehazeV1->dehaze_setting[j].air_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min[i];
                DehazeV1->dehaze_setting[j].tmax_base[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[i];
                DehazeV1->dehaze_setting[j].tmax_off[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[i];
                DehazeV1->dehaze_setting[j].tmax_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[i];
                DehazeV1->dehaze_setting[j].cfg_wt[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i];
                DehazeV1->dehaze_setting[j].cfg_air[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i];
                DehazeV1->dehaze_setting[j].cfg_tmax[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i];
                DehazeV1->dehaze_setting[j].dc_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed[i];
                DehazeV1->dehaze_setting[j].dc_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[i];
                DehazeV1->dehaze_setting[j].air_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed[i];
                DehazeV1->dehaze_setting[j].air_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur[i];
            }
            DehazeV1->dehaze_setting[j].IIR_setting.stab_fnum =
DehazeV2->DehazeTuningPara.dehaze_setting.stab_fnum; DehazeV1->dehaze_setting[j].IIR_setting.sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.sigma; DehazeV1->dehaze_setting[j].IIR_setting.wt_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.wt_sigma;
            DehazeV1->dehaze_setting[j].IIR_setting.air_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.air_sigma ;
            DehazeV1->dehaze_setting[j].IIR_setting.tmax_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.tmax_sigma;
        }
        int k = DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len - 1;
        for(int j = 0; j < 5; j++ ) {
            //dehaze
            DehazeV1->dehaze_setting[j].en = DehazeV2->DehazeTuningPara.dehaze_setting.en ? 1 : 0;
            for(int i = DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len; i < 9; i++) {
                DehazeV1->dehaze_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.ISO[k];
                DehazeV1->dehaze_setting[j].dc_min_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[k] ;
                DehazeV1->dehaze_setting[j].dc_max_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[k];
                DehazeV1->dehaze_setting[j].yhist_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[k];
                DehazeV1->dehaze_setting[j].yblk_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[k];
                DehazeV1->dehaze_setting[j].dark_th[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[k];
                DehazeV1->dehaze_setting[j].bright_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[k];
                DehazeV1->dehaze_setting[j].bright_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[k];
                DehazeV1->dehaze_setting[j].wt_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[k];
                DehazeV1->dehaze_setting[j].air_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_max[k];
                DehazeV1->dehaze_setting[j].air_min[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_min[k];
                DehazeV1->dehaze_setting[j].tmax_base[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[k];
                DehazeV1->dehaze_setting[j].tmax_off[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[k];
                DehazeV1->dehaze_setting[j].tmax_max[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[k];
                DehazeV1->dehaze_setting[j].cfg_wt[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[k];
                DehazeV1->dehaze_setting[j].cfg_air[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[k];
                DehazeV1->dehaze_setting[j].cfg_tmax[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[k];
                DehazeV1->dehaze_setting[j].dc_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed[k];
                DehazeV1->dehaze_setting[j].dc_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[k];
                DehazeV1->dehaze_setting[j].air_thed[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_thed[k];
                DehazeV1->dehaze_setting[j].air_weitcur[i] =
DehazeV2->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur[k];
            }
            DehazeV1->dehaze_setting[j].IIR_setting.stab_fnum =
DehazeV2->DehazeTuningPara.dehaze_setting.stab_fnum; DehazeV1->dehaze_setting[j].IIR_setting.sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.sigma; DehazeV1->dehaze_setting[j].IIR_setting.wt_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.wt_sigma;
            DehazeV1->dehaze_setting[j].IIR_setting.air_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.air_sigma ;
            DehazeV1->dehaze_setting[j].IIR_setting.tmax_sigma =
DehazeV2->DehazeTuningPara.dehaze_setting.tmax_sigma;
        }
    }

    //enhance
    if(DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len >= 9)
        for(int j = 0; j < 5; j++ ) {
            DehazeV1->enhance_setting[j].en = DehazeV2->DehazeTuningPara.enhance_setting.en ? 1 : 0;
            for(int i = 0; i < 9; i++) {
                DehazeV1->enhance_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO[i];
                DehazeV1->enhance_setting[j].enhance_value[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[i];
            }
        }
    else if(DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len >= 1 &&
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len < 9) { for(int j = 0; j < 5; j++ ) {
            DehazeV1->enhance_setting[j].en = DehazeV2->DehazeTuningPara.enhance_setting.en ? 1 : 0;
            for(int i = 0; i < DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len; i++)
{ DehazeV1->enhance_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO[i];
                DehazeV1->enhance_setting[j].enhance_value[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[i];
            }
        }
        int k = DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len - 1;
        for(int j = 0; j < 5; j++ ) {
            DehazeV1->enhance_setting[j].en = DehazeV2->DehazeTuningPara.enhance_setting.en ? 1 : 0;
            for(int i = DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len; i < 9; i++)
{ DehazeV1->enhance_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.ISO[k];
                DehazeV1->enhance_setting[j].enhance_value[i] =
DehazeV2->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[k];
            }
        }
    }

    //hist
    if(DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len >= 9)
        for(int j = 0; j < 5; j++ ) {
            DehazeV1->hist_setting[j].en = DehazeV2->DehazeTuningPara.hist_setting.en ? 1 : 0;
            for(int i = 0; i < 9; i++) {
                DehazeV1->hist_setting[j].hist_para_en[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_para_en ? 1 : 0;
                DehazeV1->hist_setting[j].hist_channel[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_channel ? 1 : 0; DehazeV1->hist_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO[i]; DehazeV1->hist_setting[j].hist_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio[i];
                DehazeV1->hist_setting[j].hist_th_off[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off[i]; DehazeV1->hist_setting[j].hist_k[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k[i]; DehazeV1->hist_setting[j].hist_min[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min[i];
                DehazeV1->hist_setting[j].hist_scale[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale[i];
                DehazeV1->hist_setting[j].cfg_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio[i];
            }
        }
    else if(DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len >= 1 &&
DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len < 9) { for(int j = 0; j < 5; j++ ) {
            DehazeV1->hist_setting[j].en = DehazeV2->DehazeTuningPara.hist_setting.en ? 1 : 0;
            for(int i = 0; i < DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len; i++) {
                DehazeV1->hist_setting[j].hist_para_en[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_para_en  ? 1 : 0;
                DehazeV1->hist_setting[j].hist_channel[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_channel ? 1 : 0; DehazeV1->hist_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO[i]; DehazeV1->hist_setting[j].hist_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio[i];
                DehazeV1->hist_setting[j].hist_th_off[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off[i]; DehazeV1->hist_setting[j].hist_k[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k[i]; DehazeV1->hist_setting[j].hist_min[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min[i];
                DehazeV1->hist_setting[j].hist_scale[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale[i];
                DehazeV1->hist_setting[j].cfg_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio[i];
            }
        }
        int k = DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len - 1;
        for(int j = 0; j < 5; j++ ) {
            DehazeV1->hist_setting[j].en = DehazeV2->DehazeTuningPara.hist_setting.en ? 1 : 0;
            for(int i = DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO_len; i < 9; i++) {
                DehazeV1->hist_setting[j].hist_para_en[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_para_en  ? 1 : 0;
                DehazeV1->hist_setting[j].hist_channel[i] =
DehazeV2->DehazeTuningPara.hist_setting.hist_channel ? 1 : 0; DehazeV1->hist_setting[j].iso[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.ISO[k]; DehazeV1->hist_setting[j].hist_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_gratio[k];
                DehazeV1->hist_setting[j].hist_th_off[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_th_off[k]; DehazeV1->hist_setting[j].hist_k[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_k[k]; DehazeV1->hist_setting[j].hist_min[i]
= DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_min[k];
                DehazeV1->hist_setting[j].hist_scale[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.hist_scale[k];
                DehazeV1->hist_setting[j].cfg_gratio[i] =
DehazeV2->DehazeTuningPara.hist_setting.HistData.cfg_gratio[k];
            }
        }
    }

}

void
DehazeTransferSetData(adehaze_sw_V2_t* DehazeV2, adehaze_sw_t *DehazeV1)
{
    //HWversion
    DehazeV2->HWversion = ADEHAZE_ISP20;

    //op mode
    if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_INVALID)
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_AUTO;
    else if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_MANUAL)
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_MANUAL;
    else if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_AUTO)
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_AUTO;
    else if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_OFF)
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_OFF;
    else if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_TOOL)
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_TOOL;
    else
        DehazeV2->AdehazeAtrrV20.mode = DEHAZE_API_AUTO;

    //st manual
    memcpy(&DehazeV2->AdehazeAtrrV20.stManual, &DehazeV1->stManual,
sizeof(rk_aiq_dehaze_M_attrib_t));

    //st enhance
    memcpy(&DehazeV2->AdehazeAtrrV20.stEnhanceManual, &DehazeV1->stEnhance,
sizeof(rk_aiq_dehaze_enhance_t));

    // st auto
    if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_AUTO)
        DehazeTransferSetToolData(&DehazeV2->AdehazeAtrrV20.stTool, &DehazeV1->stAuto);
    if(DehazeV1->mode == RK_AIQ_DEHAZE_MODE_TOOL)
        DehazeTransferSetToolData(&DehazeV2->AdehazeAtrrV20.stTool, &DehazeV1->stTool);
}

void
DehazeTransferGetData(adehaze_sw_V2_t* DehazeV2, adehaze_sw_t *DehazeV1)
{
    //bypass
    if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_AUTO)
        DehazeV1->byPass = true;
    else
        DehazeV1->byPass = false;

    //op mode
    if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_AUTO)
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_INVALID;
    else if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_MANUAL)
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_MANUAL;
    else if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_AUTO)
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_AUTO;
    else if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_OFF)
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_OFF;
    else if(DehazeV2->AdehazeAtrrV20.mode == DEHAZE_API_TOOL)
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_TOOL;
    else
        DehazeV1->mode = RK_AIQ_DEHAZE_MODE_INVALID;

    //st manual
    memcpy(&DehazeV1->stManual, &DehazeV2->AdehazeAtrrV20.stManual,
sizeof(rk_aiq_dehaze_M_attrib_t));

    //st enhance
    memcpy(&DehazeV1->stEnhance, &DehazeV2->AdehazeAtrrV20.stEnhanceManual,
sizeof(rk_aiq_dehaze_enhance_t));

    // st auto
    DehazeTransferGetToolData(&DehazeV2->AdehazeAtrrV20.stTool, &DehazeV1->stAuto);

    //st tool
    DehazeTransferGetToolData(&DehazeV2->AdehazeAtrrV20.stTool, &DehazeV1->stTool);
}

XCamReturn  rk_aiq_user_api_adehaze_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t attr)
{
    adehaze_sw_V2_t dehazeAttr;
    memset(&dehazeAttr, 0, sizeof(adehaze_sw_V2_t));

    DehazeTransferSetData(&dehazeAttr, &attr);

    XCamReturn ret_dehaze = rk_aiq_user_api2_adehaze_setSwAttrib(sys_ctx, dehazeAttr);

    return ret_dehaze;
}

XCamReturn  rk_aiq_user_api_adehaze_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t *attr)
{
    adehaze_sw_V2_t dehazeAttr;
    memset(&dehazeAttr, 0, sizeof(adehaze_sw_V2_t));

    DehazeTransferGetData(&dehazeAttr, attr);

    XCamReturn ret_dehaze = rk_aiq_user_api2_adehaze_getSwAttrib(sys_ctx, &dehazeAttr);

    return ret_dehaze;
}
*/

RKAIQ_END_DECLARE
