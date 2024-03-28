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
#include "rk_aiq_types_priv.h"

#define IS_UPDATE_MEM(result_ptr, result_offset) \
    *((bool*)((char*)(result_ptr) - (result_offset)))

typedef struct rk_aiq_singlecam_3a_result_s {
    uint8_t _camId;
    uint32_t _frameId;
    int16_t hdrIso;
    // ae params
    struct {
        RKAiqAecExpInfo_t* exp_tbl;
        int* exp_tbl_size;
        RKAiqExpI2cParam_t* exp_i2c_params;
        rk_aiq_ae_meas_params_t* _aeMeasParams;
        rk_aiq_hist_meas_params_t* _aeHistMeasParams;
        XCamVideoBuffer* _aecStats;
        RkAiqAlgoProcResAeShared_t _aeProcRes;
        XCamVideoBuffer* _aePreRes;
        RKAiqAecExpInfo_t _effAecExpInfo;
        bool _bEffAecExpValid;
        RkAiqSetStatsCfg stats_cfg_to_trans;
    } aec;
    // awb params
    struct {
        union {
            rk_aiq_wb_gain_t* _awbGainParams;
            rk_aiq_wb_gain_v32_t* _awbGainV32Params;
        };
        union {
            rk_aiq_awb_stat_cfg_v200_t*  _awbCfg;
            rk_aiq_awb_stat_cfg_v201_t*  _awbCfgV201;
            rk_aiq_awb_stat_cfg_v32_t*  _awbCfgV32;
            rk_aiq_isp_awb_meas_cfg_v3x_t* _awbCfgV3x;
        };
        XCamVideoBuffer* _awbStats;
        RkAiqAlgoProcResAwbShared_t _awbProcRes;
    } awb;
    rk_aiq_lsc_cfg_t* _lscConfig;
    struct {
        union {
            rk_aiq_ccm_cfg_t*  _ccmCfg;
            rk_aiq_ccm_cfg_v2_t*  _ccmCfg_v2;
        };
    } accm;
    rk_aiq_lut3d_cfg_t* _lut3dCfg;
#if USE_NEWSTRUCT
    rk_aiq_isp_gamma_params_t* gamma;
    rk_aiq_isp_drc_params_t* drc;
    rk_aiq_isp_dehaze_params_t* dehaze;
    rk_aiq_isp_dpcc_params_t* dpc;
#else
    AgammaProcRes_t* _agammaConfig;
    RkAiqAdehazeProcResult_t* _adehazeConfig;
    RkAiqAdrcProcResult_t* _adrcConfig;
    AdpccProcResult_t* _dpccConfig;
#endif
    RkAiqAmergeProcResult_t* _amergeConfig;
    RkAiqArgbirProcResult_t* _aRgbirConfig;

    struct {
        union {
            rk_aiq_isp_blc_v21_t * _blcConfig;
            rk_aiq_isp_blc_v32_t * _blcConfig_v32;
            rk_aiq_isp_blc_params_t *blc;
        };
    } ablc;


#if USE_NEWSTRUCT
    rk_aiq_isp_ynr_params_t *ynr;
#else
    struct aynr_procRes_V3_t {
        RK_YNR_Fix_V3_t*  _stFix;
        float  _sigma[YNR_V3_ISO_CURVE_POINT_NUM];
    };

    struct {
        union {
            RK_YNR_Fix_V24_t* _aynr_procRes_v24;
            RK_YNR_Fix_V22_t*  _aynr_procRes_v22;
            aynr_procRes_V3_t  _aynr_procRes_v3;
            RK_YNR_Fix_V2_t*  _aynr_procRes_v2;
        };
    } aynr;
#endif

    struct {
        union {
            float _aynr_sigma_v24[YNR_V24_ISO_CURVE_POINT_NUM];
            float _aynr_sigma_v22[YNR_V22_ISO_CURVE_POINT_NUM];
            float _aynr_sigma_v3[YNR_V3_ISO_CURVE_POINT_NUM];
        };
    } aynr_sigma;

#if USE_NEWSTRUCT
    rk_aiq_isp_cnr_params_t *cnr;
#else
    struct {
        union {
            RK_CNR_Fix_V30_t*  _acnr_procRes_v30;
            RK_CNR_Fix_V2_t*  _acnr_procRes_v2;
            RK_CNR_Fix_V1_t*  _acnr_procRes_v1;
            RK_CNR_Fix_V31_t*  _acnr_procRes_v31;
        };
    } acnr;
#endif

    struct {
        union {
            RK_Bayernr_Fix_V2_t*  _abayernr_procRes_v1;
            RK_Bayer2dnr_Fix_V2_t*  _abayer2dnr_procRes_v2;
            RK_Bayer2dnr_Fix_V23_t*  _abayer2dnr_procRes_v23;
        };
    } abayernr;

#if USE_NEWSTRUCT
    rk_aiq_isp_btnr_params_t *btnr;
#else
    struct {
        union {
            RK_Bayertnr_Fix_V2_t*  _abayertnr_procRes_v2;
            RK_Bayertnr_Fix_V23_t*  _abayertnr_procRes_v23;
            RK_Bayertnr_Fix_V30_t*  _abayertnr_procRes_v30;
        };

        XCamVideoBuffer* _tnr_stats;
    } abayertnr;
#endif

#if USE_NEWSTRUCT
    rk_aiq_isp_sharp_params_t *sharp;
#else
    struct {
        union {
            RK_SHARP_Fix_V3_t*  _asharp_procRes_v3;
            RK_SHARP_Fix_V4_t*  _asharp_procRes_v4;
            RK_SHARP_Fix_V33_t*  _asharp_procRes_v33;
            RK_SHARP_Fix_V34_t* _asharp_procRes_v34;
        };
    } asharp;
#endif

    struct {
        union {
            RK_GAIN_Fix_V2_t*  _again_procRes_v2;
        };
    } again;

    struct {
        union {
            RK_YUVME_Fix_V1_t*  _ayuvme_procRes_v1;
        };
    } ayuvme;

    //otp info
    struct rkmodule_awb_inf _otp_awb;
    struct rkmodule_lsc_inf *_otp_lsc;
} rk_aiq_singlecam_3a_result_t;

// for create_contex
typedef struct _AlgoCtxInstanceCfgCamGroup {
    AlgoCtxInstanceCfg cfg_com;
    // single cam calib
    const CamCalibDbV2Context_t* s_calibv2;
    CamCalibDbCamgroup_t* pCamgroupCalib;
    int camIdArray[RK_AIQ_CAM_GROUP_MAX_CAMS];
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
        rk_aiq_sensor_dcg_ratio_t dcg_ratio;
        int          compr_bit;
    } aec;
    int camIdArray[RK_AIQ_CAM_GROUP_MAX_CAMS];
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
    AblcProc_V32_t stAblcV32_proc_res;
    int arraySize;
    bool _gray_mode;
    int working_mode;
    bool _is_bw_sensor;
    size_t _offset_is_update;
    bool attribUpdated;
} RkAiqAlgoCamGroupProcIn;

#endif
