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

#include "adrc_xml2json.h"

void convertAdrcCalibV1ToCalibV2(CamCalibDbContext_t *calibv1, CamCalibDbV2Context_t *calibv2) {

    const CalibDb_Adrc_Para_t *adrc =
        (CalibDb_Adrc_Para_t*)(CALIBDB_GET_MODULE_PTR((void*)calibv1, adrc));
    if (adrc == NULL)
        return;

    CalibDbV2_drc_t *adrc_v2 =
        (CalibDbV2_drc_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adrc_calib));
    if (adrc_v2 == NULL)
        return;

    //len
    adrc_v2->DrcTuningPara.DrcGain.EnvLv_len = 13;
    adrc_v2->DrcTuningPara.DrcGain.DrcGain_len = 13;
    adrc_v2->DrcTuningPara.DrcGain.Alpha_len = 13;
    adrc_v2->DrcTuningPara.DrcGain.Clip_len = 13;
    adrc_v2->DrcTuningPara.HiLight.EnvLv_len = 13;
    adrc_v2->DrcTuningPara.HiLight.Strength_len = 13;
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv_len = 13;
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit_len = 13;
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast_len = 13;
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast_len = 13;

    //malloc
    adrc_v2->DrcTuningPara.DrcGain.EnvLv = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.DrcGain.DrcGain = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.DrcGain.Alpha = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.DrcGain.Clip = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.HiLight.EnvLv = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.HiLight.Strength = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast = (float *) malloc(sizeof(float) * 13);
    adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast = (float *) malloc(sizeof(float) * 13);

    float EnvLv[13] = { 0, 0.005, 0.01, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0};
    int ManualCurve[17] =  {0, 558, 1087, 1588, 2063, 2515, 2944, 3353, 3744, 4473, 5139, 5751, 6316, 6838, 7322, 7772, 8192};

    //transfer
    if (CHECK_ISP_HW_V21()) {
        adrc_v2->DrcTuningPara.Enable = false;
        for(int i = 0; i < 13; i++) {
            adrc_v2->DrcTuningPara.DrcGain.EnvLv[i] = EnvLv[i];
            adrc_v2->DrcTuningPara.DrcGain.DrcGain[i] = (float)(adrc->calib[0].sw_drc_gain) / 1024.0;
            adrc_v2->DrcTuningPara.DrcGain.Alpha[i] = 0.2;
            adrc_v2->DrcTuningPara.DrcGain.Clip[i] = 16.0;
            adrc_v2->DrcTuningPara.HiLight.EnvLv[i] = EnvLv[i];
            adrc_v2->DrcTuningPara.HiLight.Strength[i] = (float)(adrc->calib[0].sw_drc_weig_maxl) / 16.0;
            adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.EnvLv[i] = EnvLv[i];
            adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LocalWeit[i] = (float)(adrc->calib[0].sw_drc_weig_bilat) / 16.0;
            adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.GlobalContrast[i] = (float)(adrc->calib[0].sw_drc_lpdetail_ratio) / 4096.0;
            adrc_v2->DrcTuningPara.LocalTMOSetting.LocalTMOData.LoLitContrast[i] = (float)(adrc->calib[0].sw_drc_hpdetail_ratio) / 4096.0;
        }
        adrc_v2->DrcTuningPara.LocalTMOSetting.curPixWeit = (float)(adrc->calib[0].sw_drc_weicur_pix) / 255.0;
        adrc_v2->DrcTuningPara.LocalTMOSetting.preFrameWeit = (float)(adrc->calib[0].sw_drc_weipre_frame) / 255.0;
        adrc_v2->DrcTuningPara.LocalTMOSetting.Range_force_sgm = (float)(adrc->calib[0].sw_drc_force_sgm_inv0) / 8191.0;
        adrc_v2->DrcTuningPara.LocalTMOSetting.Range_sgm_cur = (float)(adrc->calib[0].sw_drc_range_sgm_inv1) / 8191.0;
        adrc_v2->DrcTuningPara.LocalTMOSetting.Range_sgm_pre = (float)(adrc->calib[0].sw_drc_range_sgm_inv0) / 8191.0;
        adrc_v2->DrcTuningPara.LocalTMOSetting.Space_sgm_cur = adrc->calib[0].sw_drc_space_sgm_inv1;
        adrc_v2->DrcTuningPara.LocalTMOSetting.Space_sgm_pre = adrc->calib[0].sw_drc_space_sgm_inv0;
        adrc_v2->DrcTuningPara.IIR_frame = adrc->calib[0].sw_drc_iir_frame;
        adrc_v2->DrcTuningPara.OutPutLongFrame = false;
        adrc_v2->DrcTuningPara.Edge_Weit = (float)(adrc->calib[0].sw_drc_edge_scl) / 255.0;
        adrc_v2->DrcTuningPara.Tolerance = 0.0;
        adrc_v2->DrcTuningPara.damp = 0.9;
        adrc_v2->DrcTuningPara.ByPassThr = 0;
        adrc_v2->DrcTuningPara.CompressSetting.Mode = COMPRESS_AUTO;
        for(int i = 0; i < ADRC_Y_NUM; i++) {
            adrc_v2->DrcTuningPara.Scale_y[i] = adrc->calib[0].sw_drc_scale_y[i];
            adrc_v2->DrcTuningPara.CompressSetting.Manual_curve[i] = ManualCurve[i];
        }
    }

}

