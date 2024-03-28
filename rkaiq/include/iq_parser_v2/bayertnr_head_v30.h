/*
 * bayernr_head_v1.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef __CALIBDBV2_BAYER_TNR_HEADER_V30_H__
#define __CALIBDBV2_BAYER_TNR_HEADER_V30_H__

#include "rk_aiq_comm.h"

// clang-format off

RKAIQ_BEGIN_DECLARE

////////////////////////bayernr V30//////////////////////////////////////
typedef struct CalibDbV2_BayerTnrV30_C_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;

    // M4_ARRAY_DESC("bayertnr_tnr_luma2sigma_idx", "s32", M4_SIZE(1,20), M4_RANGE(0,4095), "[43, 86, 129, 172, 215, 258, 301, 344, 387, 430, 473, 516, 559, 602, 645, 688, 731, 774, 817, 860]", M4_DIGIT(0), M4_DYNAMIC(0))
    int bayertnr_tnr_luma2sigma_idx[20];

    // M4_ARRAY_DESC("bayertnr_tnr_luma2sigma_val", "s32", M4_SIZE(1,20), M4_RANGE(0,4095), "256", M4_DIGIT(0), M4_DYNAMIC(0))
    int bayertnr_tnr_luma2sigma_val[20];

    // M4_ARRAY_DESC("bayertnr_spnr_luma2sigma_idx", "s32", M4_SIZE(1,16), M4_RANGE(0,4095), "[52, 104, 156, 208, 260, 312, 364, 416, 468, 520, 572, 624, 676, 728, 780, 832]", M4_DIGIT(0), M4_DYNAMIC(0))
    int bayertnr_spnr_luma2sigma_idx[16];

    // M4_ARRAY_DESC("bayertnr_spnr_curluma2sigma_val", "s32", M4_SIZE(1,16), M4_RANGE(0,4095), "128", M4_DIGIT(0), M4_DYNAMIC(0))
    int bayertnr_spnr_curluma2sigma_val[16];

    // M4_ARRAY_DESC("bayertnr_spnr_preluma2sigma_val", "s32", M4_SIZE(1,16), M4_RANGE(0,4095), "128", M4_DIGIT(0), M4_DYNAMIC(0))
    int bayertnr_spnr_preluma2sigma_val[16];

} CalibDbV2_BayerTnrV30_C_ISO_t;

typedef struct CalibDbV2_BayerTnrV30_C_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char *SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char *Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Calib_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_BayerTnrV30_C_ISO_t *Calib_ISO;
    int Calib_ISO_len;

} CalibDbV2_BayerTnrV30_C_Set_t;

typedef struct CalibDbV2_BayerTnrV30_Calib_s {
    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_BayerTnrV30_C_Set_t *Setting;
    int Setting_len;
} CalibDbV2_BayerTnrV30_Calib_t;


typedef struct CalibDbV2_BayerTnrV30_T_ISO_s {
    // M4_NUMBER_MARK_DESC("iso", "f32", M4_RANGE(50, 204800), "50", M4_DIGIT(1), "index2")
    float iso;

#if 0
    // M4_BOOL_DESC("hw_btnr_tnr_en", "1")
    bool hw_btnr_tnr_en;
#endif

    // M4_BOOL_DESC("hw_btnr_curSpnr_en", "1")
    bool hw_btnr_curSpnr_en;
    // M4_BOOL_DESC("hw_btnr_preSpnr_en", "1")
    bool hw_btnr_preSpnr_en;

    // M4_NUMBER_DESC("sw_btnr_tnrFilt_strg", "f32", M4_RANGE(0, 16), "1.0", M4_DIGIT(2))
    float sw_btnr_tnrFilt_strg;
    // M4_NUMBER_DESC("sw_btnr_curSpnrFilt_strg", "f32", M4_RANGE(0, 16), "1.0", M4_DIGIT(2))
    float sw_btnr_curSpnrFilt_strg;
    // M4_NUMBER_DESC("sw_btnr_preSpnrFilt_strg", "f32", M4_RANGE(0, 16), "1.0", M4_DIGIT(2))
    float sw_btnr_preSpnrFilt_strg;


    // M4_BOOL_DESC("hw_btnr_curSpnrSigmaIdxFilt_en", "1")
    bool hw_btnr_curSpnrSigmaIdxFilt_en;
#if 0
    // M4_NUMBER_DESC("hw_btnr_curSpnrSigmaIdxFilt_mode", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    uint8_t hw_btnr_curSpnrSigmaIdxFilt_mode;
#endif
    // M4_BOOL_DESC("hw_btnr_preSpnrSigmaIdxFilt_en", "1")
    bool hw_btnr_preSpnrSigmaIdxFilt_en;
#if 0
    // M4_NUMBER_DESC("hw_btnr_preSpnrSigmaIdxFilt_mode", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    uint8_t hw_btnr_preSpnrSigmaIdxFilt_mode;
#endif
    // M4_NUMBER_DESC("hw_btnr_curSpnr_wgt", "f32", M4_RANGE(0, 1.0), "1.0", M4_DIGIT(2))
    float hw_btnr_curSpnr_wgt;
    // M4_NUMBER_DESC("hw_btnr_preSpnr_wgt", "f32", M4_RANGE(0, 1.0), "1.0", M4_DIGIT(2))
    float hw_btnr_preSpnr_wgt;
    // M4_NUMBER_DESC("hw_btnr_curSpnrWgtCal_scale", "f32", M4_RANGE(0, 16.0), "0.15", M4_DIGIT(2))
    float hw_btnr_curSpnrWgtCal_scale;
    // M4_NUMBER_DESC("hw_btnr_curSpnrWgtCal_offset", "f32", M4_RANGE(0, 4.0), "0.0", M4_DIGIT(2))
    float hw_btnr_curSpnrWgtCal_offset;
    // M4_NUMBER_DESC("hw_btnr_preSpnrWgtCal_scale", "f32", M4_RANGE(0, 16.0), "0.15", M4_DIGIT(2))
    float hw_btnr_preSpnrWgtCal_scale;
    // M4_NUMBER_DESC("hw_btnr_preSpnrWgtCal_offset", "f32", M4_RANGE(0, 4.0), "0.0", M4_DIGIT(2))
    float hw_btnr_preSpnrWgtCal_offset;
    // M4_BOOL_DESC("hw_btnr_spnrPresigmaUse_en", "1")
    bool hw_btnr_spnrPresigmaUse_en;
    // M4_NUMBER_DESC("hw_btnr_curSpnrSigma_scale", "f32", M4_RANGE(0, 256.0), "1.0", M4_DIGIT(2))
    float hw_btnr_curSpnrSigma_scale;
    // M4_NUMBER_DESC("hw_btnr_curSpnrSigma_offset", "f32", M4_RANGE(0, 16.0), "0.0", M4_DIGIT(2))
    float hw_btnr_curSpnrSigma_offset;
    // M4_NUMBER_DESC("hw_btnr_preSpnrSigma_scale", "f32", M4_RANGE(0, 256.0), "1.0", M4_DIGIT(2))
    float hw_btnr_preSpnrSigma_scale;
    // M4_NUMBER_DESC("hw_btnr_preSpnrSigma_offset", "f32", M4_RANGE(0, 16.0), "0.0", M4_DIGIT(2))
    float hw_btnr_preSpnrSigma_offset;
    // M4_NUMBER_DESC("hw_btnr_curSpnrSigmaHdrS_scale", "f32", M4_RANGE(0, 256.0), "1.0", M4_DIGIT(2))
    float hw_btnr_curSpnrSigmaHdrS_scale;
    // M4_NUMBER_DESC("hw_btnr_curSpnrSigmaHdrS_offset", "f32", M4_RANGE(0, 16.0), "0.0", M4_DIGIT(2))
    float hw_btnr_curSpnrSigmaHdrS_offset;
    // M4_NUMBER_DESC("hw_btnr_preSpnrSigmaHdrS_scale", "f32", M4_RANGE(0, 256.0), "1.0", M4_DIGIT(2))
    float hw_btnr_preSpnrSigmaHdrS_scale;
    // M4_NUMBER_DESC("hw_btnr_preSpnrSigmaHdrS_offset", "f32", M4_RANGE(0, 16.0), "0.0", M4_DIGIT(2))
    float hw_btnr_preSpnrSigmaHdrS_offset;


    // M4_BOOL_DESC("hw_btnr_transf_en", "0")
    bool hw_btnr_transf_en;
    // M4_NUMBER_DESC("hw_btnr_transf_mode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    uint8_t hw_btnr_transf_mode;
    // M4_NUMBER_DESC("hw_btnr_transfMode0_scale", "s32", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    int hw_btnr_transfMode0_scale;
    // M4_NUMBER_DESC("sw_btnr_transfMode0_offset", "s32", M4_RANGE(0, 8191), "256", M4_DIGIT(0))
    int sw_btnr_transfMode0_offset;
    // M4_NUMBER_DESC("sw_btnr_itransfMode0_offset", "s32", M4_RANGE(0, 65535), "2048", M4_DIGIT(0))
    int sw_btnr_itransfMode0_offset;
    // M4_NUMBER_DESC("sw_btnr_transfMode1_offset", "s32", M4_RANGE(0, 8191), "256", M4_DIGIT(0))
    int sw_btnr_transfMode1_offset;
    // M4_NUMBER_DESC("sw_btnr_itransfMode1_offset", "s32", M4_RANGE(0, 65535), "1024", M4_DIGIT(0))
    int sw_btnr_itransfMode1_offset;
    // M4_NUMBER_DESC("hw_btnr_transfData_maxLimit", "s32", M4_RANGE(0, 1048575), "1048575", M4_DIGIT(0))
    int hw_btnr_transfData_maxLimit;


    // M4_BOOL_DESC("hw_btnr_gKalman_en", "0")
    bool hw_btnr_gKalman_en;
    // M4_NUMBER_DESC("hw_btnr_gKalman_wgt", "s32", M4_RANGE(0, 2048), "1024", M4_DIGIT(0))
    int hw_btnr_gKalman_wgt;


    // M4_NUMBER_DESC("hw_btnr_wgtCal_mode", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    uint8_t hw_btnr_wgtCal_mode;
    // M4_BOOL_DESC("hw_btnr_lpfHi_en", "1")
    bool hw_btnr_lpfHi_en;
    // M4_NUMBER_DESC("sw_btnr_lpfHi_sigma", "f32", M4_RANGE(0, 100.0), "1.0", M4_DIGIT(2))
    float sw_btnr_lpfHi_sigma;
    // M4_BOOL_DESC("hw_btnr_lpfLo_en", "1")
    bool hw_btnr_lpfLo_en;
    // M4_NUMBER_DESC("sw_btnr_lpfLo_sigma", "f32", M4_RANGE(0, 100.0), "1.0", M4_DIGIT(2))
    float sw_btnr_lpfLo_sigma;
    // M4_BOOL_DESC("hw_btnr_sigmaIdxFilt_en", "1")
    bool hw_btnr_sigmaIdxFilt_en;
    // M4_NUMBER_DESC("hw_btnr_sigmaIdxFilt_mode", "u8", M4_RANGE(0, 1), "1", M4_DIGIT(0))
    uint8_t hw_btnr_sigmaIdxFilt_mode;//not use yet
    // M4_NUMBER_DESC("hw_btnr_sigma_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_sigma_scale;
    // M4_BOOL_DESC("hw_btnr_wgtFilt_en", "1")
    bool hw_btnr_wgtFilt_en;
    // M4_NUMBER_DESC("hw_btnr_mode0LoWgt_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode0LoWgt_scale;
    // M4_NUMBER_DESC("hw_btnr_mode0Base_ratio", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode0Base_ratio;
    // M4_NUMBER_DESC("hw_btnr_mode1LoWgt_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode1LoWgt_scale;
    // M4_NUMBER_DESC("hw_btnr_mode1LoWgt_offset", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode1LoWgt_offset;
    // M4_NUMBER_DESC("hw_btnr_mode1Wgt_offset", "f32", M4_RANGE(0, 4.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode1Wgt_offset;
    // M4_NUMBER_DESC("hw_btnr_mode1Wgt_minLimit", "f32", M4_RANGE(0, 1.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode1Wgt_minLimit;
    // M4_NUMBER_DESC("hw_btnr_mode1Wgt_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode1Wgt_scale;




    // M4_BOOL_DESC("hw_btnr_loDetection_en", "1")
    bool hw_btnr_loDetection_en;
    // M4_BOOL_DESC("hw_btnr_loDiffVfilt_en", "1")
    bool hw_btnr_loDiffVfilt_en;
    // M4_NUMBER_DESC("hw_btnr_loDiffVfilt_mode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    uint8_t hw_btnr_loDiffVfilt_mode;//not use yet
    // M4_BOOL_DESC("hw_btnr_loDiffHfilt_en", "0")
    bool hw_btnr_loDiffHfilt_en;
    // M4_NUMBER_DESC("hw_btnr_loDiffWgtCal_scale", "f32", M4_RANGE(0, 16.0), "2.0", M4_DIGIT(2))
    float hw_btnr_loDiffWgtCal_scale;
    // M4_NUMBER_DESC("hw_btnr_loDiffWgtCal_offset", "f32", M4_RANGE(0, 16.0), "0.125", M4_DIGIT(2))
    float hw_btnr_loDiffWgtCal_offset;
    // M4_NUMBER_DESC("hw_btnr_loDiffFirstLine_scale", "f32", M4_RANGE(0, 2.0), "0.125", M4_DIGIT(2))
    float hw_btnr_loDiffFirstLine_scale;
    // M4_NUMBER_DESC("hw_btnr_loDiffVfilt_wgt", "f32", M4_RANGE(0, 1.0), "0.875", M4_DIGIT(2))
    float hw_btnr_loDiffVfilt_wgt;
    // M4_BOOL_DESC("hw_btnr_sigmaHfilt_en", "0")
    bool hw_btnr_sigmaHfilt_en;
    // M4_NUMBER_DESC("hw_btnr_sigmaFirstLine_scale", "f32", M4_RANGE(0, 2.0), "1.0", M4_DIGIT(2))
    float hw_btnr_sigmaFirstLine_scale;
    // M4_NUMBER_DESC("hw_btnr_sigmaVfilt_wgt", "f32", M4_RANGE(0, 1.0), "0.875", M4_DIGIT(2))
    float hw_btnr_sigmaVfilt_wgt;
    // M4_NUMBER_DESC("hw_btnr_loWgtCal_maxLimit", "f32", M4_RANGE(0, 16.0), "4.0", M4_DIGIT(2))
    float hw_btnr_loWgtCal_maxLimit;
    // M4_NUMBER_DESC("hw_btnr_loWgtCal_scale", "f32", M4_RANGE(0, 16.0), "3.0", M4_DIGIT(2))
    float hw_btnr_loWgtCal_scale;
    // M4_NUMBER_DESC("hw_btnr_loWgtCal_offset", "f32", M4_RANGE(0, 16.0), "0.45", M4_DIGIT(2))
    float hw_btnr_loWgtCal_offset;
    // M4_BOOL_DESC("hw_btnr_loWgtHfilt_en", "1")
    bool hw_btnr_loWgtHfilt_en;
    // M4_NUMBER_DESC("hw_btnr_loWgtVfilt_wgt", "f32", M4_RANGE(0, 1.0), "0.875", M4_DIGIT(2))
    float hw_btnr_loWgtVfilt_wgt;
    // M4_NUMBER_DESC("hw_btnr_loMgePreWgt_scale", "f32", M4_RANGE(0, 16.0), "2.0", M4_DIGIT(2))
    float hw_btnr_loMgePreWgt_scale;
    // M4_NUMBER_DESC("hw_btnr_loMgePreWgt_offset", "f32", M4_RANGE(0, 16.0), "0.0", M4_DIGIT(2))
    float hw_btnr_loMgePreWgt_offset;



    // M4_NUMBER_DESC("hw_btnr_sigmaHdrS_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_sigmaHdrS_scale;
    // M4_NUMBER_DESC("hw_btnr_mode0LoWgtHdrS_scale", "f32", M4_RANGE(0, 16.0), "1.0", M4_DIGIT(2))
    float hw_btnr_mode0LoWgtHdrS_scale;
    // M4_NUMBER_DESC("hw_btnr_mode1LoWgtHdrS_scale", "f32", M4_RANGE(0, 16.0), "0.7", M4_DIGIT(2))
    float hw_btnr_mode1LoWgtHdrS_scale;
    // M4_NUMBER_DESC("hw_btnr_mode1LoWgtHdrS_offset", "f32", M4_RANGE(0, 16.0), "0.5", M4_DIGIT(2))
    float hw_btnr_mode1LoWgtHdrS_offset;
    // M4_NUMBER_DESC("hw_btnr_mode1WgtHdrS_scale", "f32", M4_RANGE(0, 16.0), "1.4", M4_DIGIT(2))
    float hw_btnr_mode1WgtHdrS_scale;


    // M4_NUMBER_DESC("sw_btnr_loMgeFrame_maxLimit", "f32", M4_RANGE(0, 4095.0), "32", M4_DIGIT(2))
    float sw_btnr_loMgeFrame_maxLimit;
    // M4_NUMBER_DESC("sw_btnr_loMgeFrame_minLimit", "f32", M4_RANGE(0, 4095.0), "0", M4_DIGIT(2))
    float sw_btnr_loMgeFrame_minLimit;
    // M4_NUMBER_DESC("sw_btnr_hiMgeFrame_maxLimit", "f32", M4_RANGE(0, 4095.0), "32", M4_DIGIT(2))
    float sw_btnr_hiMgeFrame_maxLimit;
    // M4_NUMBER_DESC("sw_btnr_hiMgeFrame_minLimit", "f32", M4_RANGE(0, 4095.0), "4", M4_DIGIT(2))
    float sw_btnr_hiMgeFrame_minLimit;
    // M4_NUMBER_DESC("sw_btnr_loMgeFrameHdrS_maxLimit", "f32", M4_RANGE(0, 4095.0), "32", M4_DIGIT(2))
    float sw_btnr_loMgeFrameHdrS_maxLimit;
    // M4_NUMBER_DESC("sw_btnr_loMgeFrameHdrS_minLimit", "f32", M4_RANGE(0, 4095.0), "0", M4_DIGIT(2))
    float sw_btnr_loMgeFrameHdrS_minLimit;
    // M4_NUMBER_DESC("sw_btnr_hiMgeFrameHdrS_maxLimit", "f32", M4_RANGE(0, 4095.0), "32", M4_DIGIT(2))
    float sw_btnr_hiMgeFrameHdrS_maxLimit;
    // M4_NUMBER_DESC("sw_btnr_hiMgeFrameHdrS_minLimit", "f32", M4_RANGE(0, 4095.0), "0", M4_DIGIT(2))
    float sw_btnr_hiMgeFrameHdrS_minLimit;


    // M4_BOOL_DESC("hw_autoSigCount_en", "1")
    bool hw_autoSigCount_en;
    // M4_NUMBER_DESC("hw_autoSigCount_thred", "u32", M4_RANGE(0, 1048575), "0", M4_DIGIT(0))
    uint32_t hw_autoSigCount_thred;
    // M4_NUMBER_DESC("hw_autoSigCountWgt_thred", "f32", M4_RANGE(0, 1.0), "0.8", M4_DIGIT(2))
    float hw_autoSigCountWgt_thred;
    // M4_NUMBER_DESC("sw_autoSigCountFilt_wgt", "f32", M4_RANGE(0, 1.0), "0.7", M4_DIGIT(2))
    float sw_autoSigCountFilt_wgt; //not use yet
    // M4_BOOL_DESC("sw_autoSigCountSpnr_en", "1")
    bool sw_autoSigCountSpnr_en;


    // M4_NUMBER_DESC("hw_btnr_curSpnrHiWgt_minLimit", "f32", M4_RANGE(0, 1.0), "0.15", M4_DIGIT(2))
    float hw_btnr_curSpnrHiWgt_minLimit;
    // M4_NUMBER_DESC("hw_btnr_preSpnrHiWgt_minLimit", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float hw_btnr_preSpnrHiWgt_minLimit;
    // M4_NUMBER_DESC("hw_btnr_gainOut_maxLimit", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float hw_btnr_gainOut_maxLimit;
    // M4_NUMBER_DESC("hw_btnr_noiseBal_mode", "u8", M4_RANGE(0, 1), "0", M4_DIGIT(0))
    uint8_t hw_btnr_noiseBal_mode;
    // M4_NUMBER_DESC("hw_btnr_noiseBalNr_strg", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float hw_btnr_noiseBalNr_strg;

    // M4_NUMBER_DESC("sw_curSpnrSpaceWgt_sigma", "f32", M4_RANGE(0, 100.0), "25", M4_DIGIT(2))
    float sw_curSpnrSpaceWgt_sigma;
    // M4_NUMBER_DESC("sw_iirSpnrSpaceWgt_sigma", "f32", M4_RANGE(0, 100.0), "25", M4_DIGIT(2))
    float sw_iirSpnrSpaceWgt_sigma;

    // M4_ARRAY_DESC("hw_tnrWgtFltCoef", "f32", M4_SIZE(1,3), M4_RANGE(0,4.0), "[4.0, 2.0, 1.0]", M4_DIGIT(3), M4_DYNAMIC(0))
    float hw_tnrWgtFltCoef[3];
    // M4_NUMBER_DESC("hw_bay3d_lowgt_ctrl", "u8", M4_RANGE(0, 3), "0", M4_DIGIT(0))
    uint8_t hw_bay3d_lowgt_ctrl;
    // M4_NUMBER_DESC("hw_bay3d_lowgt_offinit", "u16", M4_RANGE(0, 1023), "0", M4_DIGIT(0))
    uint16_t hw_bay3d_lowgt_offinit;


} CalibDbV2_BayerTnrV30_T_ISO_t;

typedef struct CalibDbV2_BayerTnrV30_T_Set_s {
    // M4_STRING_MARK_DESC("SNR_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "LSNR",M4_DYNAMIC(0), "index1")
    char *SNR_Mode;
    // M4_STRING_DESC("Sensor_Mode", M4_SIZE(1,1), M4_RANGE(0, 64), "lcg", M4_DYNAMIC(0))
    char *Sensor_Mode;
    // M4_STRUCT_LIST_DESC("Tuning_ISO", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_BayerTnrV30_T_ISO_t *Tuning_ISO;
    int Tuning_ISO_len;
} CalibDbV2_BayerTnrV30_T_Set_t;

typedef struct CalibDbV2_BayerTnrV30_Tuning_s {
    // M4_BOOL_DESC("enable", "1")
    bool enable;

    // M4_BOOL_DESC("hw_mdBypss_en", "0")
    bool hw_mdBypss_en;
    // M4_BOOL_DESC("hw_iirSpnrOut_en", "0")
    bool hw_iirSpnrOut_en;
    // M4_BOOL_DESC("hw_loMdWgtdbg_en", "0")
    bool hw_loMdWgtdbg_en;
    // M4_BOOL_DESC("hw_mdWgtOut_en", "0")
    bool hw_mdWgtOut_en;
#if 1
    // M4_BOOL_DESC("hw_curFiltOut_en", "0")
    bool hw_curFiltOut_en;
#endif

    // M4_NUMBER_DESC("hw_loDetection_mode", "u8", M4_RANGE(0, 2), "0", M4_DIGIT(0))
    uint8_t hw_loDetection_mode;


    // M4_STRUCT_LIST_DESC("Setting", M4_SIZE_DYNAMIC, "double_index_list")
    CalibDbV2_BayerTnrV30_T_Set_t *Setting;
    int Setting_len;
} CalibDbV2_BayerTnrV30_Tuning_t;

typedef struct CalibDbV2_BayerTnrV30_s {
    // M4_STRING_DESC("Version", M4_SIZE(1,1), M4_RANGE(0, 64), "V30_LITE", M4_DYNAMIC(0))
    char* Version;
    // M4_STRUCT_DESC("CalibPara", "normal_ui_style")
    CalibDbV2_BayerTnrV30_Calib_t CalibPara;
    // M4_STRUCT_DESC("Bayernr3D", "normal_ui_style")
    CalibDbV2_BayerTnrV30_Tuning_t TuningPara;
} CalibDbV2_BayerTnrV30_t;


RKAIQ_END_DECLARE
// clang-format on

#endif
