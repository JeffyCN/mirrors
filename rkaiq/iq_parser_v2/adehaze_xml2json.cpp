/*
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

#include "adehaze_xml2json.h"

void convertDehazeCalib2CalibV2(CamCalibDbContext_t *calibv1, CamCalibDbV2Context_t *calibv2) {

    //len
    CalibDbV2_dehaze_v10_t* calibv2_adehaze_calib_V20 =
        (CalibDbV2_dehaze_v10_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adehaze_calib));
    if (calibv2_adehaze_calib_V20) {
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len = 9;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len = 9;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len = 9;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_thed_len = calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO_len;

        calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len = 9;
        calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len = calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.ISO_len;

        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len = 9;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_gratio_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_th_off_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_k_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_min_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_scale_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len = calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO_len;
    }

    CalibDbV2_dehaze_v11_t* calibv2_adehaze_calib_V21 =
        (CalibDbV2_dehaze_v11_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adehaze_calib));
    if (calibv2_adehaze_calib_V21) {
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len = 9;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dark_th_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_max_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_min_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.wt_max_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_max_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_min_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur_len = calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv_len;

        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len = 9;
        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value_len = calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma_len = calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv_len;

        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len = 9;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_gratio_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_th_off_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_k_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_min_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_scale_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.cfg_gratio_len = calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv_len;
    }

    //malloc
    if (calibv2_adehaze_calib_V20) {
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dark_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.wt_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_thed = (float *) malloc(sizeof(float) * 9);

        calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.ISO = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value = (float *) malloc(sizeof(float) * 9);

        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_gratio = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_th_off = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_k = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_scale = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.cfg_gratio = (float *) malloc(sizeof(float) * 9);
    }

    if (calibv2_adehaze_calib_V21) {
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dark_th = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.wt_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur = (float *) malloc(sizeof(float) * 9);

        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma = (float *) malloc(sizeof(float) * 9);

        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_gratio = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_th_off = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_k = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_min = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_scale = (float *) malloc(sizeof(float) * 9);
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.cfg_gratio = (float *) malloc(sizeof(float) * 9);
    }

    //dehaze v20
    if (CHECK_ISP_HW_V20()) {
        CalibDb_Dehaze_t* calibv1_dehaze_isp20 =
            (CalibDb_Dehaze_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, dehaze);
        calibv2_adehaze_calib_V20->DehazeTuningPara.Enable = calibv1_dehaze_isp20->calib_v20[0].en ? true : false;
        calibv2_adehaze_calib_V20->DehazeTuningPara.cfg_alpha = calibv1_dehaze_isp20->calib_v20[0].cfg_alpha;
        calibv2_adehaze_calib_V20->DehazeTuningPara.ByPassThr = 0;
        //dehaze
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.en = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.en ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.ISO[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.iso[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.dc_min_th[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.dc_max_th[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.yhist_th[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.yblk_th[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.dark_th[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.bright_max[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.bright_min[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.wt_max[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_max[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.air_max[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_min[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.air_min[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.tmax_base[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.tmax_off[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.tmax_max[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.cfg_wt[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.cfg_air[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.cfg_tmax[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_thed[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.dc_thed[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.dc_weitcur[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_thed[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.air_thed[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.DehazeData.air_weitcur[i] = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.air_weitcur[i];
        }
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.stab_fnum = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.IIR_setting.stab_fnum;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.sigma = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.IIR_setting.sigma;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.wt_sigma = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.IIR_setting.wt_sigma;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.air_sigma = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.IIR_setting.air_sigma;
        calibv2_adehaze_calib_V20->DehazeTuningPara.dehaze_setting.tmax_sigma = calibv1_dehaze_isp20->calib_v20[0].dehaze_setting.IIR_setting.tmax_sigma;
        //enhance
        calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.en = calibv1_dehaze_isp20->calib_v20[0].enhance_setting.en ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.ISO[i] = calibv1_dehaze_isp20->calib_v20[0].enhance_setting.iso[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[i] = calibv1_dehaze_isp20->calib_v20[0].enhance_setting.enhance_value[i];
        }
        //hist
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.en = calibv1_dehaze_isp20->calib_v20[0].hist_setting.en ? true : false;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.hist_para_en = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_para_en[0] ? true : false;
        calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.hist_channel = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_channel[0] ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.ISO[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.iso[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_gratio[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_gratio[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_th_off[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_th_off[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_k[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_k[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_min[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_min[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.hist_scale[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.hist_scale[i];
            calibv2_adehaze_calib_V20->DehazeTuningPara.hist_setting.HistData.cfg_gratio[i] = calibv1_dehaze_isp20->calib_v20[0].hist_setting.cfg_gratio[i];
        }
    }

    //dehaze v21
    if (CHECK_ISP_HW_V21()) {
        CalibDb_Dehaze_Isp21_t* calibv1_dehaze_isp21 =
            (CalibDb_Dehaze_Isp21_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, dehaze);
        float EnvLv[9] = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.7, 0.9, 1};
        calibv2_adehaze_calib_V21->DehazeTuningPara.Enable = calibv1_dehaze_isp21->calib_v21[0].en ? true : false;
        calibv2_adehaze_calib_V21->DehazeTuningPara.cfg_alpha = calibv1_dehaze_isp21->calib_v21[0].cfg_alpha;
        calibv2_adehaze_calib_V21->DehazeTuningPara.ByPassThr = 0;
        //dehaze
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.en = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.en ? true : false;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.air_lc_en = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.air_lc_en[0] ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.EnvLv[i] = EnvLv[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_min_th[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.dc_min_th[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_max_th[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.dc_max_th[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yhist_th[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.yhist_th[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.yblk_th[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.yblk_th[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dark_th[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.dark_th[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_max[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.bright_max[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bright_min[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.bright_min[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.wt_max[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.wt_max[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_max[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.air_max[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.air_min[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.air_min[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_base[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.tmax_base[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_off[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.tmax_off[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.tmax_max[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.tmax_max[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_wt[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.cfg_wt[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_air[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.cfg_air[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.cfg_tmax[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.cfg_tmax[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.dc_weitcur[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.dc_weitcur[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.bf_weight[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.bf_weight[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.range_sigma[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.range_sigma[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_pre[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.space_sigma_pre[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.DehazeData.space_sigma_cur[i] = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.space_sigma_cur[i];
        }
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.stab_fnum = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.stab_fnum;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.sigma = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.sigma;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.wt_sigma = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.wt_sigma;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.air_sigma = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.air_sigma;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.tmax_sigma = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.tmax_sigma;
        calibv2_adehaze_calib_V21->DehazeTuningPara.dehaze_setting.pre_wet = calibv1_dehaze_isp21->calib_v21[0].dehaze_setting.IIR_setting.pre_wet;
        //enhance
        calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.en = calibv1_dehaze_isp21->calib_v21[0].enhance_setting.en ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.EnvLv[i] = EnvLv[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_value[i] = calibv1_dehaze_isp21->calib_v21[0].enhance_setting.enhance_value[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.EnhanceData.enhance_chroma[i] = calibv1_dehaze_isp21->calib_v21[0].enhance_setting.enhance_chroma[i];
        }
        for(int i = 0; i < 17; i++)
            calibv2_adehaze_calib_V21->DehazeTuningPara.enhance_setting.enhance_curve[i] = calibv1_dehaze_isp21->calib_v21[0].enhance_setting.enhance_curve[i];
        //hist
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.en = calibv1_dehaze_isp21->calib_v21[0].hist_setting.en ? true : false;
        calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.hist_para_en = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_para_en[0] ? true : false;
        for(int i = 0; i < 9; i++) {
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.EnvLv[i] = EnvLv[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_gratio[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_gratio[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_th_off[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_th_off[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_k[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_k[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_min[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_min[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.hist_scale[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.hist_scale[i];
            calibv2_adehaze_calib_V21->DehazeTuningPara.hist_setting.HistData.cfg_gratio[i] = calibv1_dehaze_isp21->calib_v21[0].hist_setting.cfg_gratio[i];
        }
    }

}

