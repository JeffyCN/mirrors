/*
 * ynr_uapi_head_v24.h
 *
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef __CALIBDBV2_YNRV24_UAPI_HEADER_H__
#define __CALIBDBV2_YNRV24_UAPI_HEADER_H__

#include "rk_aiq_comm.h"
// clang-format off

RKAIQ_BEGIN_DECLARE

///////////////////////////ynr v1//////////////////////////////////////

#define YNR_V24_ISO_CURVE_POINT_BIT          4
#define YNR_V24_ISO_CURVE_POINT_NUM          ((1 << YNR_V24_ISO_CURVE_POINT_BIT)+1)


typedef struct RK_YNR_Params_V24_Select_s
{
    // M4_BOOL_DESC("enable", "1")
    bool enable;

    // M4_BOOL_DESC("sw_ynr_loSpnr_bypass", "0")
    bool sw_ynr_loSpnr_bypass;
    // M4_BOOL_DESC("sw_ynr_hiSpnr_bypass", "0")
    bool sw_ynr_hiSpnr_bypass;

    // M4_NUMBER_DESC("ynr_lci", "f32", M4_RANGE(0.0, 2.0), "0.5", M4_DIGIT(3))
    float ynr_lci;

    // M4_ARRAY_DESC("lumaPoint", "u16", M4_SIZE(1,17), M4_RANGE(0, 1024), "[0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t lumaPoint[YNR_V24_ISO_CURVE_POINT_NUM];
    // M4_ARRAY_DESC("sigma", "f32", M4_SIZE(1,17), M4_RANGE(0, 4095), "32", M4_DIGIT(3), M4_DYNAMIC(0))
    float sigma[YNR_V24_ISO_CURVE_POINT_NUM];

    // M4_ARRAY_DESC("sw_ynr_luma2loStrg_lumaPoint", "u16", M4_SIZE(1,6), M4_RANGE(0,256), "[0,32,64,128,192,256]", M4_DIGIT(0), M4_DYNAMIC(0))
    uint16_t sw_ynr_luma2loStrg_lumaPoint[6];
    // M4_ARRAY_DESC("sw_ynr_luma2loStrg_val", "f32", M4_SIZE(1,6), M4_RANGE(0,2.0), "[1.0,1.0,1.0,1.0,1.0,1.0]", M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_ynr_luma2loStrg_val[6];

    //local gain control
    // M4_NUMBER_DESC("sw_ynr_gainMerge_alpha", "f32", M4_RANGE(0, 1.0), "0", M4_DIGIT(2))
    float sw_ynr_gainMerge_alpha;
    // M4_NUMBER_DESC("sw_ynr_globalSet_gain", "f32", M4_RANGE(0, 64.0), "1.0", M4_DIGIT(2))
    float sw_ynr_globalSet_gain;

    // sw_ynr_hiSpnr_ctrl
    // M4_NUMBER_DESC("sw_ynr_hiSpnr_strg", "f32", M4_RANGE(0, 16.0), "0.4", M4_DIGIT(2))
    float sw_ynr_hiSpnr_strg;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrLocalGain_alpha", "f32", M4_RANGE(0, 1.0), "0.25", M4_DIGIT(2))
    float sw_ynr_hiSpnrLocalGain_alpha;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_centerWgt", "f32", M4_RANGE(0, 128.0), "2.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_wgtOffset", "f32", M4_RANGE(0, 1.0), "0.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_wgtOffset;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrSigma_minLimit", "f32", M4_RANGE(0, 1.0), "0.0028", M4_DIGIT(4))
    float sw_ynr_hiSpnrSigma_minLimit;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_wgt", "f32", M4_RANGE(0, 1.0), "0.45", M4_DIGIT(2))
    float sw_ynr_hiSpnrFilt_wgt;
    // M4_NUMBER_DESC("sw_ynr_hiSpnr_gainThred", "f32", M4_RANGE(0, 64.0), "2.0", M4_DIGIT(2))
    float sw_ynr_hiSpnr_gainThred;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrStrongEdge_scale", "f32", M4_RANGE(0, 32.0), "4.0", M4_DIGIT(2))
    float sw_ynr_hiSpnrStrongEdge_scale;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff0", "f32", M4_RANGE(0, 15), "7", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff0;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff1", "f32", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff1;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff2", "f32", M4_RANGE(0, 15), "3", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff2;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff3", "f32", M4_RANGE(0, 15), "6", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff3;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff4", "f32", M4_RANGE(0, 15), "5", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff4;
    // M4_NUMBER_DESC("sw_ynr_hiSpnrFilt_coeff5", "f32", M4_RANGE(0, 15), "3", M4_DIGIT(0))
    float sw_ynr_hiSpnrFilt_coeff5;

    // sw_ynr_dsFilt_ctrl
    // M4_NUMBER_DESC("sw_ynr_dsImg_edgeScale", "f32", M4_RANGE(0, 10), "1", M4_DIGIT(0))
    float sw_ynr_dsImg_edgeScale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltSoftThred_scale", "f32", M4_RANGE(0, 16.0), "0.25", M4_DIGIT(2))
    float sw_ynr_dsFiltSoftThred_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltWgtThred_scale", "f32", M4_RANGE(0, 8.0), "0.1875", M4_DIGIT(2))
    float sw_ynr_dsFiltWgtThred_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFilt_centerWgt", "f32", M4_RANGE(0, 2.0), "0.025", M4_DIGIT(4))
    float sw_ynr_dsFilt_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_dsFilt_strg", "f32", M4_RANGE(0.0, 32.0), "0.3", M4_DIGIT(3))
    float sw_ynr_dsFilt_strg;
    // M4_NUMBER_DESC("sw_ynr_dsIIRinitWgt_scale", "f32", M4_RANGE(0, 1.0), "0.06", M4_DIGIT(2))
    float sw_ynr_dsIIRinitWgt_scale;
    // M4_NUMBER_DESC("sw_ynr_dsFiltLocalGain_alpha", "f32", M4_RANGE(0, 1.0), "0.25", M4_DIGIT(2))
    float sw_ynr_dsFiltLocalGain_alpha;

    // sw_ynr_loSpnr_ctrl
    // M4_NUMBER_DESC("sw_ynr_preFilt_strg", "f32", M4_RANGE(0, 16.0), "1", M4_DIGIT(2))
    float sw_ynr_preFilt_strg;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_wgt", "f32", M4_RANGE(0, 1.0), "0.5", M4_DIGIT(2))
    float sw_ynr_loSpnr_wgt;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_centerWgt", "f32", M4_RANGE(0, 4.0), "1.2", M4_DIGIT(2))
    float sw_ynr_loSpnr_centerWgt;
    // M4_NUMBER_DESC("sw_ynr_loSpnr_strg", "f32", M4_RANGE(0, 128.0), "0.45", M4_DIGIT(2))
    float sw_ynr_loSpnr_strg;
    // M4_NUMBER_DESC("sw_ynr_loSpnrDistVstrg_scale", "f32", M4_RANGE(0, 8.0), "0.0625", M4_DIGIT(4))
    float sw_ynr_loSpnrDistVstrg_scale;
    // M4_NUMBER_DESC("sw_ynr_loSpnrDistHstrg_scale", "f32", M4_RANGE(0, 8.0), "0.75", M4_DIGIT(4))
    float sw_ynr_loSpnrDistHstrg_scale;

    // M4_ARRAY_DESC("sw_ynr_radius2strg_val", "f32", M4_SIZE(1,17), M4_RANGE(0,16), "1.0", M4_DIGIT(3), M4_DYNAMIC(0))
    float sw_ynr_radius2strg_val[17];
    // M4_ARRAY_DESC("sw_ynr_loSpnrGain2Strg_val", "f32", M4_SIZE(1,9), M4_RANGE(0,16), "[0,1,2,4,8,16,32,64,128]", M4_DIGIT(3), M4_DYNAMIC(0))
    float sw_ynr_loSpnrGain2Strg_val[9];
} RK_YNR_Params_V24_Select_t;

typedef struct Aynr_ExpInfo_V24_s {
    // M4_NUMBER_DESC("hdr_mode", "u8", M4_RANGE(0, 2), "0", M4_DIGIT(0))
    int hdr_mode;
    // M4_NUMBER_DESC("snr_mode", "s8", M4_RANGE(0, 2), "0", M4_DIGIT(0))
    int   snr_mode;

    // M4_ARRAY_DESC("time", "f32", M4_SIZE(1,3), M4_RANGE(0, 1024), "0.01", M4_DIGIT(6))
    float arTime[3];
    // M4_ARRAY_DESC("again", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float arAGain[3];
    // M4_ARRAY_DESC("dgain", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float arDGain[3];
    // M4_ARRAY_DESC("isp_dgain", "f32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float isp_dgain[3];
    // M4_NUMBER_DESC("blc_ob_predgain", "f32", M4_RANGE(0, 204800), "1", M4_DIGIT(3))
    float blc_ob_predgain;
    // M4_ARRAY_DESC("iso", "u32", M4_SIZE(1,3), M4_RANGE(0, 204800), "1", M4_DIGIT(0))
    int   arIso[3];

    // M4_NUMBER_DESC("isoLevelLow", "u8", M4_RANGE(0, 12), "0", M4_DIGIT(0))
    int isoLevelLow;
    // M4_NUMBER_DESC("isoLevelHig", "u8", M4_RANGE(0, 12), "0", M4_DIGIT(0))
    int isoLevelHig;

    // M4_NUMBER_DESC("rawWidth", "s32", M4_RANGE(0, 65535), "0", M4_DIGIT(0))
    int rawWidth;
    // M4_NUMBER_DESC("rawHeight", "s32", M4_RANGE(0, 65535), "0", M4_DIGIT(0))
    int rawHeight;
} Aynr_ExpInfo_V24_t;

typedef struct rk_aiq_ynr_info_v24_s {
    // M4_ARRAY_TABLE_DESC("sync", "array_table_ui", "none", "1")
    rk_aiq_uapi_sync_t sync;
    // M4_NUMBER_DESC("iso", "u32", M4_RANGE(0, 204800), "50", M4_DIGIT(0), "0", "0")
    int iso;
    // M4_ARRAY_TABLE_DESC("expo_info", "normal_ui_style", "none", "0", "0")
    Aynr_ExpInfo_V24_t expo_info;
} rk_aiq_ynr_info_v24_t;

RKAIQ_END_DECLARE
// clang-format on

#endif
