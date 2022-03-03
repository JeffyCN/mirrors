/*
 * rk_aiq_algo_camgroup_types.h
 *
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

#ifndef _RK_AIQ_ALGO_CAMGROUP_TYPES_H_
#define _RK_AIQ_ALGO_CAMGROUP_TYPES_H_


#include "rk_aiq_algo_types.h"
#include "RkAiqCalibDbV2Helper.h"

typedef struct rk_aiq_singlecam_3a_result_s {
    uint8_t _camId;
    uint32_t _frameId;

    // ae params
    struct {
        RKAiqAecExpInfo_t* exp_tbl;
        int* exp_tbl_size;
        rk_aiq_ae_meas_params_t* _aeMeasParams;
        rk_aiq_hist_meas_params_t* _aeHistMeasParams;
        XCamVideoBuffer* _aecStats;
        XCamVideoBuffer* _aeProcRes;
        XCamVideoBuffer* _aePreRes;
        RKAiqAecExpInfo_t _effAecExpInfo;
        bool _bEffAecExpValid;
    } aec;
    // awb params
    struct {
        rk_aiq_wb_gain_t* _awbGainParams;
        union {
            rk_aiq_awb_stat_cfg_v200_t*  _awbCfg;
            rk_aiq_awb_stat_cfg_v201_t*  _awbCfgV201;
            rk_aiq_isp_awb_meas_cfg_v3x_t* _awbCfgV3x;
        };
        XCamVideoBuffer* _awbStats;
        XCamVideoBuffer* _awbProcRes;
    } awb;
    rk_aiq_lsc_cfg_t* _lscConfig;
    AdpccProcResult_t* _dpccConfig;
    rk_aiq_ccm_cfg_t* _ccmCfg;
    rk_aiq_lut3d_cfg_t* _lut3dCfg;
    RkAiqAdehazeProcResult_t* _adehazeConfig;
    AgammaProcRes_t* _agammaConfig;
	RkAiqAdrcProcResult_t* _adrcConfig;
	RkAiqAmergeProcResult_t* _amergeConfig;
    rk_aiq_isp_blc_v21_t * _blcConfig;
    struct {
        union {
            RK_YNR_Fix_V3_t*  _aynr_procRes_v3;
            RK_YNR_Fix_V2_t*  _aynr_procRes_v2;
        };
    } aynr;

    struct {
        union {
            RK_CNR_Fix_V2_t*  _acnr_procRes_v2;
            RK_CNR_Fix_V1_t*  _acnr_procRes_v1;
        };
    } acnr;

    struct {
        union {
            RK_Bayernr_Fix_V2_t*  _abayernr_procRes_v1;
            RK_Bayer2dnr_Fix_V2_t*  _abayer2dnr_procRes_v2;
        };
    } abayernr;

    struct {
        union {
            RK_Bayertnr_Fix_V2_t*  _abayertnr_procRes_v2;
        };
    } abayertnr;

    struct {
        union {
            RK_SHARP_Fix_V3_t*  _asharp_procRes_v3;
            RK_SHARP_Fix_V4_t*  _asharp_procRes_v4;
        };
    } asharp;

    struct {
        union {
            RK_GAIN_Fix_V2_t*  _again_procRes_v2;
        };
    } again;
} rk_aiq_singlecam_3a_result_t;

// for create_contex
typedef struct _AlgoCtxInstanceCfgCamGroup {
    AlgoCtxInstanceCfg cfg_com;
    // single cam calib
    const CamCalibDbV2Context_t* s_calibv2;
    CamCalibDbCamgroup_t* pCamgroupCalib;
    int camIdArray[6];
    int camIdArrayLen;
} AlgoCtxInstanceCfgCamGroup;

// camgroup common params
typedef struct _RkAiqAlgoComCamGroup {
    RkAiqAlgoCom com;
} RkAiqAlgoComCamGroup;

// for prepare params
typedef struct _RkAiqAlgoCamGroupPrepare {
    RkAiqAlgoComCamGroup gcom;
    struct {
        float        LinePeriodsPerField;
        float        PixelClockFreqMHZ;
        float        PixelPeriodsPerLine;
        rk_aiq_sensor_nr_switch_t nr_switch;
    } aec;
    int camIdArray[6];
    int camIdArrayLen;
    // single cam calib
    const CamCalibDbV2Context_t* s_calibv2;
    CamCalibDbCamgroup_t* pCamgroupCalib;
} RkAiqAlgoCamGroupPrepare;

// for processing output params
typedef struct _RkAiqAlgoCamGroupProcOut {
    RkAiqAlgoResCom gcom;
    rk_aiq_singlecam_3a_result_t** camgroupParmasArray;
    int arraySize;
} RkAiqAlgoCamGroupProcOut;

// for processing input params
typedef struct _RkAiqAlgoCamGroupProcIn {
    RkAiqAlgoComCamGroup gcom;
    rk_aiq_singlecam_3a_result_t** camgroupParmasArray;
    int arraySize;
    bool _gray_mode;
    int working_mode;
    bool _is_bw_sensor;
} RkAiqAlgoCamGroupProcIn;

#endif
