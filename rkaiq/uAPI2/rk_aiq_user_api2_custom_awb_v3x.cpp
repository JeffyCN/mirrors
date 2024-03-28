/*
 *  Copyright (c) 2021 Rockchip Corporation
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

//#include "RkAiqCalibDbV2Helper.h"


#include "rk_aiq_user_api2_custom_awb_type_v3x.h"

RKAIQ_BEGIN_DECLARE


/* awb config of AIQ framework */
typedef struct rk_aiq_rkAwb_config_s {
    int Working_mode;//values look up in rk_aiq_working_mode_t definiton
    int RawWidth;
    int RawHeight;
    rk_aiq_isp_awb_meas_cfg_v3x_t  awbHwConfig;
} rk_aiq_rkAwb_config_t;

static XCamReturn initCustomAwbHwConfigGw(rk_aiq_customAwb_hw_cfg_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_customAwb_hw_cfg_t));
    awbHwConfig->awbEnable             =    1;
    awbHwConfig->xyDetectionEnable          =    0;
    awbHwConfig->uvDetectionEnable          =    0;
    awbHwConfig->threeDyuvEnable          =    0;
    awbHwConfig->blkWeightEnable    =    0;
    awbHwConfig->lscBypEnable    =    0;
    awbHwConfig->blkMeasureMode     =    RK_AIQ_AWB_BLK_STAT_MODE_REALWP_V201;
    awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)7;
    awbHwConfig->wpDiffWeiEnable   =    0;
    awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->lightNum      =    7;
    awbHwConfig->windowSet[0]         =    0;
    awbHwConfig->windowSet[1]          =    0;
    awbHwConfig->windowSet[2]         =    3840;
    awbHwConfig->windowSet[3]         =    2160;
    awbHwConfig->maxR          =    230;
    awbHwConfig->maxG          =    230;
    awbHwConfig->maxB          =    230;
    awbHwConfig->maxY          =    230;
    awbHwConfig->minR          =    3;
    awbHwConfig->minG          =    3;
    awbHwConfig->minB          =    3;
    awbHwConfig->minY          =    3;
    awbHwConfig->multiwindow_en  =    0;
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn initCustomAwbHwConfigWp(rk_aiq_customAwb_hw_cfg_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_isp_awb_meas_cfg_v3x_t));
    awbHwConfig->awbEnable             =    1;//rawawb enable
    awbHwConfig->xyDetectionEnable          =    1;//xy detect  enable for all light
    awbHwConfig->uvDetectionEnable          =    1;//uv detect  enable for all  light
    awbHwConfig->threeDyuvEnable          =    1;//uv detect  enable for all  light
    awbHwConfig->blkWeightEnable    =    0;
    awbHwConfig->lscBypEnable    =    0;
    awbHwConfig->blkMeasureMode     =    RK_AIQ_AWB_BLK_STAT_MODE_REALWP_V201;
    awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)7;
    awbHwConfig->wpDiffWeiEnable   =    0;
    awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->threeDyuvIllu[0]       =    7;
    awbHwConfig->threeDyuvIllu[1]       =    7;
    awbHwConfig->threeDyuvIllu[2]       =    7;
    awbHwConfig->threeDyuvIllu[3]       =    7;
    awbHwConfig->lightNum      =    7; //measurment light num ,0~7
    awbHwConfig->windowSet[0]         =    0;//crop h_offs mod 2 == 0
    awbHwConfig->windowSet[1]          =    0;//crop v_offs mod 2 == 0
    awbHwConfig->windowSet[2]         =    3840;//crop h_size,8x8:mod 8==0,4x4:mod 4==0
    awbHwConfig->windowSet[3]         =    2160;//crop v_size,8x8:mod 8==0,4x4:mod 4==0
    awbHwConfig->maxR          =    230;
    awbHwConfig->maxG          =    230;
    awbHwConfig->maxB          =    230;
    awbHwConfig->maxY          =    230;
    awbHwConfig->minR          =    3;
    awbHwConfig->minG          =    3;
    awbHwConfig->minB          =    3;
    awbHwConfig->minY          =    3;
    awbHwConfig->uvRange_param[0].pu_region[0]    =    247;
    awbHwConfig->uvRange_param[0].pv_region[0]    =    252;
    awbHwConfig->uvRange_param[0].pu_region[1]    =    114;
    awbHwConfig->uvRange_param[0].pv_region[1]    =    206;
    awbHwConfig->uvRange_param[0].pu_region[2]    =    105;
    awbHwConfig->uvRange_param[0].pv_region[2]    =    243;
    awbHwConfig->uvRange_param[0].pu_region[3]    =    246;
    awbHwConfig->uvRange_param[0].pv_region[3]    =    254;
    awbHwConfig->uvRange_param[0].slope_inv[0]      =    2961;
    awbHwConfig->uvRange_param[0].slope_inv[1]      =    -249;
    awbHwConfig->uvRange_param[0].slope_inv[2]      =    13126;
    awbHwConfig->uvRange_param[0].slope_inv[3]      =    -512;
    awbHwConfig->uvRange_param[1].pu_region[0]    =    251;
    awbHwConfig->uvRange_param[1].pv_region[0]    =    249;
    awbHwConfig->uvRange_param[1].pu_region[1]    =    159;
    awbHwConfig->uvRange_param[1].pv_region[1]    =    160;
    awbHwConfig->uvRange_param[1].pu_region[2]    =    141;
    awbHwConfig->uvRange_param[1].pv_region[2]    =    173;
    awbHwConfig->uvRange_param[1].pu_region[3]    =    249;
    awbHwConfig->uvRange_param[1].pv_region[3]    =    250;
    awbHwConfig->uvRange_param[1].slope_inv[0]      =    1059;
    awbHwConfig->uvRange_param[1].slope_inv[1]      =    -1418;
    awbHwConfig->uvRange_param[1].slope_inv[2]      =    1436;
    awbHwConfig->uvRange_param[1].slope_inv[3]      =    -2048;
    awbHwConfig->uvRange_param[2].pu_region[0]    =    252;
    awbHwConfig->uvRange_param[2].pv_region[0]    =    249;
    awbHwConfig->uvRange_param[2].pu_region[1]    =    180;
    awbHwConfig->uvRange_param[2].pv_region[1]    =    148;
    awbHwConfig->uvRange_param[2].pu_region[2]    =    158;
    awbHwConfig->uvRange_param[2].pv_region[2]    =    160;
    awbHwConfig->uvRange_param[2].pu_region[3]    =    250;
    awbHwConfig->uvRange_param[2].pv_region[3]    =    249;
    awbHwConfig->uvRange_param[2].slope_inv[0]      =    730;
    awbHwConfig->uvRange_param[2].slope_inv[1]      =    -1877;
    awbHwConfig->uvRange_param[2].slope_inv[2]      =    1059;
    awbHwConfig->uvRange_param[2].slope_inv[3]      =    262143;
    awbHwConfig->uvRange_param[3].pu_region[0]    =    254;
    awbHwConfig->uvRange_param[3].pv_region[0]    =    247;
    awbHwConfig->uvRange_param[3].pu_region[1]    =    204;
    awbHwConfig->uvRange_param[3].pv_region[1]    =    134;
    awbHwConfig->uvRange_param[3].pu_region[2]    =    180;
    awbHwConfig->uvRange_param[3].pv_region[2]    =    148;
    awbHwConfig->uvRange_param[3].pu_region[3]    =    252;
    awbHwConfig->uvRange_param[3].pv_region[3]    =    249;
    awbHwConfig->uvRange_param[3].slope_inv[0]      =    453;
    awbHwConfig->uvRange_param[3].slope_inv[1]      =    -1755;
    awbHwConfig->uvRange_param[3].slope_inv[2]      =    730;
    awbHwConfig->uvRange_param[3].slope_inv[3]      =    -1024;
    awbHwConfig->uvRange_param[4].pu_region[0]    =    255;
    awbHwConfig->uvRange_param[4].pv_region[0]    =    247;
    awbHwConfig->uvRange_param[4].pu_region[1]    =    225;
    awbHwConfig->uvRange_param[4].pv_region[1]    =    129;
    awbHwConfig->uvRange_param[4].pu_region[2]    =    201;
    awbHwConfig->uvRange_param[4].pv_region[2]    =    134;
    awbHwConfig->uvRange_param[4].pu_region[3]    =    254;
    awbHwConfig->uvRange_param[4].pv_region[3]    =    247;
    awbHwConfig->uvRange_param[4].slope_inv[0]      =    260;
    awbHwConfig->uvRange_param[4].slope_inv[1]      =    -4915;
    awbHwConfig->uvRange_param[4].slope_inv[2]      =    480;
    awbHwConfig->uvRange_param[4].slope_inv[3]      =    262143;
    awbHwConfig->uvRange_param[5].pu_region[0]    =    246;
    awbHwConfig->uvRange_param[5].pv_region[0]    =    257;
    awbHwConfig->uvRange_param[5].pu_region[1]    =    101;
    awbHwConfig->uvRange_param[5].pv_region[1]    =    278;
    awbHwConfig->uvRange_param[5].pu_region[2]    =    104;
    awbHwConfig->uvRange_param[5].pv_region[2]    =    243;
    awbHwConfig->uvRange_param[5].pu_region[3]    =    246;
    awbHwConfig->uvRange_param[5].pv_region[3]    =    254;
    awbHwConfig->uvRange_param[5].slope_inv[0]      =    -7070;
    awbHwConfig->uvRange_param[5].slope_inv[1]      =    -88;
    awbHwConfig->uvRange_param[5].slope_inv[2]      =    13219;
    awbHwConfig->uvRange_param[5].slope_inv[3]      =    0;
    awbHwConfig->uvRange_param[6].pu_region[0]    =    250;
    awbHwConfig->uvRange_param[6].pv_region[0]    =    250;
    awbHwConfig->uvRange_param[6].pu_region[1]    =    144;
    awbHwConfig->uvRange_param[6].pv_region[1]    =    171;
    awbHwConfig->uvRange_param[6].pu_region[2]    =    114;
    awbHwConfig->uvRange_param[6].pv_region[2]    =    207;
    awbHwConfig->uvRange_param[6].pu_region[3]    =    247;
    awbHwConfig->uvRange_param[6].pv_region[3]    =    252;
    awbHwConfig->uvRange_param[6].slope_inv[0]      =    1374;
    awbHwConfig->uvRange_param[6].slope_inv[1]      =    -853;
    awbHwConfig->uvRange_param[6].slope_inv[2]      =    3026;
    awbHwConfig->uvRange_param[6].slope_inv[3]      =    -1536;
    awbHwConfig->icrgb2RYuv_matrix[0]  =        12;
    awbHwConfig->icrgb2RYuv_matrix[1]  =        69;
    awbHwConfig->icrgb2RYuv_matrix[2]  =        7;
    awbHwConfig->icrgb2RYuv_matrix[3]  =        665;
    awbHwConfig->icrgb2RYuv_matrix[4]  =        -44;
    awbHwConfig->icrgb2RYuv_matrix[5]  =        -2;
    awbHwConfig->icrgb2RYuv_matrix[6]  =        32;
    awbHwConfig->icrgb2RYuv_matrix[7]  =        2268;
    awbHwConfig->icrgb2RYuv_matrix[8]  =        23;
    awbHwConfig->icrgb2RYuv_matrix[9]  =        -24;
    awbHwConfig->icrgb2RYuv_matrix[10]  =        27;
    awbHwConfig->icrgb2RYuv_matrix[11]  =        1690;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[0]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[1]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[2]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[3]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[4]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[5]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[0]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[1]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[2]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[3]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[4]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[5]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[0]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[1]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[2]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[0]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[1]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[2]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].disP1P2  =   0;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[0]  =    1534;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[1]  =    1535;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[2]  =    1027;
    awbHwConfig->rgb2xy_param.rotationMat[0]  =    -2202;
    awbHwConfig->rgb2xy_param.rotationMat[1]  =    3454;
    awbHwConfig->rgb2xy_param.rotationMat[2]  =    -535;
    awbHwConfig->rgb2xy_param.rotationMat[3]  =    3454;
    awbHwConfig->rgb2xy_param.rotationMat[4]  =    2202;
    awbHwConfig->rgb2xy_param.rotationMat[5]  =    2826;
    awbHwConfig->xyRange_param[0].NorrangeX[0]  =    -1432;
    awbHwConfig->xyRange_param[0].NorrangeX[1]  =    -962;
    awbHwConfig->xyRange_param[0].NorrangeY[0]  =    134;
    awbHwConfig->xyRange_param[0].NorrangeY[1]  =    -30;
    awbHwConfig->xyRange_param[0].SperangeX[0]  =    -1432;
    awbHwConfig->xyRange_param[0].SperangeX[1]  =    -962;
    awbHwConfig->xyRange_param[0].SperangeY[0]  =    165;
    awbHwConfig->xyRange_param[0].SperangeY[1]  =    -60;
    awbHwConfig->xyRange_param[1].NorrangeX[0]  =    -959;
    awbHwConfig->xyRange_param[1].NorrangeX[1]  =    -507;
    awbHwConfig->xyRange_param[1].NorrangeY[0]  =    -46;
    awbHwConfig->xyRange_param[1].NorrangeY[1]  =    -159;
    awbHwConfig->xyRange_param[1].SperangeX[0]  =    -961;
    awbHwConfig->xyRange_param[1].SperangeX[1]  =    -507;
    awbHwConfig->xyRange_param[1].SperangeY[0]  =    -46;
    awbHwConfig->xyRange_param[1].SperangeY[1]  =    -179;
    awbHwConfig->xyRange_param[2].NorrangeX[0]  =    -507;
    awbHwConfig->xyRange_param[2].NorrangeX[1]  =    -160;
    awbHwConfig->xyRange_param[2].NorrangeY[0]  =    181;
    awbHwConfig->xyRange_param[2].NorrangeY[1]  =    10;
    awbHwConfig->xyRange_param[2].SperangeX[0]  =    -507;
    awbHwConfig->xyRange_param[2].SperangeX[1]  =    -160;
    awbHwConfig->xyRange_param[2].SperangeY[0]  =    205;
    awbHwConfig->xyRange_param[2].SperangeY[1]  =    -20;
    awbHwConfig->xyRange_param[3].NorrangeX[0]  =    -160;
    awbHwConfig->xyRange_param[3].NorrangeX[1]  =    74;
    awbHwConfig->xyRange_param[3].NorrangeY[0]  =    79;
    awbHwConfig->xyRange_param[3].NorrangeY[1]  =    -85;
    awbHwConfig->xyRange_param[3].SperangeX[0]  =    -160;
    awbHwConfig->xyRange_param[3].SperangeX[1]  =    74;
    awbHwConfig->xyRange_param[3].SperangeY[0]  =    109;
    awbHwConfig->xyRange_param[3].SperangeY[1]  =    -103;
    awbHwConfig->xyRange_param[4].NorrangeX[0]  =    74;
    awbHwConfig->xyRange_param[4].NorrangeX[1]  =    227;
    awbHwConfig->xyRange_param[4].NorrangeY[0]  =    102;
    awbHwConfig->xyRange_param[4].NorrangeY[1]  =    -66;
    awbHwConfig->xyRange_param[4].SperangeX[0]  =    74;
    awbHwConfig->xyRange_param[4].SperangeX[1]  =    227;
    awbHwConfig->xyRange_param[4].SperangeY[0]  =    133;
    awbHwConfig->xyRange_param[4].SperangeY[1]  =    -86;
    awbHwConfig->xyRange_param[5].NorrangeX[0]  =    -1742;
    awbHwConfig->xyRange_param[5].NorrangeX[1]  =    -1432;
    awbHwConfig->xyRange_param[5].NorrangeY[0]  =    90;
    awbHwConfig->xyRange_param[5].NorrangeY[1]  =    -80;
    awbHwConfig->xyRange_param[5].SperangeX[0]  =    -1742;
    awbHwConfig->xyRange_param[5].SperangeX[1]  =    -1432;
    awbHwConfig->xyRange_param[5].SperangeY[0]  =    121;
    awbHwConfig->xyRange_param[5].SperangeY[1]  =    -111;
    awbHwConfig->xyRange_param[6].NorrangeX[0]  =    -962;
    awbHwConfig->xyRange_param[6].NorrangeX[1]  =    -509;
    awbHwConfig->xyRange_param[6].NorrangeY[0]  =    114;
    awbHwConfig->xyRange_param[6].NorrangeY[1]  =    -43;
    awbHwConfig->xyRange_param[6].SperangeX[0]  =    -962;
    awbHwConfig->xyRange_param[6].SperangeX[1]  =    -509;
    awbHwConfig->xyRange_param[6].SperangeY[0]  =    134;
    awbHwConfig->xyRange_param[6].SperangeY[1]  =    -45;
    awbHwConfig->multiwindow_en  =    0;
    awbHwConfig->multiwindow[0][0]  =    0;
    awbHwConfig->multiwindow[0][1]  =    0;
    awbHwConfig->multiwindow[0][2]  =    0;
    awbHwConfig->multiwindow[0][3]  =    0;
    awbHwConfig->multiwindow[1][0]  =    0;
    awbHwConfig->multiwindow[1][1]  =    0;
    awbHwConfig->multiwindow[1][2]  =    0;
    awbHwConfig->multiwindow[1][3]  =    0;
    awbHwConfig->multiwindow[2][0]  =    0;
    awbHwConfig->multiwindow[2][1]  =    0;
    awbHwConfig->multiwindow[2][2]  =    0;
    awbHwConfig->multiwindow[2][3]  =    0;
    awbHwConfig->multiwindow[3][0]  =    0;
    awbHwConfig->multiwindow[3][1]  =    0;
    awbHwConfig->multiwindow[3][2]  =    0;
    awbHwConfig->multiwindow[3][3]  =    0;
    awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[0].measureEnable   =     0;
    awbHwConfig->excludeWpRange[0].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[0].xu[0]           =     0;
    awbHwConfig->excludeWpRange[0].xu[1]           =     0;
    awbHwConfig->excludeWpRange[0].yv[0]           =     0;
    awbHwConfig->excludeWpRange[0].yv[1]           =     0;
    awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[1].measureEnable   =     0;
    awbHwConfig->excludeWpRange[1].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[1].xu[0]           =     0;
    awbHwConfig->excludeWpRange[1].xu[1]           =     0;
    awbHwConfig->excludeWpRange[1].yv[0]           =     0;
    awbHwConfig->excludeWpRange[1].yv[1]           =     0;
    awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[2].measureEnable   =     0;
    awbHwConfig->excludeWpRange[2].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[2].xu[0]           =     0;
    awbHwConfig->excludeWpRange[2].xu[1]           =     0;
    awbHwConfig->excludeWpRange[2].yv[0]           =     0;
    awbHwConfig->excludeWpRange[2].yv[1]           =     0;
    awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[3].measureEnable   =     0;
    awbHwConfig->excludeWpRange[3].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[3].xu[0]           =     0;
    awbHwConfig->excludeWpRange[3].xu[1]           =     0;
    awbHwConfig->excludeWpRange[3].yv[0]           =     0;
    awbHwConfig->excludeWpRange[3].yv[1]           =     0;
    awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[4].measureEnable   =     0;
    awbHwConfig->excludeWpRange[4].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[4].xu[0]           =     0;
    awbHwConfig->excludeWpRange[4].xu[1]           =     0;
    awbHwConfig->excludeWpRange[4].yv[0]           =     0;
    awbHwConfig->excludeWpRange[4].yv[1]           =     0;
    awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[5].measureEnable   =     0;
    awbHwConfig->excludeWpRange[5].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[5].xu[0]           =     0;
    awbHwConfig->excludeWpRange[5].xu[1]           =     0;
    awbHwConfig->excludeWpRange[5].yv[0]           =     0;
    awbHwConfig->excludeWpRange[5].yv[1]           =     0;
    awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[6].measureEnable   =     0;
    awbHwConfig->excludeWpRange[6].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[6].xu[0]           =     0;
    awbHwConfig->excludeWpRange[6].xu[1]           =     0;
    awbHwConfig->excludeWpRange[6].yv[0]           =     0;
    awbHwConfig->excludeWpRange[6].yv[1]           =     0;
    awbHwConfig->wpDiffwei_y[0]  =0;
    awbHwConfig->wpDiffwei_y[1]  =16;
    awbHwConfig->wpDiffwei_y[2]  =32;
    awbHwConfig->wpDiffwei_y[3]  =64;
    awbHwConfig->wpDiffwei_y[4]  =96;
    awbHwConfig->wpDiffwei_y[5]  =128;
    awbHwConfig->wpDiffwei_y[6]  =192;
    awbHwConfig->wpDiffwei_y[7]  =224;
    awbHwConfig->wpDiffwei_y[8]  =240;
    awbHwConfig->wpDiffwei_w[0]  =31;
    awbHwConfig->wpDiffwei_w[1]  =31;
    awbHwConfig->wpDiffwei_w[2]  =31;
    awbHwConfig->wpDiffwei_w[3]  =31;
    awbHwConfig->wpDiffwei_w[4]  =31;
    awbHwConfig->wpDiffwei_w[5]  =31;
    awbHwConfig->wpDiffwei_w[6]  =31;
    awbHwConfig->wpDiffwei_w[7]  =31;
    awbHwConfig->wpDiffwei_w[8]  =31;
    awbHwConfig->blkWeight[0]  =6;
    awbHwConfig->blkWeight[1]  =6;
    awbHwConfig->blkWeight[2]  =6;
    awbHwConfig->blkWeight[3]  =8;
    awbHwConfig->blkWeight[4]  =8;
    awbHwConfig->blkWeight[5]  =8;
    awbHwConfig->blkWeight[6]  =8;
    awbHwConfig->blkWeight[7]  =10;
    awbHwConfig->blkWeight[8]  =8;
    awbHwConfig->blkWeight[9]  =8;
    awbHwConfig->blkWeight[10]  =8;
    awbHwConfig->blkWeight[11]  =8;
    awbHwConfig->blkWeight[12]  =6;
    awbHwConfig->blkWeight[13]  =6;
    awbHwConfig->blkWeight[14]  =6;
    awbHwConfig->blkWeight[15]  =6;
    awbHwConfig->blkWeight[16]  =6;
    awbHwConfig->blkWeight[17]  =8;
    awbHwConfig->blkWeight[18]  =8;
    awbHwConfig->blkWeight[19]  =10;
    awbHwConfig->blkWeight[20]  =10;
    awbHwConfig->blkWeight[21]  =12;
    awbHwConfig->blkWeight[22]  =12;
    awbHwConfig->blkWeight[23]  =12;
    awbHwConfig->blkWeight[24]  =10;
    awbHwConfig->blkWeight[25]  =10;
    awbHwConfig->blkWeight[26]  =8;
    awbHwConfig->blkWeight[27]  =8;
    awbHwConfig->blkWeight[28]  =6;
    awbHwConfig->blkWeight[29]  =6;
    awbHwConfig->blkWeight[30]  =6;
    awbHwConfig->blkWeight[31]  =8;
    awbHwConfig->blkWeight[32]  =10;
    awbHwConfig->blkWeight[33]  =12;
    awbHwConfig->blkWeight[34]  =14;
    awbHwConfig->blkWeight[35]  =16;
    awbHwConfig->blkWeight[36]  =18;
    awbHwConfig->blkWeight[37]  =20;
    awbHwConfig->blkWeight[38]  =18;
    awbHwConfig->blkWeight[39]  =16;
    awbHwConfig->blkWeight[40]  =14;
    awbHwConfig->blkWeight[41]  =12;
    awbHwConfig->blkWeight[42]  =10;
    awbHwConfig->blkWeight[43]  =8;
    awbHwConfig->blkWeight[44]  =6;
    awbHwConfig->blkWeight[45]  =8;
    awbHwConfig->blkWeight[46]  =8;
    awbHwConfig->blkWeight[47]  =12;
    awbHwConfig->blkWeight[48]  =16;
    awbHwConfig->blkWeight[49]  =22;
    awbHwConfig->blkWeight[50]  =26;
    awbHwConfig->blkWeight[51]  =30;
    awbHwConfig->blkWeight[52]  =32;
    awbHwConfig->blkWeight[53]  =30;
    awbHwConfig->blkWeight[54]  =26;
    awbHwConfig->blkWeight[55]  =22;
    awbHwConfig->blkWeight[56]  =16;
    awbHwConfig->blkWeight[57]  =12;
    awbHwConfig->blkWeight[58]  =8;
    awbHwConfig->blkWeight[59]  =8;
    awbHwConfig->blkWeight[60]  =8;
    awbHwConfig->blkWeight[61]  =10;
    awbHwConfig->blkWeight[62]  =14;
    awbHwConfig->blkWeight[63]  =22;
    awbHwConfig->blkWeight[64]  =28;
    awbHwConfig->blkWeight[65]  =36;
    awbHwConfig->blkWeight[66]  =42;
    awbHwConfig->blkWeight[67]  =46;
    awbHwConfig->blkWeight[68]  =42;
    awbHwConfig->blkWeight[69]  =36;
    awbHwConfig->blkWeight[70]  =28;
    awbHwConfig->blkWeight[71]  =22;
    awbHwConfig->blkWeight[72]  =14;
    awbHwConfig->blkWeight[73]  =10;
    awbHwConfig->blkWeight[74]  =8;
    awbHwConfig->blkWeight[75]  =8;
    awbHwConfig->blkWeight[76]  =10;
    awbHwConfig->blkWeight[77]  =16;
    awbHwConfig->blkWeight[78]  =26;
    awbHwConfig->blkWeight[79]  =36;
    awbHwConfig->blkWeight[80]  =46;
    awbHwConfig->blkWeight[81]  =54;
    awbHwConfig->blkWeight[82]  =58;
    awbHwConfig->blkWeight[83]  =54;
    awbHwConfig->blkWeight[84]  =46;
    awbHwConfig->blkWeight[85]  =36;
    awbHwConfig->blkWeight[86]  =26;
    awbHwConfig->blkWeight[87]  =16;
    awbHwConfig->blkWeight[88]  =10;
    awbHwConfig->blkWeight[89]  =8;
    awbHwConfig->blkWeight[90]  =8;
    awbHwConfig->blkWeight[91]  =12;
    awbHwConfig->blkWeight[92]  =18;
    awbHwConfig->blkWeight[93]  =30;
    awbHwConfig->blkWeight[94]  =42;
    awbHwConfig->blkWeight[95]  =54;
    awbHwConfig->blkWeight[96]  =63;
    awbHwConfig->blkWeight[97]  =63;
    awbHwConfig->blkWeight[98]  =63;
    awbHwConfig->blkWeight[99]  =54;
    awbHwConfig->blkWeight[100]  =42;
    awbHwConfig->blkWeight[101]  =30;
    awbHwConfig->blkWeight[102]  =18;
    awbHwConfig->blkWeight[103]  =12;
    awbHwConfig->blkWeight[104]  =8;
    awbHwConfig->blkWeight[105]  =10;
    awbHwConfig->blkWeight[106]  =12;
    awbHwConfig->blkWeight[107]  =20;
    awbHwConfig->blkWeight[108]  =32;
    awbHwConfig->blkWeight[109]  =46;
    awbHwConfig->blkWeight[110]  =58;
    awbHwConfig->blkWeight[111]  =63;
    awbHwConfig->blkWeight[112]  =63;
    awbHwConfig->blkWeight[113]  =63;
    awbHwConfig->blkWeight[114]  =58;
    awbHwConfig->blkWeight[115]  =46;
    awbHwConfig->blkWeight[116]  =32;
    awbHwConfig->blkWeight[117]  =20;
    awbHwConfig->blkWeight[118]  =12;
    awbHwConfig->blkWeight[119]  =10;
    awbHwConfig->blkWeight[120]  =8;
    awbHwConfig->blkWeight[121]  =12;
    awbHwConfig->blkWeight[122]  =18;
    awbHwConfig->blkWeight[123]  =30;
    awbHwConfig->blkWeight[124]  =42;
    awbHwConfig->blkWeight[125]  =54;
    awbHwConfig->blkWeight[126]  =63;
    awbHwConfig->blkWeight[127]  =63;
    awbHwConfig->blkWeight[128]  =63;
    awbHwConfig->blkWeight[129]  =54;
    awbHwConfig->blkWeight[130]  =42;
    awbHwConfig->blkWeight[131]  =30;
    awbHwConfig->blkWeight[132]  =18;
    awbHwConfig->blkWeight[133]  =12;
    awbHwConfig->blkWeight[134]  =8;
    awbHwConfig->blkWeight[135]  =8;
    awbHwConfig->blkWeight[136]  =10;
    awbHwConfig->blkWeight[137]  =16;
    awbHwConfig->blkWeight[138]  =26;
    awbHwConfig->blkWeight[139]  =36;
    awbHwConfig->blkWeight[140]  =46;
    awbHwConfig->blkWeight[141]  =54;
    awbHwConfig->blkWeight[142]  =58;
    awbHwConfig->blkWeight[143]  =54;
    awbHwConfig->blkWeight[144]  =46;
    awbHwConfig->blkWeight[145]  =36;
    awbHwConfig->blkWeight[146]  =26;
    awbHwConfig->blkWeight[147]  =16;
    awbHwConfig->blkWeight[148]  =10;
    awbHwConfig->blkWeight[149]  =8;
    awbHwConfig->blkWeight[150]  =8;
    awbHwConfig->blkWeight[151]  =10;
    awbHwConfig->blkWeight[152]  =14;
    awbHwConfig->blkWeight[153]  =22;
    awbHwConfig->blkWeight[154]  =28;
    awbHwConfig->blkWeight[155]  =36;
    awbHwConfig->blkWeight[156]  =42;
    awbHwConfig->blkWeight[157]  =46;
    awbHwConfig->blkWeight[158]  =42;
    awbHwConfig->blkWeight[159]  =36;
    awbHwConfig->blkWeight[160]  =28;
    awbHwConfig->blkWeight[161]  =22;
    awbHwConfig->blkWeight[162]  =14;
    awbHwConfig->blkWeight[163]  =10;
    awbHwConfig->blkWeight[164]  =8;
    awbHwConfig->blkWeight[165]  =8;
    awbHwConfig->blkWeight[166]  =8;
    awbHwConfig->blkWeight[167]  =12;
    awbHwConfig->blkWeight[168]  =16;
    awbHwConfig->blkWeight[169]  =22;
    awbHwConfig->blkWeight[170]  =26;
    awbHwConfig->blkWeight[171]  =30;
    awbHwConfig->blkWeight[172]  =32;
    awbHwConfig->blkWeight[173]  =30;
    awbHwConfig->blkWeight[174]  =26;
    awbHwConfig->blkWeight[175]  =22;
    awbHwConfig->blkWeight[176]  =16;
    awbHwConfig->blkWeight[177]  =12;
    awbHwConfig->blkWeight[178]  =8;
    awbHwConfig->blkWeight[179]  =8;
    awbHwConfig->blkWeight[180]  =6;
    awbHwConfig->blkWeight[181]  =8;
    awbHwConfig->blkWeight[182]  =10;
    awbHwConfig->blkWeight[183]  =12;
    awbHwConfig->blkWeight[184]  =14;
    awbHwConfig->blkWeight[185]  =16;
    awbHwConfig->blkWeight[186]  =18;
    awbHwConfig->blkWeight[187]  =20;
    awbHwConfig->blkWeight[188]  =18;
    awbHwConfig->blkWeight[189]  =16;
    awbHwConfig->blkWeight[190]  =14;
    awbHwConfig->blkWeight[191]  =12;
    awbHwConfig->blkWeight[192]  =10;
    awbHwConfig->blkWeight[193]  =8;
    awbHwConfig->blkWeight[194]  =6;
    awbHwConfig->blkWeight[195]  =6;
    awbHwConfig->blkWeight[196]  =6;
    awbHwConfig->blkWeight[197]  =8;
    awbHwConfig->blkWeight[198]  =8;
    awbHwConfig->blkWeight[199]  =10;
    awbHwConfig->blkWeight[200]  =10;
    awbHwConfig->blkWeight[201]  =12;
    awbHwConfig->blkWeight[202]  =12;
    awbHwConfig->blkWeight[203]  =12;
    awbHwConfig->blkWeight[204]  =10;
    awbHwConfig->blkWeight[205]  =10;
    awbHwConfig->blkWeight[206]  =8;
    awbHwConfig->blkWeight[207]  =8;
    awbHwConfig->blkWeight[208]  =6;
    awbHwConfig->blkWeight[209]  =6;
    awbHwConfig->blkWeight[210]  =6;
    awbHwConfig->blkWeight[211]  =6;
    awbHwConfig->blkWeight[212]  =6;
    awbHwConfig->blkWeight[213]  =8;
    awbHwConfig->blkWeight[214]  =8;
    awbHwConfig->blkWeight[215]  =8;
    awbHwConfig->blkWeight[216]  =8;
    awbHwConfig->blkWeight[217]  =10;
    awbHwConfig->blkWeight[218]  =8;
    awbHwConfig->blkWeight[219]  =8;
    awbHwConfig->blkWeight[220]  =8;
    awbHwConfig->blkWeight[221]  =8;
    awbHwConfig->blkWeight[222]  =6;
    awbHwConfig->blkWeight[223]  =6;
    awbHwConfig->blkWeight[224]  =6;

    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn initAwbHwFullConfigGw(rk_aiq_isp_awb_meas_cfg_v3x_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_isp_awb_meas_cfg_v3x_t));
    awbHwConfig->awbEnable             =    1;
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    0;
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    0;
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]    =    0;
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]    =    0;
    awbHwConfig->lscBypEnable    =    0;
    awbHwConfig->blkStatisticsEnable    =    1;
    awbHwConfig->blkMeasureMode     =    (rk_aiq_awb_blk_stat_mode_v201_t)0;
    awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)7;
    awbHwConfig->blkStatisticsWithLumaWeightEn  =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =    0;
    awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->lightNum      =    7;
    awbHwConfig->windowSet[0]         =    0;
    awbHwConfig->windowSet[1]          =    0;
    awbHwConfig->windowSet[2]         =    3840;
    awbHwConfig->windowSet[3]         =    2160;
    awbHwConfig->dsMode      =    (rk_aiq_down_scale_mode_t)1;
    awbHwConfig->maxR          =    230;
    awbHwConfig->maxG          =    230;
    awbHwConfig->maxB          =    230;
    awbHwConfig->maxY          =    230;
    awbHwConfig->minR          =    3;
    awbHwConfig->minG          =    3;
    awbHwConfig->minB          =    3;
    awbHwConfig->minY          =    3;
    awbHwConfig->pre_wbgain_inv_r  =    256;
    awbHwConfig->pre_wbgain_inv_g  =    256;
    awbHwConfig->pre_wbgain_inv_b  =    256;
#ifdef ISP_HW_V30
    awbHwConfig->multiwindow_en  =    0;
#endif
    awbHwConfig->blk_rtdw_measure_en  =    0;
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}

static XCamReturn initAwbHwFullConfigWp(rk_aiq_isp_awb_meas_cfg_v3x_t  *awbHwConfig)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    memset(awbHwConfig,0,sizeof(rk_aiq_isp_awb_meas_cfg_v3x_t));
    awbHwConfig->awbEnable             =    1;//rawawb enable
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    1;//xy detect  enable for all light
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    1;//uv detect  enable for all  light
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    1;//uv detect  enable for all  light
    awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    1;//xy detect  enable for all light
    awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    1;//uv detect  enable for all  light
    awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    1;//uv detect  enable for all  light
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]    =    0;
    awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]    =    0;
    awbHwConfig->lscBypEnable    =    0;
    awbHwConfig->blkStatisticsEnable    =    1;
    awbHwConfig->blkMeasureMode     =    (rk_aiq_awb_blk_stat_mode_v201_t)0;
    awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)7;
    awbHwConfig->blkStatisticsWithLumaWeightEn  =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =    0;
    awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =    0;
    awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)0;
    awbHwConfig->threeDyuvIllu[0]       =    7;
    awbHwConfig->threeDyuvIllu[1]       =    7;
    awbHwConfig->threeDyuvIllu[2]       =    7;
    awbHwConfig->threeDyuvIllu[3]       =    7;
    awbHwConfig->lightNum      =    7; //measurment light num ,0~7
    awbHwConfig->windowSet[0]         =    0;//crop h_offs mod 2 == 0
    awbHwConfig->windowSet[1]          =    0;//crop v_offs mod 2 == 0
    awbHwConfig->windowSet[2]         =    3840;//crop h_size,8x8:mod 8==0,4x4:mod 4==0
    awbHwConfig->windowSet[3]         =    2160;//crop v_size,8x8:mod 8==0,4x4:mod 4==0
    awbHwConfig->dsMode      =    (rk_aiq_down_scale_mode_t)1;//downscale,1:8x8,0:4x4
    awbHwConfig->maxR          =    230;
    awbHwConfig->maxG          =    230;
    awbHwConfig->maxB          =    230;
    awbHwConfig->maxY          =    230;
    awbHwConfig->minR          =    3;
    awbHwConfig->minG          =    3;
    awbHwConfig->minB          =    3;
    awbHwConfig->minY          =    3;
    awbHwConfig->uvRange_param[0].pu_region[0]    =    247;
    awbHwConfig->uvRange_param[0].pv_region[0]    =    252;
    awbHwConfig->uvRange_param[0].pu_region[1]    =    114;
    awbHwConfig->uvRange_param[0].pv_region[1]    =    206;
    awbHwConfig->uvRange_param[0].pu_region[2]    =    105;
    awbHwConfig->uvRange_param[0].pv_region[2]    =    243;
    awbHwConfig->uvRange_param[0].pu_region[3]    =    246;
    awbHwConfig->uvRange_param[0].pv_region[3]    =    254;
    awbHwConfig->uvRange_param[0].slope_inv[0]      =    2961;
    awbHwConfig->uvRange_param[0].slope_inv[1]      =    -249;
    awbHwConfig->uvRange_param[0].slope_inv[2]      =    13126;
    awbHwConfig->uvRange_param[0].slope_inv[3]      =    -512;
    awbHwConfig->uvRange_param[1].pu_region[0]    =    251;
    awbHwConfig->uvRange_param[1].pv_region[0]    =    249;
    awbHwConfig->uvRange_param[1].pu_region[1]    =    159;
    awbHwConfig->uvRange_param[1].pv_region[1]    =    160;
    awbHwConfig->uvRange_param[1].pu_region[2]    =    141;
    awbHwConfig->uvRange_param[1].pv_region[2]    =    173;
    awbHwConfig->uvRange_param[1].pu_region[3]    =    249;
    awbHwConfig->uvRange_param[1].pv_region[3]    =    250;
    awbHwConfig->uvRange_param[1].slope_inv[0]      =    1059;
    awbHwConfig->uvRange_param[1].slope_inv[1]      =    -1418;
    awbHwConfig->uvRange_param[1].slope_inv[2]      =    1436;
    awbHwConfig->uvRange_param[1].slope_inv[3]      =    -2048;
    awbHwConfig->uvRange_param[2].pu_region[0]    =    252;
    awbHwConfig->uvRange_param[2].pv_region[0]    =    249;
    awbHwConfig->uvRange_param[2].pu_region[1]    =    180;
    awbHwConfig->uvRange_param[2].pv_region[1]    =    148;
    awbHwConfig->uvRange_param[2].pu_region[2]    =    158;
    awbHwConfig->uvRange_param[2].pv_region[2]    =    160;
    awbHwConfig->uvRange_param[2].pu_region[3]    =    250;
    awbHwConfig->uvRange_param[2].pv_region[3]    =    249;
    awbHwConfig->uvRange_param[2].slope_inv[0]      =    730;
    awbHwConfig->uvRange_param[2].slope_inv[1]      =    -1877;
    awbHwConfig->uvRange_param[2].slope_inv[2]      =    1059;
    awbHwConfig->uvRange_param[2].slope_inv[3]      =    262143;
    awbHwConfig->uvRange_param[3].pu_region[0]    =    254;
    awbHwConfig->uvRange_param[3].pv_region[0]    =    247;
    awbHwConfig->uvRange_param[3].pu_region[1]    =    204;
    awbHwConfig->uvRange_param[3].pv_region[1]    =    134;
    awbHwConfig->uvRange_param[3].pu_region[2]    =    180;
    awbHwConfig->uvRange_param[3].pv_region[2]    =    148;
    awbHwConfig->uvRange_param[3].pu_region[3]    =    252;
    awbHwConfig->uvRange_param[3].pv_region[3]    =    249;
    awbHwConfig->uvRange_param[3].slope_inv[0]      =    453;
    awbHwConfig->uvRange_param[3].slope_inv[1]      =    -1755;
    awbHwConfig->uvRange_param[3].slope_inv[2]      =    730;
    awbHwConfig->uvRange_param[3].slope_inv[3]      =    -1024;
    awbHwConfig->uvRange_param[4].pu_region[0]    =    255;
    awbHwConfig->uvRange_param[4].pv_region[0]    =    247;
    awbHwConfig->uvRange_param[4].pu_region[1]    =    225;
    awbHwConfig->uvRange_param[4].pv_region[1]    =    129;
    awbHwConfig->uvRange_param[4].pu_region[2]    =    201;
    awbHwConfig->uvRange_param[4].pv_region[2]    =    134;
    awbHwConfig->uvRange_param[4].pu_region[3]    =    254;
    awbHwConfig->uvRange_param[4].pv_region[3]    =    247;
    awbHwConfig->uvRange_param[4].slope_inv[0]      =    260;
    awbHwConfig->uvRange_param[4].slope_inv[1]      =    -4915;
    awbHwConfig->uvRange_param[4].slope_inv[2]      =    480;
    awbHwConfig->uvRange_param[4].slope_inv[3]      =    262143;
    awbHwConfig->uvRange_param[5].pu_region[0]    =    246;
    awbHwConfig->uvRange_param[5].pv_region[0]    =    257;
    awbHwConfig->uvRange_param[5].pu_region[1]    =    101;
    awbHwConfig->uvRange_param[5].pv_region[1]    =    278;
    awbHwConfig->uvRange_param[5].pu_region[2]    =    104;
    awbHwConfig->uvRange_param[5].pv_region[2]    =    243;
    awbHwConfig->uvRange_param[5].pu_region[3]    =    246;
    awbHwConfig->uvRange_param[5].pv_region[3]    =    254;
    awbHwConfig->uvRange_param[5].slope_inv[0]      =    -7070;
    awbHwConfig->uvRange_param[5].slope_inv[1]      =    -88;
    awbHwConfig->uvRange_param[5].slope_inv[2]      =    13219;
    awbHwConfig->uvRange_param[5].slope_inv[3]      =    0;
    awbHwConfig->uvRange_param[6].pu_region[0]    =    250;
    awbHwConfig->uvRange_param[6].pv_region[0]    =    250;
    awbHwConfig->uvRange_param[6].pu_region[1]    =    144;
    awbHwConfig->uvRange_param[6].pv_region[1]    =    171;
    awbHwConfig->uvRange_param[6].pu_region[2]    =    114;
    awbHwConfig->uvRange_param[6].pv_region[2]    =    207;
    awbHwConfig->uvRange_param[6].pu_region[3]    =    247;
    awbHwConfig->uvRange_param[6].pv_region[3]    =    252;
    awbHwConfig->uvRange_param[6].slope_inv[0]      =    1374;
    awbHwConfig->uvRange_param[6].slope_inv[1]      =    -853;
    awbHwConfig->uvRange_param[6].slope_inv[2]      =    3026;
    awbHwConfig->uvRange_param[6].slope_inv[3]      =    -1536;
    awbHwConfig->icrgb2RYuv_matrix[0]  =        12;
    awbHwConfig->icrgb2RYuv_matrix[1]  =        69;
    awbHwConfig->icrgb2RYuv_matrix[2]  =        7;
    awbHwConfig->icrgb2RYuv_matrix[3]  =        665;
    awbHwConfig->icrgb2RYuv_matrix[4]  =        -44;
    awbHwConfig->icrgb2RYuv_matrix[5]  =        -2;
    awbHwConfig->icrgb2RYuv_matrix[6]  =        32;
    awbHwConfig->icrgb2RYuv_matrix[7]  =        2268;
    awbHwConfig->icrgb2RYuv_matrix[8]  =        23;
    awbHwConfig->icrgb2RYuv_matrix[9]  =        -24;
    awbHwConfig->icrgb2RYuv_matrix[10]  =        27;
    awbHwConfig->icrgb2RYuv_matrix[11]  =        1690;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[0].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[1].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[0]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[1]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[2]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[3]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[4]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[5]  =        0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].lineP1[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[0]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[1]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[2]  =    0;
    awbHwConfig->ic3Dyuv2Range_param[2].disP1P2  =    0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[0]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[1]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[2]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[3]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[4]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[5]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[0]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[1]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[2]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[3]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[4]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[5]  =       0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[0]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[1]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].lineP1[2]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[0]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[1]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[2]  =   0;
    awbHwConfig->ic3Dyuv2Range_param[3].disP1P2  =   0;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[0]  =    1534;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[1]  =    1535;
    awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[2]  =    1027;
    awbHwConfig->rgb2xy_param.rotationMat[0]  =    -2202;
    awbHwConfig->rgb2xy_param.rotationMat[1]  =    3454;
    awbHwConfig->rgb2xy_param.rotationMat[2]  =    -535;
    awbHwConfig->rgb2xy_param.rotationMat[3]  =    3454;
    awbHwConfig->rgb2xy_param.rotationMat[4]  =    2202;
    awbHwConfig->rgb2xy_param.rotationMat[5]  =    2826;
    awbHwConfig->xyRange_param[0].NorrangeX[0]  =    -1432;
    awbHwConfig->xyRange_param[0].NorrangeX[1]  =    -962;
    awbHwConfig->xyRange_param[0].NorrangeY[0]  =    134;
    awbHwConfig->xyRange_param[0].NorrangeY[1]  =    -30;
    awbHwConfig->xyRange_param[0].SperangeX[0]  =    -1432;
    awbHwConfig->xyRange_param[0].SperangeX[1]  =    -962;
    awbHwConfig->xyRange_param[0].SperangeY[0]  =    165;
    awbHwConfig->xyRange_param[0].SperangeY[1]  =    -60;
    awbHwConfig->xyRange_param[1].NorrangeX[0]  =    -959;
    awbHwConfig->xyRange_param[1].NorrangeX[1]  =    -507;
    awbHwConfig->xyRange_param[1].NorrangeY[0]  =    -46;
    awbHwConfig->xyRange_param[1].NorrangeY[1]  =    -159;
    awbHwConfig->xyRange_param[1].SperangeX[0]  =    -961;
    awbHwConfig->xyRange_param[1].SperangeX[1]  =    -507;
    awbHwConfig->xyRange_param[1].SperangeY[0]  =    -46;
    awbHwConfig->xyRange_param[1].SperangeY[1]  =    -179;
    awbHwConfig->xyRange_param[2].NorrangeX[0]  =    -507;
    awbHwConfig->xyRange_param[2].NorrangeX[1]  =    -160;
    awbHwConfig->xyRange_param[2].NorrangeY[0]  =    181;
    awbHwConfig->xyRange_param[2].NorrangeY[1]  =    10;
    awbHwConfig->xyRange_param[2].SperangeX[0]  =    -507;
    awbHwConfig->xyRange_param[2].SperangeX[1]  =    -160;
    awbHwConfig->xyRange_param[2].SperangeY[0]  =    205;
    awbHwConfig->xyRange_param[2].SperangeY[1]  =    -20;
    awbHwConfig->xyRange_param[3].NorrangeX[0]  =    -160;
    awbHwConfig->xyRange_param[3].NorrangeX[1]  =    74;
    awbHwConfig->xyRange_param[3].NorrangeY[0]  =    79;
    awbHwConfig->xyRange_param[3].NorrangeY[1]  =    -85;
    awbHwConfig->xyRange_param[3].SperangeX[0]  =    -160;
    awbHwConfig->xyRange_param[3].SperangeX[1]  =    74;
    awbHwConfig->xyRange_param[3].SperangeY[0]  =    109;
    awbHwConfig->xyRange_param[3].SperangeY[1]  =    -103;
    awbHwConfig->xyRange_param[4].NorrangeX[0]  =    74;
    awbHwConfig->xyRange_param[4].NorrangeX[1]  =    227;
    awbHwConfig->xyRange_param[4].NorrangeY[0]  =    102;
    awbHwConfig->xyRange_param[4].NorrangeY[1]  =    -66;
    awbHwConfig->xyRange_param[4].SperangeX[0]  =    74;
    awbHwConfig->xyRange_param[4].SperangeX[1]  =    227;
    awbHwConfig->xyRange_param[4].SperangeY[0]  =    133;
    awbHwConfig->xyRange_param[4].SperangeY[1]  =    -86;
    awbHwConfig->xyRange_param[5].NorrangeX[0]  =    -1742;
    awbHwConfig->xyRange_param[5].NorrangeX[1]  =    -1432;
    awbHwConfig->xyRange_param[5].NorrangeY[0]  =    90;
    awbHwConfig->xyRange_param[5].NorrangeY[1]  =    -80;
    awbHwConfig->xyRange_param[5].SperangeX[0]  =    -1742;
    awbHwConfig->xyRange_param[5].SperangeX[1]  =    -1432;
    awbHwConfig->xyRange_param[5].SperangeY[0]  =    121;
    awbHwConfig->xyRange_param[5].SperangeY[1]  =    -111;
    awbHwConfig->xyRange_param[6].NorrangeX[0]  =    -962;
    awbHwConfig->xyRange_param[6].NorrangeX[1]  =    -509;
    awbHwConfig->xyRange_param[6].NorrangeY[0]  =    114;
    awbHwConfig->xyRange_param[6].NorrangeY[1]  =    -43;
    awbHwConfig->xyRange_param[6].SperangeX[0]  =    -962;
    awbHwConfig->xyRange_param[6].SperangeX[1]  =    -509;
    awbHwConfig->xyRange_param[6].SperangeY[0]  =    134;
    awbHwConfig->xyRange_param[6].SperangeY[1]  =    -45;
    awbHwConfig->pre_wbgain_inv_r  =    256;
    awbHwConfig->pre_wbgain_inv_g  =    256;
    awbHwConfig->pre_wbgain_inv_b  =    256;
#ifdef ISP_HW_V30
    awbHwConfig->multiwindow_en  =    0;
    awbHwConfig->multiwindow[0][0]  =    0;
    awbHwConfig->multiwindow[0][1]  =    0;
    awbHwConfig->multiwindow[0][2]  =    0;
    awbHwConfig->multiwindow[0][3]  =    0;
    awbHwConfig->multiwindow[1][0]  =    0;
    awbHwConfig->multiwindow[1][1]  =    0;
    awbHwConfig->multiwindow[1][2]  =    0;
    awbHwConfig->multiwindow[1][3]  =    0;
    awbHwConfig->multiwindow[2][0]  =    0;
    awbHwConfig->multiwindow[2][1]  =    0;
    awbHwConfig->multiwindow[2][2]  =    0;
    awbHwConfig->multiwindow[2][3]  =    0;
    awbHwConfig->multiwindow[3][0]  =    0;
    awbHwConfig->multiwindow[3][1]  =    0;
    awbHwConfig->multiwindow[3][2]  =    0;
    awbHwConfig->multiwindow[3][3]  =    0;
#endif
    awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[0].measureEnable   =     0;
    awbHwConfig->excludeWpRange[0].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[0].xu[0]           =     0;
    awbHwConfig->excludeWpRange[0].xu[1]           =     0;
    awbHwConfig->excludeWpRange[0].yv[0]           =     0;
    awbHwConfig->excludeWpRange[0].yv[1]           =     0;
    awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[1].measureEnable   =     0;
    awbHwConfig->excludeWpRange[1].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[1].xu[0]           =     0;
    awbHwConfig->excludeWpRange[1].xu[1]           =     0;
    awbHwConfig->excludeWpRange[1].yv[0]           =     0;
    awbHwConfig->excludeWpRange[1].yv[1]           =     0;
    awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[2].measureEnable   =     0;
    awbHwConfig->excludeWpRange[2].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[2].xu[0]           =     0;
    awbHwConfig->excludeWpRange[2].xu[1]           =     0;
    awbHwConfig->excludeWpRange[2].yv[0]           =     0;
    awbHwConfig->excludeWpRange[2].yv[1]           =     0;
    awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[3].measureEnable   =     0;
    awbHwConfig->excludeWpRange[3].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[3].xu[0]           =     0;
    awbHwConfig->excludeWpRange[3].xu[1]           =     0;
    awbHwConfig->excludeWpRange[3].yv[0]           =     0;
    awbHwConfig->excludeWpRange[3].yv[1]           =     0;
    awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[4].measureEnable   =     0;
    awbHwConfig->excludeWpRange[4].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[4].xu[0]           =     0;
    awbHwConfig->excludeWpRange[4].xu[1]           =     0;
    awbHwConfig->excludeWpRange[4].yv[0]           =     0;
    awbHwConfig->excludeWpRange[4].yv[1]           =     0;
    awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[5].measureEnable   =     0;
    awbHwConfig->excludeWpRange[5].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[5].xu[0]           =     0;
    awbHwConfig->excludeWpRange[5].xu[1]           =     0;
    awbHwConfig->excludeWpRange[5].yv[0]           =     0;
    awbHwConfig->excludeWpRange[5].yv[1]           =     0;
    awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     1;
    awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     1;
    awbHwConfig->excludeWpRange[6].measureEnable   =     0;
    awbHwConfig->excludeWpRange[6].domain          =     (rk_aiq_awb_exc_range_domain_t)0;
    awbHwConfig->excludeWpRange[6].xu[0]           =     0;
    awbHwConfig->excludeWpRange[6].xu[1]           =     0;
    awbHwConfig->excludeWpRange[6].yv[0]           =     0;
    awbHwConfig->excludeWpRange[6].yv[1]           =     0;
    awbHwConfig->wpDiffwei_y[0]  =0;
    awbHwConfig->wpDiffwei_y[1]  =16;
    awbHwConfig->wpDiffwei_y[2]  =32;
    awbHwConfig->wpDiffwei_y[3]  =64;
    awbHwConfig->wpDiffwei_y[4]  =96;
    awbHwConfig->wpDiffwei_y[5]  =128;
    awbHwConfig->wpDiffwei_y[6]  =192;
    awbHwConfig->wpDiffwei_y[7]  =224;
    awbHwConfig->wpDiffwei_y[8]  =240;
    awbHwConfig->wpDiffwei_w[0]  =31;
    awbHwConfig->wpDiffwei_w[1]  =31;
    awbHwConfig->wpDiffwei_w[2]  =31;
    awbHwConfig->wpDiffwei_w[3]  =31;
    awbHwConfig->wpDiffwei_w[4]  =31;
    awbHwConfig->wpDiffwei_w[5]  =31;
    awbHwConfig->wpDiffwei_w[6]  =31;
    awbHwConfig->wpDiffwei_w[7]  =31;
    awbHwConfig->wpDiffwei_w[8]  =31;
    awbHwConfig->blkWeight[0]  =6;
    awbHwConfig->blkWeight[1]  =6;
    awbHwConfig->blkWeight[2]  =6;
    awbHwConfig->blkWeight[3]  =8;
    awbHwConfig->blkWeight[4]  =8;
    awbHwConfig->blkWeight[5]  =8;
    awbHwConfig->blkWeight[6]  =8;
    awbHwConfig->blkWeight[7]  =10;
    awbHwConfig->blkWeight[8]  =8;
    awbHwConfig->blkWeight[9]  =8;
    awbHwConfig->blkWeight[10]  =8;
    awbHwConfig->blkWeight[11]  =8;
    awbHwConfig->blkWeight[12]  =6;
    awbHwConfig->blkWeight[13]  =6;
    awbHwConfig->blkWeight[14]  =6;
    awbHwConfig->blkWeight[15]  =6;
    awbHwConfig->blkWeight[16]  =6;
    awbHwConfig->blkWeight[17]  =8;
    awbHwConfig->blkWeight[18]  =8;
    awbHwConfig->blkWeight[19]  =10;
    awbHwConfig->blkWeight[20]  =10;
    awbHwConfig->blkWeight[21]  =12;
    awbHwConfig->blkWeight[22]  =12;
    awbHwConfig->blkWeight[23]  =12;
    awbHwConfig->blkWeight[24]  =10;
    awbHwConfig->blkWeight[25]  =10;
    awbHwConfig->blkWeight[26]  =8;
    awbHwConfig->blkWeight[27]  =8;
    awbHwConfig->blkWeight[28]  =6;
    awbHwConfig->blkWeight[29]  =6;
    awbHwConfig->blkWeight[30]  =6;
    awbHwConfig->blkWeight[31]  =8;
    awbHwConfig->blkWeight[32]  =10;
    awbHwConfig->blkWeight[33]  =12;
    awbHwConfig->blkWeight[34]  =14;
    awbHwConfig->blkWeight[35]  =16;
    awbHwConfig->blkWeight[36]  =18;
    awbHwConfig->blkWeight[37]  =20;
    awbHwConfig->blkWeight[38]  =18;
    awbHwConfig->blkWeight[39]  =16;
    awbHwConfig->blkWeight[40]  =14;
    awbHwConfig->blkWeight[41]  =12;
    awbHwConfig->blkWeight[42]  =10;
    awbHwConfig->blkWeight[43]  =8;
    awbHwConfig->blkWeight[44]  =6;
    awbHwConfig->blkWeight[45]  =8;
    awbHwConfig->blkWeight[46]  =8;
    awbHwConfig->blkWeight[47]  =12;
    awbHwConfig->blkWeight[48]  =16;
    awbHwConfig->blkWeight[49]  =22;
    awbHwConfig->blkWeight[50]  =26;
    awbHwConfig->blkWeight[51]  =30;
    awbHwConfig->blkWeight[52]  =32;
    awbHwConfig->blkWeight[53]  =30;
    awbHwConfig->blkWeight[54]  =26;
    awbHwConfig->blkWeight[55]  =22;
    awbHwConfig->blkWeight[56]  =16;
    awbHwConfig->blkWeight[57]  =12;
    awbHwConfig->blkWeight[58]  =8;
    awbHwConfig->blkWeight[59]  =8;
    awbHwConfig->blkWeight[60]  =8;
    awbHwConfig->blkWeight[61]  =10;
    awbHwConfig->blkWeight[62]  =14;
    awbHwConfig->blkWeight[63]  =22;
    awbHwConfig->blkWeight[64]  =28;
    awbHwConfig->blkWeight[65]  =36;
    awbHwConfig->blkWeight[66]  =42;
    awbHwConfig->blkWeight[67]  =46;
    awbHwConfig->blkWeight[68]  =42;
    awbHwConfig->blkWeight[69]  =36;
    awbHwConfig->blkWeight[70]  =28;
    awbHwConfig->blkWeight[71]  =22;
    awbHwConfig->blkWeight[72]  =14;
    awbHwConfig->blkWeight[73]  =10;
    awbHwConfig->blkWeight[74]  =8;
    awbHwConfig->blkWeight[75]  =8;
    awbHwConfig->blkWeight[76]  =10;
    awbHwConfig->blkWeight[77]  =16;
    awbHwConfig->blkWeight[78]  =26;
    awbHwConfig->blkWeight[79]  =36;
    awbHwConfig->blkWeight[80]  =46;
    awbHwConfig->blkWeight[81]  =54;
    awbHwConfig->blkWeight[82]  =58;
    awbHwConfig->blkWeight[83]  =54;
    awbHwConfig->blkWeight[84]  =46;
    awbHwConfig->blkWeight[85]  =36;
    awbHwConfig->blkWeight[86]  =26;
    awbHwConfig->blkWeight[87]  =16;
    awbHwConfig->blkWeight[88]  =10;
    awbHwConfig->blkWeight[89]  =8;
    awbHwConfig->blkWeight[90]  =8;
    awbHwConfig->blkWeight[91]  =12;
    awbHwConfig->blkWeight[92]  =18;
    awbHwConfig->blkWeight[93]  =30;
    awbHwConfig->blkWeight[94]  =42;
    awbHwConfig->blkWeight[95]  =54;
    awbHwConfig->blkWeight[96]  =63;
    awbHwConfig->blkWeight[97]  =63;
    awbHwConfig->blkWeight[98]  =63;
    awbHwConfig->blkWeight[99]  =54;
    awbHwConfig->blkWeight[100]  =42;
    awbHwConfig->blkWeight[101]  =30;
    awbHwConfig->blkWeight[102]  =18;
    awbHwConfig->blkWeight[103]  =12;
    awbHwConfig->blkWeight[104]  =8;
    awbHwConfig->blkWeight[105]  =10;
    awbHwConfig->blkWeight[106]  =12;
    awbHwConfig->blkWeight[107]  =20;
    awbHwConfig->blkWeight[108]  =32;
    awbHwConfig->blkWeight[109]  =46;
    awbHwConfig->blkWeight[110]  =58;
    awbHwConfig->blkWeight[111]  =63;
    awbHwConfig->blkWeight[112]  =63;
    awbHwConfig->blkWeight[113]  =63;
    awbHwConfig->blkWeight[114]  =58;
    awbHwConfig->blkWeight[115]  =46;
    awbHwConfig->blkWeight[116]  =32;
    awbHwConfig->blkWeight[117]  =20;
    awbHwConfig->blkWeight[118]  =12;
    awbHwConfig->blkWeight[119]  =10;
    awbHwConfig->blkWeight[120]  =8;
    awbHwConfig->blkWeight[121]  =12;
    awbHwConfig->blkWeight[122]  =18;
    awbHwConfig->blkWeight[123]  =30;
    awbHwConfig->blkWeight[124]  =42;
    awbHwConfig->blkWeight[125]  =54;
    awbHwConfig->blkWeight[126]  =63;
    awbHwConfig->blkWeight[127]  =63;
    awbHwConfig->blkWeight[128]  =63;
    awbHwConfig->blkWeight[129]  =54;
    awbHwConfig->blkWeight[130]  =42;
    awbHwConfig->blkWeight[131]  =30;
    awbHwConfig->blkWeight[132]  =18;
    awbHwConfig->blkWeight[133]  =12;
    awbHwConfig->blkWeight[134]  =8;
    awbHwConfig->blkWeight[135]  =8;
    awbHwConfig->blkWeight[136]  =10;
    awbHwConfig->blkWeight[137]  =16;
    awbHwConfig->blkWeight[138]  =26;
    awbHwConfig->blkWeight[139]  =36;
    awbHwConfig->blkWeight[140]  =46;
    awbHwConfig->blkWeight[141]  =54;
    awbHwConfig->blkWeight[142]  =58;
    awbHwConfig->blkWeight[143]  =54;
    awbHwConfig->blkWeight[144]  =46;
    awbHwConfig->blkWeight[145]  =36;
    awbHwConfig->blkWeight[146]  =26;
    awbHwConfig->blkWeight[147]  =16;
    awbHwConfig->blkWeight[148]  =10;
    awbHwConfig->blkWeight[149]  =8;
    awbHwConfig->blkWeight[150]  =8;
    awbHwConfig->blkWeight[151]  =10;
    awbHwConfig->blkWeight[152]  =14;
    awbHwConfig->blkWeight[153]  =22;
    awbHwConfig->blkWeight[154]  =28;
    awbHwConfig->blkWeight[155]  =36;
    awbHwConfig->blkWeight[156]  =42;
    awbHwConfig->blkWeight[157]  =46;
    awbHwConfig->blkWeight[158]  =42;
    awbHwConfig->blkWeight[159]  =36;
    awbHwConfig->blkWeight[160]  =28;
    awbHwConfig->blkWeight[161]  =22;
    awbHwConfig->blkWeight[162]  =14;
    awbHwConfig->blkWeight[163]  =10;
    awbHwConfig->blkWeight[164]  =8;
    awbHwConfig->blkWeight[165]  =8;
    awbHwConfig->blkWeight[166]  =8;
    awbHwConfig->blkWeight[167]  =12;
    awbHwConfig->blkWeight[168]  =16;
    awbHwConfig->blkWeight[169]  =22;
    awbHwConfig->blkWeight[170]  =26;
    awbHwConfig->blkWeight[171]  =30;
    awbHwConfig->blkWeight[172]  =32;
    awbHwConfig->blkWeight[173]  =30;
    awbHwConfig->blkWeight[174]  =26;
    awbHwConfig->blkWeight[175]  =22;
    awbHwConfig->blkWeight[176]  =16;
    awbHwConfig->blkWeight[177]  =12;
    awbHwConfig->blkWeight[178]  =8;
    awbHwConfig->blkWeight[179]  =8;
    awbHwConfig->blkWeight[180]  =6;
    awbHwConfig->blkWeight[181]  =8;
    awbHwConfig->blkWeight[182]  =10;
    awbHwConfig->blkWeight[183]  =12;
    awbHwConfig->blkWeight[184]  =14;
    awbHwConfig->blkWeight[185]  =16;
    awbHwConfig->blkWeight[186]  =18;
    awbHwConfig->blkWeight[187]  =20;
    awbHwConfig->blkWeight[188]  =18;
    awbHwConfig->blkWeight[189]  =16;
    awbHwConfig->blkWeight[190]  =14;
    awbHwConfig->blkWeight[191]  =12;
    awbHwConfig->blkWeight[192]  =10;
    awbHwConfig->blkWeight[193]  =8;
    awbHwConfig->blkWeight[194]  =6;
    awbHwConfig->blkWeight[195]  =6;
    awbHwConfig->blkWeight[196]  =6;
    awbHwConfig->blkWeight[197]  =8;
    awbHwConfig->blkWeight[198]  =8;
    awbHwConfig->blkWeight[199]  =10;
    awbHwConfig->blkWeight[200]  =10;
    awbHwConfig->blkWeight[201]  =12;
    awbHwConfig->blkWeight[202]  =12;
    awbHwConfig->blkWeight[203]  =12;
    awbHwConfig->blkWeight[204]  =10;
    awbHwConfig->blkWeight[205]  =10;
    awbHwConfig->blkWeight[206]  =8;
    awbHwConfig->blkWeight[207]  =8;
    awbHwConfig->blkWeight[208]  =6;
    awbHwConfig->blkWeight[209]  =6;
    awbHwConfig->blkWeight[210]  =6;
    awbHwConfig->blkWeight[211]  =6;
    awbHwConfig->blkWeight[212]  =6;
    awbHwConfig->blkWeight[213]  =8;
    awbHwConfig->blkWeight[214]  =8;
    awbHwConfig->blkWeight[215]  =8;
    awbHwConfig->blkWeight[216]  =8;
    awbHwConfig->blkWeight[217]  =10;
    awbHwConfig->blkWeight[218]  =8;
    awbHwConfig->blkWeight[219]  =8;
    awbHwConfig->blkWeight[220]  =8;
    awbHwConfig->blkWeight[221]  =8;
    awbHwConfig->blkWeight[222]  =6;
    awbHwConfig->blkWeight[223]  =6;
    awbHwConfig->blkWeight[224]  =6;
    awbHwConfig->blk_rtdw_measure_en  =    0;

    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return ret;
}


static void initCustomAwbRes(rk_aiq_customeAwb_results_t* customAwb, rk_aiq_rkAwb_config_t* pConfig)
{
    customAwb->IsConverged = false;
    customAwb->awb_gain_algo.rgain =1.0;
    customAwb->awb_gain_algo.bgain =1.0;
    customAwb->awb_gain_algo.grgain =1.0;
    customAwb->awb_gain_algo.gbgain =1.0;
    customAwb->awb_smooth_factor = 0.5;
    rk_aiq_customAwb_hw_cfg_t  *awbHwConfig = &customAwb->awbHwConfig;
    const rk_aiq_isp_awb_meas_cfg_v3x_t  *awbHwConfigFull = &pConfig->awbHwConfig;
    awbHwConfig->awbEnable   = awbHwConfigFull->awbEnable;
    awbHwConfig->lscBypEnable   = awbHwConfigFull->lscBypEnable;
    awbHwConfig->uvDetectionEnable   = awbHwConfigFull->uvDetectionEnable[0];
    awbHwConfig->xyDetectionEnable   = awbHwConfigFull->xyDetectionEnable[0];
    awbHwConfig->threeDyuvEnable   = awbHwConfigFull->threeDyuvEnable[0];
    memcpy(awbHwConfig->threeDyuvIllu,awbHwConfigFull->threeDyuvIllu, sizeof(awbHwConfigFull->threeDyuvIllu));
    awbHwConfig->wpDiffWeiEnable   = awbHwConfigFull->wpDiffWeiEnable[0];
    awbHwConfig->blkWeightEnable   = awbHwConfigFull->blkWeightEnable[0];
    awbHwConfig->blkMeasureMode   = awbHwConfigFull->blkMeasureMode;
#ifdef ISP_HW_V30
    awbHwConfig->multiwindow_en   = awbHwConfigFull->multiwindow_en;
    memcpy(awbHwConfig->multiwindow, awbHwConfigFull->multiwindow,sizeof(awbHwConfigFull->multiwindow));
#endif
    awbHwConfig->frameChoose   = awbHwConfigFull->frameChoose;
    memcpy(awbHwConfig->windowSet,awbHwConfigFull->windowSet,sizeof(awbHwConfigFull->windowSet));
    awbHwConfig->lightNum   = awbHwConfigFull->lightNum;
    awbHwConfig->maxR   = awbHwConfigFull->maxR;
    awbHwConfig->minR   = awbHwConfigFull->minR;
    awbHwConfig->maxG   = awbHwConfigFull->maxG;
    awbHwConfig->minG   = awbHwConfigFull->minG;
    awbHwConfig->maxB   = awbHwConfigFull->maxB;
    awbHwConfig->minB   = awbHwConfigFull->minB;
    awbHwConfig->maxY   = awbHwConfigFull->maxY;
    awbHwConfig->minY   = awbHwConfigFull->minY;
    memcpy(awbHwConfig->icrgb2RYuv_matrix,awbHwConfigFull->icrgb2RYuv_matrix,sizeof(awbHwConfigFull->icrgb2RYuv_matrix));
    memcpy(awbHwConfig->ic3Dyuv2Range_param,awbHwConfigFull->ic3Dyuv2Range_param,sizeof(RK_AIQ_AWB_YUV_LS_PARA_NUM));
    memcpy(awbHwConfig->uvRange_param,awbHwConfigFull->uvRange_param,sizeof(awbHwConfigFull->uvRange_param));
    awbHwConfig->rgb2xy_param   = awbHwConfigFull->rgb2xy_param;
    memcpy(awbHwConfig->xyRange_param,awbHwConfigFull->xyRange_param,sizeof(awbHwConfigFull->xyRange_param));
    memcpy(awbHwConfig->excludeWpRange,awbHwConfigFull->excludeWpRange,sizeof(awbHwConfig->excludeWpRange));
    memcpy(awbHwConfig->wpDiffwei_y, awbHwConfigFull->wpDiffwei_y,sizeof(awbHwConfigFull->wpDiffwei_y));
    memcpy(awbHwConfig->wpDiffwei_w, awbHwConfigFull->wpDiffwei_w,sizeof(awbHwConfigFull->wpDiffwei_w));
    awbHwConfig->xyRangeTypeForWpHist   = awbHwConfigFull->xyRangeTypeForWpHist;
    memcpy(awbHwConfig->blkWeight, awbHwConfigFull->blkWeight,sizeof(awbHwConfigFull->blkWeight));
    awbHwConfig->xyRangeTypeForBlkStatistics   = awbHwConfigFull->xyRangeTypeForBlkStatistics;
    awbHwConfig->illIdxForBlkStatistics   = awbHwConfigFull->illIdxForBlkStatistics;

}

int32_t  awb_window_init(void* ctx,  rk_aiq_rkAwb_config_t *rkCfg)
{
    rkCfg->awbHwConfig.windowSet[0]=0;
    rkCfg->awbHwConfig.windowSet[1]=0;
    rkCfg->awbHwConfig.windowSet[2]=rkCfg->RawWidth;
    rkCfg->awbHwConfig.windowSet[3]=rkCfg->RawHeight;
    return 0;
}
int32_t  frame_choose_init(void* ctx, rk_aiq_rkAwb_config_t *rkCfg)
{

    if((rk_aiq_working_mode_t)rkCfg->Working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        rkCfg->awbHwConfig.frameChoose = 0;
    } else{
        rkCfg->awbHwConfig.frameChoose = 1;//Middle frame
    }

    return 0;
}

int32_t  awb_window_check(const rk_aiq_rkAwb_config_t *rkCfg, unsigned short *windowSet)
{

    if(windowSet[0] > rkCfg->RawWidth ||
            windowSet[0] + windowSet[2] > rkCfg->RawWidth ||
            windowSet[1] > rkCfg->RawHeight ||
            windowSet[1] + windowSet[3] > rkCfg->RawHeight ) {
        LOGW_AWB("windowSet[%d,%d,%d,%d]is invaild!!! Reset to full window\n", windowSet[0],
                 windowSet[1], windowSet[2], windowSet[3]);
        windowSet[0]=0;
        windowSet[1]=0;
        windowSet[2]=rkCfg->RawWidth;
        windowSet[3]=rkCfg->RawHeight;
    }

    return 0;
}



static
void _rkAwbStats2CustomAwbStats(rk_aiq_customAwb_stats_t* customAwb,
                              rk_aiq_awb_stat_res_v201_t* rkAwb)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    memcpy(customAwb->light,rkAwb->light,sizeof(customAwb->light));
#ifdef ISP_HW_V30
    memcpy(customAwb->WpNo2,rkAwb->WpNo2,sizeof(customAwb->WpNo2));
#endif
    memcpy(customAwb->blockResult,rkAwb->blockResult,sizeof(customAwb->blockResult));
#ifdef ISP_HW_V30
    memcpy(customAwb->multiwindowLightResult,rkAwb->multiwindowLightResult,sizeof(customAwb->multiwindowLightResult));
    memcpy(customAwb->excWpRangeResult,rkAwb->excWpRangeResult,sizeof(customAwb->excWpRangeResult));
#endif
    memcpy(customAwb->WpNoHist,rkAwb->WpNoHist,sizeof(customAwb->WpNoHist));
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
}

static void _customAwbHw2rkAwbHwCfg( const rk_aiq_customeAwb_results_t* customAwbProcRes, rk_aiq_isp_awb_meas_cfg_v3x_t *awbHwConfigFull)
{
    const rk_aiq_customAwb_hw_cfg_t  *awbHwConfig = &customAwbProcRes->awbHwConfig;
    awbHwConfigFull->awbEnable   = awbHwConfig->awbEnable;
    awbHwConfigFull->lscBypEnable   = awbHwConfig->lscBypEnable;
    for(int i=0;i<RK_AIQ_AWB_XY_TYPE_MAX_V201;i++){
        awbHwConfigFull->uvDetectionEnable[i]   = awbHwConfig->uvDetectionEnable;
        awbHwConfigFull->xyDetectionEnable[i]   = awbHwConfig->xyDetectionEnable;
        awbHwConfigFull->threeDyuvEnable[i]   = awbHwConfig->threeDyuvEnable;
        awbHwConfigFull->wpDiffWeiEnable[i]   = awbHwConfig->wpDiffWeiEnable;
        awbHwConfigFull->blkWeightEnable[i]   = awbHwConfig->blkWeightEnable;
    }
    memcpy(awbHwConfigFull->threeDyuvIllu,awbHwConfig->threeDyuvIllu, sizeof(awbHwConfig->threeDyuvIllu));
    awbHwConfigFull->blkMeasureMode   = awbHwConfig->blkMeasureMode;
#ifdef ISP_HW_V30
    awbHwConfigFull->multiwindow_en   = awbHwConfig->multiwindow_en;
    memcpy(awbHwConfigFull->multiwindow, awbHwConfig->multiwindow,sizeof(awbHwConfig->multiwindow));
#endif
    awbHwConfigFull->frameChoose   = awbHwConfig->frameChoose;
    memcpy(awbHwConfigFull->windowSet,awbHwConfig->windowSet,sizeof(awbHwConfig->windowSet));
    awbHwConfigFull->lightNum   = awbHwConfig->lightNum;
    awbHwConfigFull->maxR   = awbHwConfig->maxR;
    awbHwConfigFull->minR   = awbHwConfig->minR;
    awbHwConfigFull->maxG   = awbHwConfig->maxG;
    awbHwConfigFull->minG   = awbHwConfig->minG;
    awbHwConfigFull->maxB   = awbHwConfig->maxB;
    awbHwConfigFull->minB   = awbHwConfig->minB;
    awbHwConfigFull->maxY   = awbHwConfig->maxY;
    awbHwConfigFull->minY   = awbHwConfig->minY;
    memcpy(awbHwConfigFull->icrgb2RYuv_matrix,awbHwConfig->icrgb2RYuv_matrix,sizeof(awbHwConfig->icrgb2RYuv_matrix));
    memcpy(awbHwConfigFull->ic3Dyuv2Range_param,awbHwConfig->ic3Dyuv2Range_param,sizeof(RK_AIQ_AWB_YUV_LS_PARA_NUM));
    memcpy(awbHwConfigFull->uvRange_param,awbHwConfig->uvRange_param,sizeof(awbHwConfig->uvRange_param));
    awbHwConfigFull->rgb2xy_param   = awbHwConfig->rgb2xy_param;
    memcpy(awbHwConfigFull->xyRange_param,awbHwConfig->xyRange_param,sizeof(awbHwConfig->xyRange_param));
    memcpy(awbHwConfigFull->excludeWpRange,awbHwConfig->excludeWpRange,sizeof(awbHwConfigFull->excludeWpRange));
    memcpy(awbHwConfigFull->wpDiffwei_y, awbHwConfig->wpDiffwei_y,sizeof(awbHwConfig->wpDiffwei_y));
    memcpy(awbHwConfigFull->wpDiffwei_w, awbHwConfig->wpDiffwei_w,sizeof(awbHwConfig->wpDiffwei_w));
    awbHwConfigFull->xyRangeTypeForWpHist   = awbHwConfig->xyRangeTypeForWpHist;
    memcpy(awbHwConfigFull->blkWeight, awbHwConfig->blkWeight,sizeof(awbHwConfig->blkWeight));
    awbHwConfigFull->xyRangeTypeForBlkStatistics   = awbHwConfig->xyRangeTypeForBlkStatistics;
    awbHwConfigFull->illIdxForBlkStatistics   = awbHwConfig->illIdxForBlkStatistics;

}


static
void _customAwbRes2rkAwbRes( RkAiqAlgoProcResAwb* rkAwbProcRes,
                          rk_aiq_customeAwb_results_t* customAwbProcRes, const rk_aiq_isp_awb_meas_cfg_v3x_t &awbHwConfig)
{

    rkAwbProcRes->awbConverged = customAwbProcRes->IsConverged;
    *rkAwbProcRes->awb_gain_algo= customAwbProcRes->awb_gain_algo;
    rkAwbProcRes->awb_smooth_factor = customAwbProcRes->awb_smooth_factor;
    *rkAwbProcRes->awb_hw1_para=  awbHwConfig;
}

static XCamReturn _rkAwbStats2CustomGroupAwbStats( rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result , int camera_num,rk_aiq_customAwb_stats_t *customStats)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    XCamVideoBuffer* awbStatsBuf = nullptr;
    RkAiqAwbStats* xAwbStats = nullptr;
    rk_aiq_customAwb_stats_t *awbStat = customStats;
    rk_aiq_customAwb_stats_t *awbStat2 = customStats;
    for(int i=0;i<camera_num;i++){
        awbStatsBuf = rk_aiq_singlecam_3a_result[i]->awb._awbStats;
        if (awbStatsBuf) {
           xAwbStats = (RkAiqAwbStats*)awbStatsBuf->map(awbStatsBuf);
           if (!xAwbStats) {
               LOGE_AWBGROUP("awb stats is null for %dth camera",i);
               return(XCAM_RETURN_ERROR_FAILED);
           }
        } else {
           LOGE_AWBGROUP("awb stats is null for %dth camera",i);
           return(XCAM_RETURN_ERROR_FAILED);
        }
        if(i>0){
            if(awbStat->next ==nullptr){
                awbStat2 = (rk_aiq_customAwb_stats_t*)malloc(sizeof(rk_aiq_customAwb_stats_t));
                memset(awbStat2,0,sizeof(rk_aiq_customAwb_stats_t));
                awbStat->next = awbStat2;
               // LOGE_AWB_SUBM(0xff, "%s  %dth camera:,malloc awbStat:%p", __func__,i,awbStat2);
            }else{
                awbStat2 = awbStat->next;
            }
            awbStat = awbStat->next;

        }
        _rkAwbStats2CustomAwbStats(awbStat2, &xAwbStats->awb_stats_v3x);
        //WriteMeasureResult(xAwbStats->awb_stats_v3x,XCORE_LOG_LEVEL_LOW1+1,i);
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static XCamReturn customAwbStatsRelease( rk_aiq_customAwb_stats_t *customStats)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_customAwb_stats_t *awbStat = customStats->next;
    rk_aiq_customAwb_stats_t *awbStat2;
    while(awbStat!=nullptr){
        awbStat2 = awbStat->next;
        //LOGE_AWB_SUBM(0xff, "%s free awbStat %p ", __func__,awbStat);
        free(awbStat);
        awbStat = awbStat2;
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static void _customGruopAwbHw2rkAwbHwCfg( const rk_aiq_customeAwb_single_results_t* customAwbSingelRes,
    rk_aiq_isp_awb_meas_cfg_v3x_t *awbHwConfigFull,  const rk_aiq_wb_gain_t** awb_gain_algo)
{
    const rk_aiq_customAwb_single_hw_cfg_t  *awbHwConfig = &customAwbSingelRes->awbHwConfig;
    for(int i=0;i<RK_AIQ_AWB_XY_TYPE_MAX_V201;i++){
        awbHwConfigFull->blkWeightEnable[i]   = awbHwConfig->blkWeightEnable;
    }
#ifdef ISP_HW_V30
    awbHwConfigFull->multiwindow_en   = awbHwConfig->multiwindow_en;
    memcpy(awbHwConfigFull->multiwindow, awbHwConfig->multiwindow,sizeof(awbHwConfig->multiwindow));
#endif
    memcpy(awbHwConfigFull->windowSet,awbHwConfig->windowSet,sizeof(awbHwConfig->windowSet));
    memcpy(awbHwConfigFull->blkWeight, awbHwConfig->blkWeight,sizeof(awbHwConfig->blkWeight));
    *awb_gain_algo = &customAwbSingelRes->awb_gain_algo;
}
static XCamReturn _customGruopAwbRes2rkAwbRes(rk_aiq_singlecam_3a_result_t ** rk_aiq_singlecam_3a_result , int camera_num,
                          rk_aiq_rkAwb_config_t *rkCfg, rk_aiq_customeAwb_results_t* customAwbProcRes)
{
    LOG1_AWB_SUBM(0xff, "%s ENTER", __func__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_isp_awb_meas_cfg_v3x_t *awbHwConfigFull = &rkCfg->awbHwConfig;
    const rk_aiq_wb_gain_t* _awbGainParams = &customAwbProcRes->awb_gain_algo;//update by first camera
    //awb_window_check(rkCfg,customAwbProcRes->awbHwConfig.windowSet);
    _customAwbHw2rkAwbHwCfg(customAwbProcRes,awbHwConfigFull);//update by first camera
    awb_window_check(rkCfg,awbHwConfigFull->windowSet);
    RkAiqAlgoProcResAwbShared_t* xCamAwbProcRes;
    RkAiqAlgoProcResAwbShared_t* procResPara;
    rk_aiq_customeAwb_single_results_t* customAwbProcRes2 = customAwbProcRes->next;
    for(int i=0; i<camera_num; i++){
        xCamAwbProcRes = &rk_aiq_singlecam_3a_result[i]->awb._awbProcRes;
        procResPara = nullptr;
        if (xCamAwbProcRes) {
            procResPara = (RkAiqAlgoProcResAwbShared_t*)xCamAwbProcRes;
            if(procResPara==nullptr){
                LOGE_AWBGROUP("_awbProcRes is null for %dth camera",i);
                return(XCAM_RETURN_ERROR_FAILED);
            }
        }else {
           LOGE_AWBGROUP("_awbProcRes is null for %dth camera",i);
           return(XCAM_RETURN_ERROR_FAILED);
        }
        if(i>0 && customAwbProcRes2 != nullptr){
            _customGruopAwbHw2rkAwbHwCfg(customAwbProcRes2,awbHwConfigFull,&_awbGainParams);//update by each camera
            customAwbProcRes2 = customAwbProcRes2->next;
            awb_window_check(rkCfg,awbHwConfigFull->windowSet);
        }
        memcpy(rk_aiq_singlecam_3a_result[i]->awb._awbCfgV3x,awbHwConfigFull,sizeof(rk_aiq_isp_awb_meas_cfg_v3x_t));
        //memcpy(&procResPara->awb_hw1_para,awbHwConfigFull,sizeof(rk_aiq_isp_awb_meas_cfg_v3x_t));
        memcpy(rk_aiq_singlecam_3a_result[i]->awb._awbGainParams,_awbGainParams,sizeof(rk_aiq_wb_gain_t));
        memcpy(&procResPara->awb_gain_algo, _awbGainParams, sizeof(rk_aiq_wb_gain_t));
        procResPara->awb_smooth_factor = customAwbProcRes->awb_smooth_factor;
        procResPara->awbConverged = customAwbProcRes->IsConverged;
        //procResPara->awb_cfg_update = true;
        //procResPara->awb_gain_update= true;
        //WriteDataForThirdParty(*rk_aiq_singlecam_3a_result[i]->awb._awbCfgV3x,XCORE_LOG_LEVEL_LOW1+1,i);
    }
    LOG1_AWB_SUBM(0xff, "%s EXIT", __func__);
    return(ret);

}

static void WriteMeasureResult(rk_aiq_awb_stat_res_v201_t &awb_measure_result, int log_level ,int camera_id = 0)
{

    if(log_level <XCORE_LOG_LEVEL_LOW1) {
        return;
    }
    FILE * fid;
    char fName[100];
    sprintf(fName, "./tmp/MeasureResultByCustom_camera%d.txt",camera_id);
    fid = fopen(fName, "w");
    if (fid == NULL) {
        return;
    }
    fprintf(fid, "WpNoHist:       ");
    for (int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        fprintf(fid, "%6d,", awb_measure_result.WpNoHist[i]);
    }
    fprintf(fid, "\n");
    {
        for (int i = 0; i < RK_AIQ_AWB_MAX_WHITEREGIONS_NUM; i++)
        {
            fprintf(fid, " illuminant %dth:\n", i);
#ifdef AWB_WPNUM2_EN
            fprintf(fid, "WPNUM2[%d]=    %7d\n", i, awb_measure_result.WpNo2[i]);
#endif
            fprintf(fid, "SUM_RGain_NOR[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[0].RgainValue);
            fprintf(fid, "SUM_BGain_NOR[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[0].BgainValue);
            fprintf(fid, "WPNUM_NOR[%d]=     %7lld\n", i, awb_measure_result.light[i].xYType[0].WpNo);
            fprintf(fid, "SUM_RGain_BIG[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[1].RgainValue);
            fprintf(fid, "SUM_BGain_BIG[%d]=%7lld\n", i, awb_measure_result.light[i].xYType[1].BgainValue);
            fprintf(fid, "WPNUM_BIG[%d]=     %7lld\n", i, awb_measure_result.light[i].xYType[1].WpNo);


        }
        fprintf(fid, "blockresult[15][15]:\n");
        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_VERHOR * RK_AIQ_AWB_GRID_NUM_VERHOR; i++)
        {

            fprintf(fid, "blk[%2d,%2d]:     blk_sum_r=%7lld,        blk_sum_g=%7lld,        blk_sum_b=%7lld,   blk_sum_num=%7lld\n"
                    , i / 15, i % 15, awb_measure_result.blockResult[i].Rvalue,
                    awb_measure_result.blockResult[i].Gvalue, awb_measure_result.blockResult[i].Bvalue,
                    awb_measure_result.blockResult[i].WpNo);

        }
        fprintf(fid, "\n");
    }
    //new measurement
    {
#ifdef AWB_MULTI_WINDOW_EN
        fprintf(fid, " multiwindowLightResult:\n");
        for (int i = 0; i < 4; i++)
        {
            fprintf(fid, " illuminant %dth:\n", i);
            fprintf(fid, "SUM_RGain_NOR[%d]=%7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[0].RgainValue);
            fprintf(fid, "SUM_BGain_NOR[%d]=%7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[0].BgainValue);
            fprintf(fid, "WPNUM_NOR[%d]=    %7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[0].WpNo);
            fprintf(fid, "SUM_RGain_BIG[%d]=%7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[1].RgainValue);
            fprintf(fid, "SUM_BGain_BIG[%d]=%7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[1].BgainValue);
            fprintf(fid, "WPNUM_BIG[%d]=    %7lld\n", i, awb_measure_result.multiwindowLightResult[i].xYType[1].WpNo);

        }
#endif
#ifdef AWB_EXCWP_RANGE_STAT_EN
        fprintf(fid, " excWpRangeResult:\n");
        for (int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++)
        {
            fprintf(fid, " %d:\n", i);
            fprintf(fid, "    SUM_RGain[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].RgainValue);
            fprintf(fid, "    SUM_RGain[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].BgainValue);
            fprintf(fid, "   WPNUM_NOR[%d]=%7lld\n", i, awb_measure_result.excWpRangeResult[i].WpNo);
        }
#endif
    }
    fclose(fid);
}


static void WriteDataForThirdParty(const rk_aiq_isp_awb_meas_cfg_v3x_t &wpDetectPara,int log_level ,int camera_id=0)
{
    if(log_level < XCORE_LOG_LEVEL_LOW1){
        return;
    }
    char fName[100];
    sprintf(fName, "./tmp/awbHwConfigByCustom_%d.txt",camera_id);
    FILE* fp = fopen(fName, "wb");
    if(fp != NULL)
    {

        fprintf(fp, "awbHwConfig->awbEnable             =    %d;//rawawb enable\n", wpDetectPara.awbEnable);
        fprintf(fp, "awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//xy detect  enable for all light\n", wpDetectPara.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//xy detect  enable for all light\n", wpDetectPara.xyDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.uvDetectionEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]          =    %d;//uv detect  enable for all  light\n", wpDetectPara.threeDyuvEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]    =    %d;\n", wpDetectPara.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]    =    %d;\n", wpDetectPara.blkWeightEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->lscBypEnable    =    %d;\n", wpDetectPara.lscBypEnable);

        fprintf(fp, "awbHwConfig->blkStatisticsEnable    =    %d;\n", wpDetectPara.blkStatisticsEnable);
        fprintf(fp, "awbHwConfig->blkMeasureMode     =    (rk_aiq_awb_blk_stat_mode_v201_t)%d;\n", wpDetectPara.blkMeasureMode);
        fprintf(fp, "awbHwConfig->xyRangeTypeForBlkStatistics     =    (rk_aiq_awb_xy_type_v201_t)%d;\n", wpDetectPara.xyRangeTypeForBlkStatistics);
        fprintf(fp, "awbHwConfig->illIdxForBlkStatistics     =    (rk_aiq_awb_blk_stat_realwp_ill_e)%d;\n", wpDetectPara.illIdxForBlkStatistics);
        fprintf(fp, "awbHwConfig->blkStatisticsWithLumaWeightEn  =    %d;\n", wpDetectPara.blkStatisticsWithLumaWeightEn);

        fprintf(fp, "awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =    %d;\n", wpDetectPara.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);
        fprintf(fp, "awbHwConfig->wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =    %d;\n", wpDetectPara.wpDiffWeiEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);
        fprintf(fp, "awbHwConfig->xyRangeTypeForWpHist    =    (rk_aiq_awb_xy_type_v201_t)%d;\n", wpDetectPara.xyRangeTypeForWpHist);

        fprintf(fp, "awbHwConfig->threeDyuvIllu[0]       =    %d;\n", wpDetectPara.threeDyuvIllu[0]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[1]       =    %d;\n", wpDetectPara.threeDyuvIllu[1]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[2]       =    %d;\n", wpDetectPara.threeDyuvIllu[2]);
        fprintf(fp, "awbHwConfig->threeDyuvIllu[3]       =    %d;\n", wpDetectPara.threeDyuvIllu[3]);



        fprintf(fp, "awbHwConfig->lightNum      =    %d; //measurment light num ,0~7\n", wpDetectPara.lightNum                                );
        fprintf(fp, "awbHwConfig->windowSet[0]         =    %d;//crop h_offs mod 2 == 0\n", wpDetectPara.windowSet[0]                                );
        fprintf(fp, "awbHwConfig->windowSet[1]          =    %d;//crop v_offs mod 2 == 0\n", wpDetectPara.windowSet[1]                                );
        fprintf(fp, "awbHwConfig->windowSet[2]         =    %d;//crop h_size,8x8:mod 8==0,4x4:mod 4==0\n", wpDetectPara.windowSet[2]    );
        fprintf(fp, "awbHwConfig->windowSet[3]         =    %d;//crop v_size,8x8:mod 8==0,4x4:mod 4==0\n", wpDetectPara.windowSet[3]    );

        /*if (wpDetectPara.windowSet[2]>2560||wpDetectPara.windowSet[3]>2560)
        Ds_rate = 8;*/
        int tmp;
        switch (wpDetectPara.dsMode)
        {
        case RK_AIQ_AWB_DS_4X4:
            tmp = 0;
            break;
        default:
            tmp = 1;
        }
        fprintf(fp, "awbHwConfig->dsMode      =    (rk_aiq_down_scale_mode_t)%d;//downscale,1:8x8,0:4x4\n",    tmp                            );
        fprintf(fp, "awbHwConfig->maxR          =    %d;\n",    wpDetectPara.maxR        );
        fprintf(fp, "awbHwConfig->maxG          =    %d;\n",    wpDetectPara.maxG        );
        fprintf(fp, "awbHwConfig->maxB          =    %d;\n",    wpDetectPara.maxB        );
        fprintf(fp, "awbHwConfig->maxY          =    %d;\n",    wpDetectPara.maxY        );
        fprintf(fp, "awbHwConfig->minR          =    %d;\n",    wpDetectPara.minR        );
        fprintf(fp, "awbHwConfig->minG          =    %d;\n",    wpDetectPara.minG        );
        fprintf(fp, "awbHwConfig->minB          =    %d;\n",    wpDetectPara.minB        );
        fprintf(fp, "awbHwConfig->minY          =    %d;\n",    wpDetectPara.minY        );
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[0]    =    %d;//uv detect white point region vertex and inverse slope\n", wpDetectPara.uvRange_param[0].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[0].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[0].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[0].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[0].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[1].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[1].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[1].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[1].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[2].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[2].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[2].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[2].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[3].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[3].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[3].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[3].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[4].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[4].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[4].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[4].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[4].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[4].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[4].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[4].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[4].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[4].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[4].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[4].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[4].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[5].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[5].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[5].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[5].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[5].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[5].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[5].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[5].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[5].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[5].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[5].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[5].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[5].slope_inv[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pu_region[0]    =    %d;\n", wpDetectPara.uvRange_param[6].pu_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pv_region[0]    =    %d;\n", wpDetectPara.uvRange_param[6].pv_region[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pu_region[1]    =    %d;\n", wpDetectPara.uvRange_param[6].pu_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pv_region[1]    =    %d;\n", wpDetectPara.uvRange_param[6].pv_region[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pu_region[2]    =    %d;\n", wpDetectPara.uvRange_param[6].pu_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pv_region[2]    =    %d;\n", wpDetectPara.uvRange_param[6].pv_region[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pu_region[3]    =    %d;\n", wpDetectPara.uvRange_param[6].pu_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].pv_region[3]    =    %d;\n", wpDetectPara.uvRange_param[6].pv_region[3]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].slope_inv[0]      =    %d;\n", wpDetectPara.uvRange_param[6].slope_inv[0]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].slope_inv[1]      =    %d;\n", wpDetectPara.uvRange_param[6].slope_inv[1]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].slope_inv[2]      =    %d;\n", wpDetectPara.uvRange_param[6].slope_inv[2]);
        fprintf(fp, "awbHwConfig->uvRange_param[6].slope_inv[3]      =    %d;\n", wpDetectPara.uvRange_param[6].slope_inv[3]);

        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[0]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[0]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[1]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[1]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[2]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[2]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[3]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[3]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[4]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[4]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[5]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[5]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[6]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[6]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[7]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[7]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[8]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[8]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[9]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[9]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[10]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[10]);
        fprintf(fp, "awbHwConfig->icrgb2RYuv_matrix[11]  =        %d;\n", wpDetectPara.icrgb2RYuv_matrix[11]);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[0].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[0].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[1].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[1].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcurve_u[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[0]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[1]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[2]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[3]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[4]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].thcure_th[5]  =        %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].lineP1[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[0]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[1]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].vP1P2[2]  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[2].disP1P2  =    %d;\n", wpDetectPara.ic3Dyuv2Range_param[2].disP1P2);

        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[0]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[1]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[2]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[3]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[4]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcurve_u[5]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcurve_u[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[0]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[1]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[2]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[3]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[3]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[4]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[4]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].thcure_th[5]  =       %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].thcure_th[5]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[0]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[1]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].lineP1[2]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].lineP1[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[0]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[0]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[1]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[1]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].vP1P2[2]  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].vP1P2[2]);
        fprintf(fp, "awbHwConfig->ic3Dyuv2Range_param[3].disP1P2  =   %d;\n", wpDetectPara.ic3Dyuv2Range_param[3].disP1P2);
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[0]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[0] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[1]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[1] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.pseudoLuminanceWeight[2]  =    %d;\n", wpDetectPara.rgb2xy_param.pseudoLuminanceWeight[2] ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[0]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[0]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[1]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[1]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[2]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[2]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[3]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[3]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[4]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[4]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->rgb2xy_param.rotationMat[5]  =    %d;\n", wpDetectPara.rgb2xy_param.rotationMat[5]            ); //rgb2xy parpmeter
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeX[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeX[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeY[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[0].NorrangeY[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeX[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeX[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeY[0]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[0].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[0].SperangeY[1]            ); //xy detect xy region
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[1].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[1].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[1].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[2].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[2].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[2].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[3].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[3].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[3].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[4].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[4].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[4].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[4].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[4].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[4].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[4].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[4].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[4].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[5].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[5].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[5].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[5].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[5].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[5].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[5].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[5].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[5].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].NorrangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[6].NorrangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].NorrangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[6].NorrangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].NorrangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[6].NorrangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].NorrangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[6].NorrangeY[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].SperangeX[0]  =    %d;\n", wpDetectPara.xyRange_param[6].SperangeX[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].SperangeX[1]  =    %d;\n", wpDetectPara.xyRange_param[6].SperangeX[1]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].SperangeY[0]  =    %d;\n", wpDetectPara.xyRange_param[6].SperangeY[0]            );
        fprintf(fp, "awbHwConfig->xyRange_param[6].SperangeY[1]  =    %d;\n", wpDetectPara.xyRange_param[6].SperangeY[1]            );
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_r  =    %d;\n", wpDetectPara.pre_wbgain_inv_r);
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_g  =    %d;\n", wpDetectPara.pre_wbgain_inv_g);
        fprintf(fp, "awbHwConfig->pre_wbgain_inv_b  =    %d;\n", wpDetectPara.pre_wbgain_inv_b);
        //add new
#if defined(ISP_HW_V30)
        fprintf(fp, "awbHwConfig->multiwindow_en  =    %d;\n", wpDetectPara.multiwindow_en); // multiwindow auto white balance measure enable
        fprintf(fp, "awbHwConfig->multiwindow[0][0]  =    %d;\n", wpDetectPara.multiwindow[0][0]); // multiwindow0 row start
        fprintf(fp, "awbHwConfig->multiwindow[0][1]  =    %d;\n", wpDetectPara.multiwindow[0][1]); // multiwindow0 column start
        fprintf(fp, "awbHwConfig->multiwindow[0][2]  =    %d;\n", wpDetectPara.multiwindow[0][2]); // multiwindow0 row size
        fprintf(fp, "awbHwConfig->multiwindow[0][3]  =    %d;\n", wpDetectPara.multiwindow[0][3]); // multiwindow0 column size
        fprintf(fp, "awbHwConfig->multiwindow[1][0]  =    %d;\n", wpDetectPara.multiwindow[1][0]); // multiwindow1 row start
        fprintf(fp, "awbHwConfig->multiwindow[1][1]  =    %d;\n", wpDetectPara.multiwindow[1][1]); // multiwindow1 column start
        fprintf(fp, "awbHwConfig->multiwindow[1][2]  =    %d;\n", wpDetectPara.multiwindow[1][2]); // multiwindow1 row size
        fprintf(fp, "awbHwConfig->multiwindow[1][3]  =    %d;\n", wpDetectPara.multiwindow[1][3]); // multiwindow1 column size
        fprintf(fp, "awbHwConfig->multiwindow[2][0]  =    %d;\n", wpDetectPara.multiwindow[2][0]); // multiwindow2 row start
        fprintf(fp, "awbHwConfig->multiwindow[2][1]  =    %d;\n", wpDetectPara.multiwindow[2][1]); // multiwindow2 column start
        fprintf(fp, "awbHwConfig->multiwindow[2][2]  =    %d;\n", wpDetectPara.multiwindow[2][2]); // multiwindow2 row size
        fprintf(fp, "awbHwConfig->multiwindow[2][3]  =    %d;\n", wpDetectPara.multiwindow[2][3]); // multiwindow2 column size
        fprintf(fp, "awbHwConfig->multiwindow[3][0]  =    %d;\n", wpDetectPara.multiwindow[3][0]); // multiwindow3 row start
        fprintf(fp, "awbHwConfig->multiwindow[3][1]  =    %d;\n", wpDetectPara.multiwindow[3][1]); // multiwindow3 column start
        fprintf(fp, "awbHwConfig->multiwindow[3][2]  =    %d;\n", wpDetectPara.multiwindow[3][2]); // multiwindow3 row size
        fprintf(fp, "awbHwConfig->multiwindow[3][3]  =    %d;\n", wpDetectPara.multiwindow[3][3]); // multiwindow3 column size
#endif
        fprintf(fp, "awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[0].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[0].measureEnable); //white points region0 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[0].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[0].domain       ); //white points region0 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[0].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[0].xu[0]        ); //left x/u boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[0].xu[1]        ); //right x/u boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[0].yv[0]        ); //up y/v boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[0].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[0].yv[1]        ); //down y/v boundary of white points exclusion region0
        fprintf(fp, "awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[1].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[1].measureEnable); //white points region1 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[1].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[1].domain       ); //white points region1 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[1].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[1].xu[0]        ); //left x/u boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[1].xu[1]        ); //right x/u boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[1].yv[0]        ); //up y/v boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[1].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[1].yv[1]        ); //down y/v boundary of white points exclusion region1
        fprintf(fp, "awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[2].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[2].measureEnable); //white points region2 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[2].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[2].domain       ); //white points region2 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[2].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[2].xu[0]        ); //left x/u boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[2].xu[1]        ); //right x/u boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[2].yv[0]        ); //up y/v boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[2].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[2].yv[1]        ); //down y/v boundary of white points exclusion region2
        fprintf(fp, "awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[3].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[3].measureEnable); //white points region3 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[3].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[3].domain       ); //white points region3 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[3].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[3].xu[0]        ); //left x/u boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[3].xu[1]        ); //right x/u boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[3].yv[0]        ); //up y/v boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[3].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[3].yv[1]        ); //down y/v boundary of white points exclusion region3
        fprintf(fp, "awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[4].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[4].measureEnable); //white points region4 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[4].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[4].domain       ); //white points region4 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[4].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[4].xu[0]        ); //left x/u boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[4].xu[1]        ); //right x/u boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[4].yv[0]        ); //up y/v boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[4].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[4].yv[1]        ); //down y/v boundary of white points exclusion region4
        fprintf(fp, "awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[5].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[5].measureEnable); //white points region5 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[5].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[5].domain       ); //white points region5 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[5].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[5].xu[0]        ); //left x/u boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[5].xu[1]        ); //right x/u boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[5].yv[0]        ); //up y/v boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[5].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[5].yv[1]        ); //down y/v boundary of white points exclusion region5
        fprintf(fp, "awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]   =     %d;\n", wpDetectPara.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_NORMAL_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]   =     %d;\n", wpDetectPara.excludeWpRange[6].excludeEnable[RK_AIQ_AWB_XY_TYPE_BIG_V201]);  //white points region0 exclusion enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].measureEnable   =     %d;\n", wpDetectPara.excludeWpRange[6].measureEnable);  //white points region6 measure enable
        fprintf(fp, "awbHwConfig->excludeWpRange[6].domain          =     (rk_aiq_awb_exc_range_domain_t)%d;\n", wpDetectPara.excludeWpRange[6].domain       ); //white points region6 exclusion in domain,1 uv domain,2 xy domain.
        fprintf(fp, "awbHwConfig->excludeWpRange[6].xu[0]           =     %d;\n", wpDetectPara.excludeWpRange[6].xu[0]        ); //left x/u boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].xu[1]           =     %d;\n", wpDetectPara.excludeWpRange[6].xu[1]        ); //right x/u boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].yv[0]           =     %d;\n", wpDetectPara.excludeWpRange[6].yv[0]        ); //up y/v boundary of white points exclusion region6
        fprintf(fp, "awbHwConfig->excludeWpRange[6].yv[1]           =     %d;\n", wpDetectPara.excludeWpRange[6].yv[1]        ); //down y/v boundary of white points exclusion region6

        for (int i = 0; i < RK_AIQ_AWBWP_WEIGHT_CURVE_DOT_NUM; i++) {
            fprintf(fp, "awbHwConfig->wpDiffwei_y[%d]  =%d;\n", i, wpDetectPara.wpDiffwei_y[i]);
        }
        for (int i = 0; i < RK_AIQ_AWBWP_WEIGHT_CURVE_DOT_NUM; i++) {
            fprintf(fp, "awbHwConfig->wpDiffwei_w[%d]  =%d;\n", i, wpDetectPara.wpDiffwei_w[i]);
        }
        for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++) {
            fprintf(fp, "awbHwConfig->blkWeight[%d]  =%d;\n", i, wpDetectPara.blkWeight[i]);
        }
        fprintf(fp, "awbHwConfig->blk_rtdw_measure_en  =    %d;\n", wpDetectPara.blk_rtdw_measure_en );
        fclose(fp);

    }
}


RKAIQ_END_DECLARE
