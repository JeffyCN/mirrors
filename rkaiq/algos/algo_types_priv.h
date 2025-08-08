/*
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

#ifndef _ALGO_TYPES_PRIV_H_
#define _ALGO_TYPES_PRIV_H_

#include "rk_aiq_algo_des.h"
#if USE_NEWSTRUCT
//#include "include/algos/adehaze/rk_aiq_types_adehaze_stat.h"
#include "newStruct/demosaic/include/demosaic_algo_api.h"
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"
#include "newStruct/gamma/include/gamma_algo_api.h"
#include "newStruct/ynr/include/ynr_algo_api.h"
#include "newStruct/sharp/include/sharp_algo_api.h"
#include "newStruct/cnr/include/cnr_algo_api.h"
#include "newStruct/drc/include/drc_algo_api.h"
#include "newStruct/blc/include/blc_algo_api.h"
#include "newStruct/dpc/include/dpc_algo_api.h"
#include "newStruct/gic/include/gic_algo_api.h"
#include "newStruct/cac/include/cac_algo_api.h"
// #include "newStruct/cac/include/lut_buffer.h"
#include "newStruct/csm/include/csm_algo_api.h"
#include "newStruct/lsc/include/lsc_algo_api.h"
#include "newStruct/merge/include/merge_algo_api.h"
#include "isp/rk_aiq_stats_aeStats25.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "newStruct/cgc/include/cgc_algo_api.h"
#include "newStruct/cp/include/cp_algo_api.h"
#include "newStruct/ie/include/ie_algo_api.h"
#include "newStruct/gain/include/gain_algo_api.h"
#include "newStruct/ccm/include/ccm_algo_api.h"
#ifdef ISP_HW_V39
#include "newStruct/3dlut/include/3dlut_algo_api.h"
#include "newStruct/dehaze/include/dehaze_algo_api.h"
#include "newStruct/ldc/include/ldc_algo_api.h"
#if RKAIQ_HAVE_YUVME
#include "newStruct/yme/include/yme_algo_api.h"
#endif
#endif
#ifdef ISP_HW_V33
#include "newStruct/hsv/include/hsv_algo_api.h"
#include "newStruct/ldc/include/ldc_algo_api.h"
#endif
#ifdef ISP_HW_V35
#include "newStruct/hsv/include/hsv_algo_api.h"
#include "newStruct/ldc/include/ldc_algo_api.h"
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
#include "newStruct/rgbir/include/rgbir_algo_api.h"
#endif
#endif

#include "rk_aiq_types.h"
#include "xcore/base/xcam_buffer.h"
#include "algos/ae/rk_aiq_types_ae_algo_int.h"
#include "algos/afd/rk_aiq_types_afd_algo_int.h"
#include "algos/amtd/rk_aiq_types_amtd_algo_int.h"
#include "awb/rk_aiq_types_awb_algo_int.h"

#include "newStruct/aibnr/include/aibnr_algo_api.h"
#include "newStruct/airms/include/airms_algo_api.h"
#include "newStruct/aiynr/include/aiynr_algo_api.h"
#include "c_base/aiq_list.h"


typedef struct Aynr_ProcResult_V3_s Aynr_ProcResult_V3_t;
typedef struct Aynr_ProcResult_V22_s Aynr_ProcResult_V22_t;
typedef struct Aynr_ProcResult_V24_s Aynr_ProcResult_V24_t;
typedef struct AblcProc_V32_s AblcProc_V32_t;
typedef struct AblcProc_s AblcProc_t;

typedef struct aiq_shared_base_s AlgoRstShared_t;

typedef struct _RkAiqResComb {
#ifdef USE_IMPLEMENT_C
    AlgoRstShared_t* ae_pre_res_c;
    AlgoRstShared_t* ae_proc_res_c;
    AlgoRstShared_t* awb_proc_res_c;
#endif
    XCamVideoBuffer* ae_pre_res;
    XCamVideoBuffer* ae_proc_res;
    XCamVideoBuffer* awb_proc_res;
    AblcProc_t*       ablc_proc_res;
    AblcProc_V32_t*   ablcV32_proc_res;
    bool             bayernr3d_en;
    Aynr_ProcResult_V3_t* aynrV3_proc_res;
    Aynr_ProcResult_V22_t* aynrV22_proc_res;
    Aynr_ProcResult_V24_t* aynrV24_proc_res;
#if USE_NEWSTRUCT
    ynr_param_t* ynr_proc_res;
    bool blc_en;
    blc_param_t* blc_proc_res;
#endif
} RkAiqResComb;

/****************************************/
// old structs compatible with C, moved from
// rk_aiq_algo_types.h
typedef struct _RkAiqAlgoProcResAblc {
    RkAiqAlgoResCom res_com;
    AblcProc_t* ablc_proc_res;
} RkAiqAlgoProcResAblc;

typedef struct _RkAiqAlgoConfigAblc {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAblc;

typedef struct _RkAiqAlgoProcAblc {
    RkAiqAlgoCom com;
    int iso;
    int hdr_mode;
} RkAiqAlgoProcAblc;

typedef struct RkAiqAlgoProcResAeShared_s {
    bool IsConverged;
    bool IsEnvChanged;
    bool IsAutoAfd;
    bool LongFrmMode;
} RkAiqAlgoProcResAeShared_t;

typedef struct RkAiqAlgoProcResAwbShared_s {
    rk_aiq_wb_gain_t awb_gain_algo;
    float awb_smooth_factor;
    float varianceLuma;
    bool awbConverged;
} RkAiqAlgoProcResAwbShared_t;

typedef struct _RkAiqAlgoPreResAe {
    RkAiqAlgoResCom res_com;
    AecPreResult_t  ae_pre_res_rk;
} RkAiqAlgoPreResAe;

typedef struct _RkAiqSetStatsCfg  {
    bool isUpdate;              /* config update flag */
    bool UpdateStats;           /* stats update flag */
    bool UseSubWinStats;
    int8_t RawStatsChnSel;      /* RawStatsChnEn_t */
    int8_t YRangeMode;          /* CalibDb_CamYRangeModeV2_t */
    unsigned char* BigWeight;
    unsigned char* LiteWeight;
} RkAiqSetStatsCfg;

typedef struct _RkAiqAlgoProcResAe {
    RkAiqAlgoResCom res_com;
    rk_aiq_exposure_params_t* new_ae_exp;
    RKAiqExpI2cParam_t* exp_i2c_params;
    AecProcResult_t*   ae_proc_res_rk;
    RkAiqSetStatsCfg stats_cfg_to_trans;
#if USE_NEWSTRUCT
    aeStats_cfg_t*   ae_stats_cfg;
#else
    rk_aiq_isp_aec_meas_t* ae_meas;
    rk_aiq_isp_hist_meas_t* hist_meas;
#endif
} RkAiqAlgoProcResAe;

// Ae
typedef struct _RkAiqAlgoConfigAe {
    RkAiqAlgoCom com;
    /*params related to Hardware setting*/
    RkAiqAecHwConfig_t HwCtrl;
    /*params related to driver setting*/
    int                      RawWidth;
    int                      RawHeight;
    rk_aiq_sensor_nr_switch_t nr_switch;
    rk_aiq_sensor_dcg_ratio_t dcg_ratio;
    float        LinePeriodsPerField;
    float        PixelClockFreqMHZ;
    float        PixelPeriodsPerLine;
    int          compr_bit;
} RkAiqAlgoConfigAe;

typedef struct _RkAiqAlgoPreAe {
    RkAiqAlgoCom com;
    union {
        rk_aiq_isp_aec_stats_t* aecStatsBuf;
        RKAiqAecStatsV25_t*     aecStatsV25Buf;
    };
    bool af_prior;
    AmtdProcResult_t amtdRes;
} RkAiqAlgoPreAe;

typedef struct _RkAiqAlgoProcAe {
    RkAiqAlgoCom com;
    union {
        rk_aiq_isp_aec_stats_t* aecStatsBuf;
        RKAiqAecStatsV25_t*     aecStatsV25Buf;
    };
} RkAiqAlgoProcAe;

typedef struct _RkAiqAlgoPostAe {
    RkAiqAlgoCom com;
} RkAiqAlgoPostAe;

typedef struct _RkAiqAlgoPostResAe {
    RkAiqAlgoResCom res_com;
    AecPostResult_t ae_post_res_rk;
} RkAiqAlgoPostResAe;

// afd
typedef struct _RkAiqAlgoConfigAfd {
    RkAiqAlgoCom com;
    int          RawWidth;
    int          RawHeight;
    /*params related to driver setting*/
    float        LinePeriodsPerField;
    float        PixelClockFreqMHZ;
    float        PixelPeriodsPerLine;
} RkAiqAlgoConfigAfd;

typedef struct _RkAiqAlgoPreAfd {
    RkAiqAlgoCom com;
    int          thumbW;
    int          thumbH;
    XCamVideoBuffer* thumbStatsS;
    XCamVideoBuffer* thumbStatsL;
    RkAiqAlgoProcResAeShared_t* aeProcRes;
} RkAiqAlgoPreAfd;

typedef struct _RkAiqAlgoPreResAfd {
    RkAiqAlgoResCom res_com;
} RkAiqAlgoPreResAfd;

typedef struct _RkAiqAlgoProcAfd {
    RkAiqAlgoCom com;
    int hdr_mode;
} RkAiqAlgoProcAfd;

typedef struct _RkAiqAlgoProcResAfd {
    RkAiqAlgoResCom res_com;
    AfdProcResult_t afdRes;
} RkAiqAlgoProcResAfd;

// amtd
typedef struct _RkAiqAlgoConfigAmtd {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAmtd;

typedef struct _RkAiqAlgoPreAmtd {
    RkAiqAlgoCom com;
    AiqList_t* amdImuList;
} RkAiqAlgoPreAmtd;

typedef struct _RkAiqAlgoPreResAmtd {
    RkAiqAlgoResCom res_com;
} RkAiqAlgoPreResAmtd;

typedef struct _RkAiqAlgoProcAmtd {
    RkAiqAlgoCom com;
    AiqList_t* amdImuList;
} RkAiqAlgoProcAmtd;

typedef struct _RkAiqAlgoProcResAmtd {
    RkAiqAlgoResCom res_com;
    AmtdProcResult_t amtdRes;
} RkAiqAlgoProcResAmtd;


// acsm
typedef struct _RkAiqAlgoConfigAcsm {
    RkAiqAlgoCom com;
} RkAiqAlgoConfigAcsm;

typedef struct _RkAiqAlgoProcAcsm {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAcsm;

typedef struct _RkAiqAlgoProcResAcsm {
    RkAiqAlgoResCom res_com;
    rk_aiq_acsm_params_t* acsm_res;
} RkAiqAlgoProcResAcsm;

// forward declare
typedef struct isp_drv_share_mem_ops_s isp_drv_share_mem_ops_t;

//Awb
typedef struct _RkAiqAlgoConfigAwb {
    RkAiqAlgoCom com;
    int rawBit;
    isp_drv_share_mem_ops_t *mem_ops_ptr;
} RkAiqAlgoConfigAwb;

typedef struct _RkAiqAlgoProcAwb {
    RkAiqAlgoCom com;
    union {
        rk_aiq_awb_stat_res_v200_t* awb_statsBuf;
        rk_aiq_awb_stat_res_v201_t* awb_statsBuf_v201;
        rk_aiq_isp_awb_stats_v3x_t* awb_statsBuf_v3x;
        rk_aiq_isp_awb_stats_v32_t* awb_statsBuf_v32;
        awbStats_stats_priv_t* awb_statsBuf_v39;
    };
    rk_aiq_isp_blc_t* blc_cfg_effect;
    AblcProc_t* ablcProcRes;
    AblcProc_V32_t* ablcProcResV32;
    bool ablcProcResVaid;
    struct rkmodule_awb_inf *awb_otp;
    RKAiqAecStats_t* aecStatsBuf;
} RkAiqAlgoProcAwb;

typedef struct _RkAiqAlgoProcResAwb {
    RkAiqAlgoResCom res_com;
    bool awb_cfg_update;
    union {
        rk_aiq_awb_stat_cfg_v200_t* awb_hw0_para;
        rk_aiq_awb_stat_cfg_v201_t* awb_hw1_para;
        rk_aiq_awb_stat_cfg_v32_t* awb_hw32_para;
        awbStats_cfg_priv_t* awb_hw_cfg_priv;
    };
    bool awb_gain_update;
#if RKAIQ_HAVE_AWB_V32|| RKAIQ_HAVE_AWB_V39
    rk_aiq_wb_gain_v32_t* awb_gain_algo;
#else
    rk_aiq_wb_gain_t* awb_gain_algo;
#endif
#if !USE_IMPLEMENT_C
    rk_aiq_wb_gain_t awb_gain_algo_golden;
#endif
    color_tempture_info_t cctGloabl;
    color_tempture_info_t cctFirst[4];
    float awb_smooth_factor;
    float varianceLuma;
    bool awbConverged;

    uint32_t id;
} RkAiqAlgoProcResAwb;

/****************************************/

#if USE_NEWSTRUCT
/*
typedef struct {
    histeq_param_t histeq_param;
    dehaze_param_t dehaze_param;
    unsigned int MDehazeStrth;
    unsigned int MEnhanceStrth;
    unsigned int MEnhanceChromeStrth;
} rk_aiq_isp_dehaze_v39_t;
*/

typedef struct rk_aiq_isp_drc_v39_s {
    drc_param_t drc_param;
    trans_api_attrib_t trans_attr;
    float L2S_Ratio;
    unsigned char compr_bit;
    bool *damping;
} rk_aiq_isp_drc_v39_t;

typedef struct rk_aiq_isp_blc_v33_s {
    blc_param_t blc_param;
    bool *damping;
    bool *aeIsConverged;
} rk_aiq_isp_blc_v33_t;
#endif

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcBtnr;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    gamma_param_t* gammaRes;
#endif
} RkAiqAlgoProcResGamma;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    drc_param_t* drcRes;
#endif
} RkAiqAlgoProcResDrc;

#define DRC_AE_HIST_BIN_NUM (256)
typedef struct rkisp_adrc_stats_s {
    uint32_t frame_id;
    bool stats_true;
    int ae_hist_total_num;
    unsigned int aeHiatBins[DRC_AE_HIST_BIN_NUM];
} rkisp_adrc_stats_t;

typedef struct {
    RkAiqAlgoCom com;
    bool LongFrmMode;
    bool blc_ob_enable;
    bool aeIsConverged;
    float isp_ob_predgain;
    rkisp_adrc_stats_t drc_stats;
#if USE_NEWSTRUCT
    trans_params_static_t staTrans;
    FrameNumber_t FrameNumber;
    NextData_t NextData;
#endif
} RkAiqAlgoProcDrc;

typedef struct {
    RkAiqAlgoCom com;
    int working_mode;
    unsigned char compr_bit;
} RkAiqAlgoConfigDrc;

typedef struct {
    RkAiqAlgoCom com;
#if USE_NEWSTRUCT
    union {
        dehaze_stats_v12_t* dehaze_stats_v12;
        dehaze_stats_v14_t* dehaze_stats_v14;
    };
    bool stats_true;
    bool blc_ob_enable;
    float isp_ob_predgain;
#endif
} RkAiqAlgoProcDehaze;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    ynr_param_t* ynrRes;
#endif
} RkAiqAlgoProcResYnr;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcYnr;

typedef struct {
    RkAiqAlgoCom com;
    float blc_ob_predgain;
} RkAiqAlgoProcCnr;

typedef struct {
    RkAiqAlgoCom com;
    bool aeIsConverged;
    bool ishdr;
    bool damping;
} RkAiqAlgoProcBlc;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    blc_param_t* blcRes;
#endif
} RkAiqAlgoProcResBlc;

typedef struct _RkAiqAlgoProcResDpcc {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    dpc_param_t* dpcRes;
#endif
} RkAiqAlgoProcResDpcc;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    gic_param_t* gicRes;
#endif
} RkAiqAlgoProcResGic;

#if RKAIQ_HAVE_YUVME
typedef struct {
    RkAiqAlgoCom com;
    float ob_predgain;
} RkAiqAlgoProcYme;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    yme_param_t* ymeRes;
#endif
} RkAiqAlgoProcResYme;
#endif

typedef struct _RkAiqAlgoProcResCac {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    cac_param_t* cacRes;
#endif
} RkAiqAlgoProcResCac;

// to do  define  isp_drv_share_mem_ops_s and rk_aiq_lut_share_mem_info_t
typedef struct isp_drv_share_mem_ops_s isp_drv_share_mem_ops_t;
//#define RkAiqAwbStats // todo

typedef struct _RkAiqAlgoProcCac {
    RkAiqAlgoCom com;
    int iso;
} RkAiqAlgoProcCac;

typedef struct _RkAiqAlgoProcResLdch {
    RkAiqAlgoResCom res_com;
} RkAiqAlgoProcResLdch;

typedef struct _RkAiqAlgoConfigLdch {
    RkAiqAlgoCom com;
    const char* resource_path;
    isp_drv_share_mem_ops_t *mem_ops_ptr;
    bool is_multi_isp;
    uint8_t multi_isp_extended_pixel;
} RkAiqAlgoConfigLdch;

typedef struct _RkAiqAlgoProcResCsm {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    csm_param_t* csmRes;
#endif
} RkAiqAlgoProcResCsm;
typedef struct _RkAiqAlgoProcResMerge {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    mge_param_t* mergeRes;
#endif
} RkAiqAlgoProcResMerge;

typedef struct {
    RkAiqAlgoCom com;
    bool LongFrmMode;
} RkAiqAlgoProcMerge;

#if RKAIQ_HAVE_RGBIR_REMOSAIC
typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcRgbir;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    rgbir_param_t* rgbirRes;
#endif
} RkAiqAlgoProcResRgbir;
#endif

typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcCgc;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    cgc_param_t* cgcRes;
#endif
} RkAiqAlgoProcResCgc;

typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcCp;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    cp_param_t* cpRes;
#endif
} RkAiqAlgoProcResCp;

typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcIe;

typedef struct {
    RkAiqAlgoResCom res_com;
#if USE_NEWSTRUCT
    ie_param_t* ieRes;
#endif
} RkAiqAlgoProcResIe;

typedef struct _RkAiqAlgoConfigGain {
    RkAiqAlgoCom com;
    isp_drv_share_mem_ops_t *mem_ops_ptr;
} RkAiqAlgoConfigGain;

typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcGain;

typedef struct _RkAiqAlgoConfigAf {
    RkAiqAlgoCom com;
    unsigned char af_mode;
    unsigned short win_h_offs;
    unsigned short win_v_offs;
    unsigned short win_h_size;
    unsigned short win_v_size;
    rk_aiq_lens_descriptor lens_des;
    struct rkmodule_af_inf *otp_af;
    struct rkmodule_pdaf_inf *otp_pdaf;
} RkAiqAlgoConfigAf;

typedef struct _RkAiqAlgoProcAf {
    RkAiqAlgoCom com;
    union {
        rk_aiq_isp_af_stats_t* xcam_af_stats;
        rk_aiq_isp_af_stats_v3x_t* xcam_af_stats_v3x;
#ifdef USE_NEWSTRUCT
        afStats_stats_t *afStats_stats;
#endif
    };
    rk_aiq_af_algo_motor_stat_t* stat_motor;
    RKAiqAecExpInfo_t* aecExpInfo;
    RKAiqAecStats_t* xcam_aec_stats;
    rk_aiq_isp_pdaf_stats_t* xcam_pdaf_stats;
    bool ae_stable;
} RkAiqAlgoProcAf;

typedef struct _RkAiqAlgoProcResAf {
    RkAiqAlgoResCom res_com;
    bool af_cfg_update;
    bool af_focus_update;
    bool lockae;
    bool lockae_en;
    union {
        rk_aiq_isp_af_meas_t* af_isp_param;
        rk_aiq_isp_af_meas_v3x_t* af_isp_param_v3x;
        rk_aiq_isp_af_v31_t* af_isp_param_v31;
        rk_aiq_isp_af_v32_t* af_isp_param_v32;
        rk_aiq_isp_af_v33_t* af_isp_param_v33;
#ifdef USE_NEWSTRUCT
        afStats_cfg_t *afStats_cfg;
#endif
    };
    rk_aiq_af_focus_pos_meas_t* af_focus_param;
    uint32_t id;
} RkAiqAlgoProcResAf;

typedef struct _RkAiqAeSetStatsCfg  {
    bool isUpdate;              /* config update flag */
    bool UpdateStats;           /* stats update flag */
    int8_t RawStatsChnSel;      /* RawStatsChnEn_t */
    int8_t YRangeMode;          /* CalibDb_CamYRangeModeV2_t */
    unsigned char* BigWeight;
    unsigned char* LiteWeight;
} RkAiqAeSetStatsCfg;

typedef struct {
    float sensorGain;
    float awbGain[2];
    float awbIIRDampCoef;
    float varianceLuma;
    bool grayMode;
    bool awbConverged;
    uint32_t count;
} illu_estm_info_t;


typedef struct {
    RkAiqAlgoCom com;
    illu_estm_info_t illu_info;
} RkAiqAlgoProcLut3d;

typedef struct {
    RkAiqAlgoCom com;
    illu_estm_info_t illu_info;
} RkAiqAlgoProcHsv;

typedef struct {
    RkAiqAlgoCom com;
    illu_estm_info_t illu_info;
} RkAiqAlgoProcCcm;

typedef struct {
    RkAiqAlgoCom com;
    illu_estm_info_t illu_info;
} RkAiqAlgoProcLsc;

#if USE_NEWSTRUCT
// aldc
typedef struct _RkAiqAlgoConfigLdc {
    RkAiqAlgoCom com;
    isp_drv_share_mem_ops_t* mem_ops;
    bool is_multi_isp;
    char iqpath[255];
} RkAiqAlgoConfigLdc;

typedef struct _RkAiqAlgoProcLdc {
    RkAiqAlgoCom com;
    AiqLdcUpdMeshMode upd_mesh_mode;
} RkAiqAlgoProcLdc;
#endif

// aibnr
typedef struct rk_aiq_isp_aibnr_params_s {
    aibnr_param_t aibnr_param;
    bool is_bypass;
    bool is_hdr;
    uint8_t bayer_fmt;
    uint16_t prev_blacklvl;
    uint16_t post_blacklvl;
    uint16_t sw_in_comp_y[33];
    uint16_t sw_out_decomp_y[33];
    int frameIso;
} rk_aiq_isp_aibnr_params_t;

typedef struct {
    RkAiqAlgoResCom res_com;
    aibnr_param_t* aibnrRes;
} RkAiqAlgoProcResAibnr;

typedef struct {
    RkAiqAlgoCom com;
} RkAiqAlgoProcAibnr;

// airms
typedef airms_param_t rk_aiq_isp_airms_params_t;

// aiynr
typedef struct rk_aiq_isp_aiynr_params_s {
    aiynr_param_t aiynr_param;
    bool is_bypass;
    bool is_hdr;
    bool sw_nar_inverse;
    uint16_t sw_neg_noiselimit;
    uint16_t sw_pos_noiselimit;
    uint16_t sw_in_comp_y[33];
    uint16_t sw_out_decomp_y[33];
} rk_aiq_isp_aiynr_params_t;

#endif
