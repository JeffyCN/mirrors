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
#ifndef RK_AIQ_POOL_H
#define RK_AIQ_POOL_H

#include <map>

#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_priv.h"
#include "shared_item_pool.h"
#include "thumbnails.h"

namespace RkCam {

typedef struct RKAiqAecExpInfoWrapper_s {
    rk_aiq_exposure_params_t new_ae_exp;
    RKAiqExpI2cParam_t exp_i2c_params;
    AecProcResult_t      ae_proc_res_rk;
    Sensor_dpcc_res_t SensorDpccInfo;
    int algo_id;
    RKAiqAecExpInfoWrapper_s() {
        exp_i2c_params.bValid = false;
        memset(&new_ae_exp, 0, sizeof(new_ae_exp));
        memset(&ae_proc_res_rk, 0, sizeof(ae_proc_res_rk));
        algo_id = 0;
    };
} RKAiqAecExpInfoWrapper_t;

typedef struct RKAiqSensorExpInfo_t: public XCam::BufferData {
    RKAiqSensorExpInfo_t () {
        memset(&aecExpInfo, 0, sizeof(aecExpInfo));
        exp_i2c_params = NULL;
    }
    rk_aiq_exposure_params_t aecExpInfo;
    Sensor_dpcc_res_t SensorDpccInfo;
    RKAiqExpI2cParam_t* exp_i2c_params;
} RKAiqSensorExpInfo_t;

typedef rk_aiq_isp_params_t<RKAiqAecExpInfoWrapper_t> rk_aiq_exposure_params_wrapper_t;
typedef SharedItemPool<rk_aiq_exposure_params_wrapper_t> RkAiqExpParamsPool;
typedef SharedItemProxy<rk_aiq_exposure_params_wrapper_t> RkAiqExpParamsProxy;

typedef RKAiqSensorExpInfo_t rk_aiq_sensor_exp_info_t;
typedef SharedItemPool<rk_aiq_sensor_exp_info_t> RkAiqSensorExpParamsPool;
typedef SharedItemProxy<rk_aiq_sensor_exp_info_t> RkAiqSensorExpParamsProxy;

typedef struct RkAiqSofInfoWrapper_s : public XCam::BufferData {
    uint32_t sequence;
    SmartPtr<RkAiqSensorExpParamsProxy> preExp;
    SmartPtr<RkAiqSensorExpParamsProxy> curExp;
    SmartPtr<RkAiqSensorExpParamsProxy> nxtExp;
    int64_t sof;
    int iso;
    void reset() {
        preExp.release();
        curExp.release();
        nxtExp.release();
        iso = 0;
    }
} RkAiqSofInfoWrapper_t;

typedef struct RKAiqAfInfoWrapper_s : public XCam::BufferData {
    struct timeval focusStartTim;
    struct timeval focusEndTim;
    struct timeval zoomStartTim;
    struct timeval zoomEndTim;
    int64_t sofTime;
    int32_t focusCode;
    int32_t zoomCode;
    float angleZ;
    uint32_t lowPassId;
    int32_t lowPassFv4_4[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassFv8_8[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassHighLht[RKAIQ_RAWAF_SUMDATA_NUM];
    int32_t lowPassHighLht2[RKAIQ_RAWAF_SUMDATA_NUM];
    bool zoomCorrection;
    bool focusCorrection;
} RKAiqAfInfoWrapper_t;

typedef struct RkAiqPirisInfoWrapper_s {
    int             step;
    int             laststep;
    bool            update;
    struct timeval  StartTim;
    struct timeval  EndTim;
} RkAiqPirisInfoWrapper_t;

typedef struct RkAiqIrisInfoWrapper_s : public XCam::BufferData {
    //TODO Merge
    //RkAiqIrisType_t           IrisType;
    RkAiqPirisInfoWrapper_t   PIris;
    RkAiqDCIrisParam_t        DCIris;
    RkAiqHDCIrisParam_t       HDCIris;
    uint64_t                  sofTime;
} RkAiqIrisInfoWrapper_t;

typedef struct RKAiqCpslInfoWrapper_s : public XCam::BufferData {
    rk_aiq_flash_setting_t fl;
    bool update_fl;
    rk_aiq_ir_setting_t ir;
    rk_aiq_flash_setting_t fl_ir;
    bool update_ir;
    uint32_t frame_id;
} RKAiqCpslInfoWrapper_t;

typedef enum _cam3aResultType {
    RESULT_TYPE_INVALID = -1,
    RESULT_TYPE_EXPOSURE_PARAM,
    RESULT_TYPE_AEC_PARAM,
    RESULT_TYPE_HIST_PARAM,
    RESULT_TYPE_AWB_PARAM,
    RESULT_TYPE_AWBGAIN_PARAM,
    RESULT_TYPE_AF_PARAM,
    RESULT_TYPE_DPCC_PARAM,
    RESULT_TYPE_MERGE_PARAM,
    RESULT_TYPE_TMO_PARAM,
    RESULT_TYPE_CCM_PARAM,
    RESULT_TYPE_LSC_PARAM,
    RESULT_TYPE_BLC_PARAM,
    RESULT_TYPE_RAWNR_PARAM,
    RESULT_TYPE_GIC_PARAM,
    RESULT_TYPE_DEBAYER_PARAM,
    RESULT_TYPE_LDCH_PARAM,
    RESULT_TYPE_LUT3D_PARAM = 0x10,
    RESULT_TYPE_DEHAZE_PARAM,
    RESULT_TYPE_AGAMMA_PARAM,
    RESULT_TYPE_ADEGAMMA_PARAM,
    RESULT_TYPE_WDR_PARAM,
    RESULT_TYPE_CSM_PARAM,
    RESULT_TYPE_CGC_PARAM,
    RESULT_TYPE_CONV422_PARAM,
    RESULT_TYPE_YUVCONV_PARAM,
    RESULT_TYPE_GAIN_PARAM,
    RESULT_TYPE_CP_PARAM,
    RESULT_TYPE_IE_PARAM,
    RESULT_TYPE_MOTION_PARAM,
    RESULT_TYPE_IRIS_PARAM,
    RESULT_TYPE_CPSL_PARAM,
    RESULT_TYPE_FLASH_PARAM,

    RESULT_TYPE_TNR_PARAM = 0x20,
    RESULT_TYPE_YNR_PARAM,
    RESULT_TYPE_UVNR_PARAM,
    RESULT_TYPE_SHARPEN_PARAM,
    RESULT_TYPE_EDGEFLT_PARAM,
    RESULT_TYPE_FEC_PARAM,
    RESULT_TYPE_ORB_PARAM,

    RESULT_TYPE_FOCUS_PARAM,

    // isp21 result
    RESULT_TYPE_DRC_PARAM = 0x28,
    // isp3x result
    RESULT_TYPE_CAC_PARAM = 0x29,
    RESULT_TYPE_AFD_PARAM = 0x2a,
    // isp39 result
    RESULT_TYPE_RGBIR_PARAM = 0x2b,
    RESULT_TYPE_TRANS_PARAM = 0x2c,
    RESULT_TYPE_MAX_PARAM,
} cam3aResultType;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
static const char* Cam3aResultType2Str[RESULT_TYPE_MAX_PARAM] = {
    [RESULT_TYPE_EXPOSURE_PARAM] = "EXPOSURE",
    [RESULT_TYPE_AEC_PARAM]      = "AEC",
    [RESULT_TYPE_HIST_PARAM]     = "HIST",
    [RESULT_TYPE_AWB_PARAM]      = "AWB",
    [RESULT_TYPE_AWBGAIN_PARAM]  = "AWB_GAIN",
    [RESULT_TYPE_AF_PARAM]       = "AF",
    [RESULT_TYPE_DPCC_PARAM]     = "DPCC",
    [RESULT_TYPE_MERGE_PARAM]    = "MERGE",
    [RESULT_TYPE_TMO_PARAM]      = "TMO",
    [RESULT_TYPE_CCM_PARAM]      = "CCM",
    [RESULT_TYPE_LSC_PARAM]      = "LSC",
    [RESULT_TYPE_BLC_PARAM]      = "BLC",
    [RESULT_TYPE_RAWNR_PARAM]    = "RAWNR",
    [RESULT_TYPE_GIC_PARAM]      = "GIC",
    [RESULT_TYPE_DEBAYER_PARAM]  = "DEBAYER",
    [RESULT_TYPE_LDCH_PARAM]     = "LDCH",
    [RESULT_TYPE_LUT3D_PARAM]    = "LUT3D",
    [RESULT_TYPE_DEHAZE_PARAM]   = "DEHAZE",
    [RESULT_TYPE_AGAMMA_PARAM]   = "AGAMMA",
    [RESULT_TYPE_ADEGAMMA_PARAM] = "ADEGAMMA",
    [RESULT_TYPE_WDR_PARAM]      = "WDR",
    [RESULT_TYPE_CSM_PARAM]      = "CSM",
    [RESULT_TYPE_CGC_PARAM]      = "CGC",
    [RESULT_TYPE_CONV422_PARAM]  = "CONV22",
    [RESULT_TYPE_YUVCONV_PARAM]  = "YUVCONV",
    [RESULT_TYPE_GAIN_PARAM]     = "GAIN",
    [RESULT_TYPE_CP_PARAM]       = "CP",
    [RESULT_TYPE_IE_PARAM]       = "IE",
    [RESULT_TYPE_MOTION_PARAM]   = "MOTION",
    [RESULT_TYPE_IRIS_PARAM]     = "IRIS",
    [RESULT_TYPE_CPSL_PARAM]     = "CPSL",
    [RESULT_TYPE_FLASH_PARAM]    = "FLASH",
    [RESULT_TYPE_TNR_PARAM]      = "TNR",
    [RESULT_TYPE_YNR_PARAM]      = "YNR",
    [RESULT_TYPE_UVNR_PARAM]     = "UVNR",
    [RESULT_TYPE_SHARPEN_PARAM]  = "SHARPEN",
    [RESULT_TYPE_EDGEFLT_PARAM]  = "EDGEFLT",
    [RESULT_TYPE_FEC_PARAM]      = "FEC",
    [RESULT_TYPE_ORB_PARAM]      = "ORB",
    [RESULT_TYPE_FOCUS_PARAM]    = "FOCUS",
    [RESULT_TYPE_DRC_PARAM]      = "DRC",
    [RESULT_TYPE_CAC_PARAM]      = "CAC",
    [RESULT_TYPE_AFD_PARAM]      = "AFD",
    [RESULT_TYPE_RGBIR_PARAM]    = "RGBIR",
    [RESULT_TYPE_TRANS_PARAM]    = "TRANS",
};
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

typedef SharedItemBase cam3aResult;
typedef std::list<SmartPtr< cam3aResult>> cam3aResultList;
typedef std::map<int, std::list<SmartPtr<cam3aResult>>> cam3aResultMap;

typedef struct RkAiqThumbnailSrcWrapper_s : public XCam::BufferData {
    SmartPtr<rkaiq_image_source_t> thumbnail;
} RkAiqThumbnailSrcWrapper_t;

typedef SharedItemPool<RkAiqIspStats> RkAiqIspStatsIntPool;
typedef SharedItemProxy<RkAiqIspStats> RkAiqIspStatsIntProxy;

typedef SharedItemPool<RkAiqAecStats> RkAiqAecStatsPool;
typedef SharedItemProxy<RkAiqAecStats> RkAiqAecStatsProxy;

typedef SharedItemPool<RkAiqAwbStats> RkAiqAwbStatsPool;
typedef SharedItemProxy<RkAiqAwbStats> RkAiqAwbStatsProxy;

typedef SharedItemPool<RkAiqAtmoStats> RkAiqAtmoStatsPool;
typedef SharedItemProxy<RkAiqAtmoStats> RkAiqAtmoStatsProxy;

typedef SharedItemPool<RkAiqAdehazeStats> RkAiqAdehazeStatsPool;
typedef SharedItemProxy<RkAiqAdehazeStats> RkAiqAdehazeStatsProxy;

typedef SharedItemPool<RkAiqAgainStats> RkAiqAgainStatsPool;
typedef SharedItemProxy<RkAiqAgainStats> RkAiqAgainStatsProxy;

typedef SharedItemPool<RkAiqAfStats> RkAiqAfStatsPool;
typedef SharedItemProxy<RkAiqAfStats> RkAiqAfStatsProxy;

typedef SharedItemPool<RkAiqPdafStats> RkAiqPdafStatsPool;
typedef SharedItemProxy<RkAiqPdafStats> RkAiqPdafStatsProxy;

typedef SharedItemPool<RkAiqSofInfoWrapper_t> RkAiqSofInfoWrapperPool;
typedef SharedItemProxy<RkAiqSofInfoWrapper_t> RkAiqSofInfoWrapperProxy;

typedef SharedItemPool<RkAiqOrbStats> RkAiqOrbStatsPool;
typedef SharedItemProxy<RkAiqOrbStats> RkAiqOrbStatsProxy;

typedef SharedItemPool<RkAiqThumbnailSrcWrapper_t> RkAiqThumbnailSrcPool;
typedef SharedItemProxy<RkAiqThumbnailSrcWrapper_t> RkAiqThumbnailSrcProxy;

class RkAiqIspStats : public XCam::BufferData {
public:
    explicit RkAiqIspStats() {
        aec_stats_valid      = false;
        awb_stats_valid      = false;
        awb_cfg_effect_valid = false;
        af_stats_valid       = false;
        atmo_stats_valid     = false;
        adehaze_stats_valid  = false;
        frame_id             = -1;
    };
    virtual ~RkAiqIspStats() {
        reset();
    };
    void reset() {
        AecStatsProxy.release();
        AwbStatsProxy.release();
        AfStatsProxy.release();
        AtmoStatsProxy.release();
        AdehazeStatsProxy.release();
    }
    SmartPtr<RkAiqAecStatsProxy> AecStatsProxy = nullptr;
    bool aec_stats_valid;
    SmartPtr<RkAiqAwbStatsProxy> AwbStatsProxy = nullptr;
    bool awb_stats_valid;
    bool awb_cfg_effect_valid;
    SmartPtr<RkAiqAfStatsProxy> AfStatsProxy = nullptr;
    bool af_stats_valid;
    SmartPtr<RkAiqAtmoStatsProxy> AtmoStatsProxy = nullptr;
    bool atmo_stats_valid;
    SmartPtr<RkAiqAdehazeStatsProxy> AdehazeStatsProxy = nullptr;
    bool adehaze_stats_valid;
    uint32_t frame_id;

private:
    XCAM_DEAD_COPY(RkAiqIspStats);
};

typedef RKAiqAfInfoWrapper_t rk_aiq_af_info_wrapper_t;
typedef RkAiqIrisInfoWrapper_t rk_aiq_iris_params_wrapper_t;

typedef SharedItemPool<rk_aiq_iris_params_wrapper_t> RkAiqIrisParamsPool;
typedef SharedItemProxy<rk_aiq_iris_params_wrapper_t> RkAiqIrisParamsProxy;

typedef SharedItemPool<rk_aiq_af_info_wrapper_t> RkAiqAfInfoPool;
typedef SharedItemProxy<rk_aiq_af_info_wrapper_t> RkAiqAfInfoProxy;
typedef SharedItemPool<rk_aiq_focus_params_wrapper_t> RkAiqFocusParamsPool;
typedef SharedItemProxy<rk_aiq_focus_params_wrapper_t> RkAiqFocusParamsProxy;
typedef SharedItemPool<RKAiqCpslInfoWrapper_t> RkAiqCpslParamsPool;
typedef SharedItemProxy<RKAiqCpslInfoWrapper_t> RkAiqCpslParamsProxy;

typedef SharedItemPool<rk_aiq_isp_aec_params_t>         RkAiqIspAecParamsPool;
typedef SharedItemProxy<rk_aiq_isp_aec_params_t>        RkAiqIspAecParamsProxy;
typedef SharedItemPool<rk_aiq_isp_hist_params_t>        RkAiqIspHistParamsPool;
typedef SharedItemProxy<rk_aiq_isp_hist_params_t>       RkAiqIspHistParamsProxy;
typedef SharedItemPool<rk_aiq_isp_awb_params_t>         RkAiqIspAwbParamsPool;
typedef SharedItemProxy<rk_aiq_isp_awb_params_t>        RkAiqIspAwbParamsProxy;
typedef SharedItemPool<rk_aiq_isp_awb_gain_params_t>    RkAiqIspAwbGainParamsPool;
typedef SharedItemProxy<rk_aiq_isp_awb_gain_params_t>   RkAiqIspAwbGainParamsProxy;
typedef SharedItemPool<rk_aiq_isp_af_params_t>          RkAiqIspAfParamsPool;
typedef SharedItemProxy<rk_aiq_isp_af_params_t>         RkAiqIspAfParamsProxy;
typedef SharedItemPool<rk_aiq_isp_dpcc_params_t>        RkAiqIspDpccParamsPool;
typedef SharedItemProxy<rk_aiq_isp_dpcc_params_t>       RkAiqIspDpccParamsProxy;
typedef SharedItemPool<rk_aiq_isp_merge_params_t>       RkAiqIspMergeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_merge_params_t>      RkAiqIspMergeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_tmo_params_t>         RkAiqIspTmoParamsPool;
typedef SharedItemProxy<rk_aiq_isp_tmo_params_t>        RkAiqIspTmoParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ccm_params_t>         RkAiqIspCcmParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ccm_params_t>        RkAiqIspCcmParamsProxy;
typedef SharedItemPool<rk_aiq_isp_lsc_params_t>         RkAiqIspLscParamsPool;
typedef SharedItemProxy<rk_aiq_isp_lsc_params_t>        RkAiqIspLscParamsProxy;
typedef SharedItemPool<rk_aiq_isp_blc_params_t>         RkAiqIspBlcParamsPool;
typedef SharedItemProxy<rk_aiq_isp_blc_params_t>        RkAiqIspBlcParamsProxy;
typedef SharedItemPool<rk_aiq_isp_rawnr_params_t>       RkAiqIspRawnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_rawnr_params_t>      RkAiqIspRawnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_gic_params_t>         RkAiqIspGicParamsPool;
typedef SharedItemProxy<rk_aiq_isp_gic_params_t>        RkAiqIspGicParamsProxy;
typedef SharedItemPool<rk_aiq_isp_debayer_params_t>     RkAiqIspDebayerParamsPool;
typedef SharedItemProxy<rk_aiq_isp_debayer_params_t>    RkAiqIspDebayerParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ldch_params_t>        RkAiqIspLdchParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ldch_params_t>       RkAiqIspLdchParamsProxy;
typedef SharedItemPool<rk_aiq_isp_lut3d_params_t>       RkAiqIspLut3dParamsPool;
typedef SharedItemProxy<rk_aiq_isp_lut3d_params_t>      RkAiqIspLut3dParamsProxy;
typedef SharedItemPool<rk_aiq_isp_dehaze_params_t>      RkAiqIspDehazeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_dehaze_params_t>     RkAiqIspDehazeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_agamma_params_t>      RkAiqIspAgammaParamsPool;
typedef SharedItemProxy<rk_aiq_isp_agamma_params_t>     RkAiqIspAgammaParamsProxy;
typedef SharedItemPool<rk_aiq_isp_adegamma_params_t>    RkAiqIspAdegammaParamsPool;
typedef SharedItemProxy<rk_aiq_isp_adegamma_params_t>   RkAiqIspAdegammaParamsProxy;
typedef SharedItemPool<rk_aiq_isp_wdr_params_t>         RkAiqIspWdrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_wdr_params_t>        RkAiqIspWdrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_csm_params_t>         RkAiqIspCsmParamsPool;
typedef SharedItemProxy<rk_aiq_isp_csm_params_t>        RkAiqIspCsmParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cgc_params_t>         RkAiqIspCgcParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cgc_params_t>        RkAiqIspCgcParamsProxy;
typedef SharedItemPool<rk_aiq_isp_conv422_params_t>     RkAiqIspConv422ParamsPool;
typedef SharedItemProxy<rk_aiq_isp_conv422_params_t>    RkAiqIspConv422ParamsProxy;
typedef SharedItemPool<rk_aiq_isp_yuvconv_params_t>     RkAiqIspYuvconvParamsPool;
typedef SharedItemProxy<rk_aiq_isp_yuvconv_params_t>    RkAiqIspYuvconvParamsProxy;
typedef SharedItemPool<rk_aiq_isp_gain_params_t>        RkAiqIspGainParamsPool;
typedef SharedItemProxy<rk_aiq_isp_gain_params_t>       RkAiqIspGainParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cp_params_t>          RkAiqIspCpParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cp_params_t>         RkAiqIspCpParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ie_params_t>          RkAiqIspIeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ie_params_t>         RkAiqIspIeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_motion_params_t>      RkAiqIspMotionParamsPool;
typedef SharedItemProxy<rk_aiq_isp_motion_params_t>     RkAiqIspMotionParamsProxy;
typedef SharedItemPool<rk_aiq_isp_md_params_t>          RkAiqIspMdParamsPool;
typedef SharedItemProxy<rk_aiq_isp_md_params_t>         RkAiqIspMdParamsProxy;
typedef SharedItemPool<rk_aiq_isp_tnr_params_t>         RkAiqIspTnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_tnr_params_t>        RkAiqIspTnrParamsProxy;
#if (USE_NEWSTRUCT == 0)
typedef SharedItemPool<rk_aiq_isp_ynr_params_t>         RkAiqIspYnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ynr_params_t>        RkAiqIspYnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cnr_params_t>         RkAiqIspCnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cnr_params_t>        RkAiqIspCnrParamsProxy;
#endif
typedef SharedItemPool<rk_aiq_isp_uvnr_params_t>        RkAiqIspUvnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_uvnr_params_t>       RkAiqIspUvnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_sharpen_params_t>     RkAiqIspSharpenParamsPool;
typedef SharedItemProxy<rk_aiq_isp_sharpen_params_t>    RkAiqIspSharpenParamsProxy;
typedef SharedItemPool<rk_aiq_isp_edgeflt_params_t>     RkAiqIspEdgefltParamsPool;
typedef SharedItemProxy<rk_aiq_isp_edgeflt_params_t>    RkAiqIspEdgefltParamsProxy;
typedef SharedItemPool<rk_aiq_isp_fec_params_t>         RkAiqIspFecParamsPool;
typedef SharedItemProxy<rk_aiq_isp_fec_params_t>        RkAiqIspFecParamsProxy;
typedef SharedItemPool<rk_aiq_isp_orb_params_t>         RkAiqIspOrbParamsPool;
typedef SharedItemProxy<rk_aiq_isp_orb_params_t>        RkAiqIspOrbParamsProxy;
typedef SharedItemPool<rk_aiq_isp_afd_params_t>         RkAiqIspAfdParamsPool;
typedef SharedItemProxy<rk_aiq_isp_afd_params_t>        RkAiqIspAfdParamsProxy;

//v21 pools
typedef SharedItemPool<rk_aiq_isp_drc_params_t>         RkAiqIspDrcParamsPool;
typedef SharedItemProxy<rk_aiq_isp_drc_params_t>        RkAiqIspDrcParamsProxy;
typedef SharedItemPool<rk_aiq_isp_baynr_params_t>       RkAiqIspBaynrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_baynr_params_t>      RkAiqIspBaynrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_bay3d_params_t>       RkAiqIspBa3dParamsPool;
typedef SharedItemProxy<rk_aiq_isp_bay3d_params_t>      RkAiqIspBa3dParamsProxy;

//v3x pools
#if RKAIQ_HAVE_CAC
typedef SharedItemPool<rk_aiq_isp_cac_params_t>         RkAiqIspCacParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cac_params_t>        RkAiqIspCacParamsProxy;
#endif
//v39 pools
#if RKAIQ_HAVE_YUVME
typedef SharedItemPool<rk_aiq_isp_yuvme_params_t>       RkAiqIspYuvmeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_yuvme_params_t>      RkAiqIspYuvmeParamsProxy;
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
typedef SharedItemPool<rk_aiq_isp_rgbir_params_t>       RkAiqIspRgbirParamsPool;
typedef SharedItemProxy<rk_aiq_isp_rgbir_params_t>      RkAiqIspRgbirParamsProxy;
#endif
// new struct
#if USE_NEWSTRUCT
typedef SharedItemPool<rk_aiq_isp_dm_params_t> RkAiqIspDmParamsPool;
typedef SharedItemProxy<rk_aiq_isp_dm_params_t> RkAiqIspDmParamsProxy;
typedef SharedItemPool<rk_aiq_isp_btnr_params_t> RkAiqIspBtnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_btnr_params_t> RkAiqIspBtnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_gamma_params_t>      RkAiqIspGammaParamsPool;
typedef SharedItemProxy<rk_aiq_isp_gamma_params_t>     RkAiqIspGammaParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ynr_params_t> RkAiqIspYnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ynr_params_t> RkAiqIspYnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_sharp_params_t> RkAiqIspSharpParamsPool;
typedef SharedItemProxy<rk_aiq_isp_sharp_params_t> RkAiqIspSharpParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cnr_params_t> RkAiqIspCnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cnr_params_t> RkAiqIspCnrParamsProxy;
#endif

class RkAiqFullParams : public XCam::BufferData {
public:
   explicit RkAiqFullParams()
       : mFrmId(0),
         mExposureParams(NULL),
         mFocusParams(NULL),
         mIrisParams(NULL),
         mCpslParams(NULL)

         ,
         mAecParams(NULL),
         mHistParams(NULL),
         mAwbParams(NULL),
         mAwbGainParams(NULL),
         mAfParams(NULL),
#ifndef USE_NEWSTRUCT
         mDpccParams(NULL),
#endif
         mMergeParams(NULL),
         mTmoParams(NULL),
         mCcmParams(NULL),
         mLscParams(NULL),
         mBlcParams(NULL),
         mRawnrParams(NULL),
         mGicParams(NULL),
         mDebayerParams(NULL),
         mLdchParams(NULL),
         mLut3dParams(NULL),
         mDehazeParams(NULL),
         mAgammaParams(NULL),
         mWdrParams(NULL),
         mCsmParams(NULL),
         mCgcParams(NULL),
         mConv422Params(NULL),
         mYuvconvParams(NULL),
         mGainParams(NULL),
         mCpParams(NULL),
         mIeParams(NULL),
         mMotionParams(NULL),
         mMdParams(NULL)

         ,
         mTnrParams(NULL),
         mYnrParams(NULL),
         mUvnrParams(NULL),
         mSharpenParams(NULL),
         mEdgefltParams(NULL),
         mFecParams(NULL),
         mOrbParams(NULL)
         // TODO: change full params to list
         // V21 differential modules
         ,
         mDrcParams(NULL),
         mBaynrParams(NULL)
   // , mBa3dParams(NULL)
   // V39 differential modules
#if RKAIQ_HAVE_CAC
         ,
         mCacParams(NULL)
#endif
         ,
         mAfdParams(NULL)
#if RKAIQ_HAVE_YUVME
         ,
         mYuvmeParams(NULL)
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
         ,
         mRgbirParams(NULL)
#endif
#if USE_NEWSTRUCT
         ,
         mDmParams(NULL),
         mBtnrParams(NULL),
         mGammaParams(NULL),
         mSharpParams(NULL)
#endif
             {};
    ~RkAiqFullParams() {
        reset();
    };

    void reset() {
#if 0// Do not release to save cpu usage
        // MUST release, cause some malloc was made in mExposureParams
        mExposureParams.release();
        mFocusParams.release();
        mIrisParams.release();
        mCpslParams.release();

        mAecParams.release();
        mHistParams.release();
        mAwbParams.release();
        mAwbGainParams.release();
        mAfParams.release();
#ifndef USE_NEWSTRUCT
        mDpccParams.release();
#endif
        mMergeParams.release();
        mTmoParams.release();
        mCcmParams.release();
        mLscParams.release();
        mBlcParams.release();
        mRawnrParams.release();
        mGicParams.release();
        mDebayerParams.release();
        mLdchParams.release();
        mLut3dParams.release();
        mDehazeParams.release();
        mAgammaParams.release();
        mAdegammaParams.release();
        mWdrParams.release();
        mCsmParams.release();
        mCgcParams.release();
        mConv422Params.release();
        mYuvconvParams.release();
        mGainParams.release();
        mCpParams.release();
        mIeParams.release();
        mMotionParams.release();
        mMdParams.release();

        mTnrParams.release();
        mYnrParams.release();
        mUvnrParams.release();
        mSharpenParams.release();
        mEdgefltParams.release();
        mFecParams.release();
        mOrbParams.release();

        // TODO: change full params to list
        // V21 differential modules
        mDrcParams.release();
        mBaynrParams.release();
        // mBa3dParams.release();
        mCnrParams.release();
        // TODO: change full params to list
        // V3x differential modules
#if RKAIQ_HAVE_CAC
        mCacParams.release();
#endif
        // V32 lite differential modules
        mAfdParams.release();
#endif
   };
    uint32_t                                mFrmId;
    SmartPtr<RkAiqExpParamsProxy>           mExposureParams;
    SmartPtr<RkAiqFocusParamsProxy>         mFocusParams;
    SmartPtr<RkAiqIrisParamsProxy>          mIrisParams;
    SmartPtr<RkAiqCpslParamsProxy>          mCpslParams;
    SmartPtr<RkAiqIspAecParamsProxy>        mAecParams;
    SmartPtr<RkAiqIspHistParamsProxy>       mHistParams;
    SmartPtr<RkAiqIspAwbParamsProxy>        mAwbParams;
    SmartPtr<RkAiqIspAwbGainParamsProxy>    mAwbGainParams;
    SmartPtr<RkAiqIspAfParamsProxy>         mAfParams;
    SmartPtr<RkAiqIspDpccParamsProxy>       mDpccParams;
    // SmartPtr<RkAiqIspHdrParamsProxy>        mHdrParams;
    SmartPtr<RkAiqIspMergeParamsProxy>      mMergeParams;
    SmartPtr<RkAiqIspTmoParamsProxy>        mTmoParams;
    SmartPtr<RkAiqIspCcmParamsProxy>        mCcmParams;
    SmartPtr<RkAiqIspLscParamsProxy>        mLscParams;
    SmartPtr<RkAiqIspBlcParamsProxy>        mBlcParams;
    SmartPtr<RkAiqIspRawnrParamsProxy>      mRawnrParams;
    SmartPtr<RkAiqIspGicParamsProxy>        mGicParams;
    SmartPtr<RkAiqIspDebayerParamsProxy>    mDebayerParams;
    SmartPtr<RkAiqIspLdchParamsProxy>       mLdchParams;
    SmartPtr<RkAiqIspLut3dParamsProxy>      mLut3dParams;
    SmartPtr<RkAiqIspDehazeParamsProxy>     mDehazeParams;
    SmartPtr<RkAiqIspAgammaParamsProxy>     mAgammaParams;
    SmartPtr<RkAiqIspAdegammaParamsProxy>   mAdegammaParams;
    SmartPtr<RkAiqIspWdrParamsProxy>        mWdrParams;
    SmartPtr<RkAiqIspCsmParamsProxy>        mCsmParams;
    SmartPtr<RkAiqIspCgcParamsProxy>        mCgcParams;
    SmartPtr<RkAiqIspConv422ParamsProxy>    mConv422Params;
    SmartPtr<RkAiqIspYuvconvParamsProxy>    mYuvconvParams;
    SmartPtr<RkAiqIspGainParamsProxy>       mGainParams;
    SmartPtr<RkAiqIspCpParamsProxy>         mCpParams;
    SmartPtr<RkAiqIspIeParamsProxy>         mIeParams;
    SmartPtr<RkAiqIspMotionParamsProxy>     mMotionParams;
    SmartPtr<RkAiqIspMdParamsProxy>         mMdParams;

    SmartPtr<RkAiqIspTnrParamsProxy>        mTnrParams;
    SmartPtr<RkAiqIspYnrParamsProxy>        mYnrParams;
    SmartPtr<RkAiqIspUvnrParamsProxy>       mUvnrParams;
    SmartPtr<RkAiqIspSharpenParamsProxy>    mSharpenParams;
    SmartPtr<RkAiqIspEdgefltParamsProxy>    mEdgefltParams;
    SmartPtr<RkAiqIspFecParamsProxy>        mFecParams;
    SmartPtr<RkAiqIspOrbParamsProxy>        mOrbParams;

    // TODO: change full params to list
    // V21 differential modules
    SmartPtr<RkAiqIspDrcParamsProxy>        mDrcParams;
    SmartPtr<RkAiqIspBaynrParamsProxy>   mBaynrParams;
    //SmartPtr<RkAiqIspBa3dParamsProxyV21>    mBa3dParams;
    SmartPtr<RkAiqIspCnrParamsProxy>     mCnrParams;
#if RKAIQ_HAVE_CAC
    SmartPtr<RkAiqIspCacParamsProxy>     mCacParams;
#endif
    SmartPtr<RkAiqIspAfdParamsProxy>         mAfdParams;

    // V39 differential modules
#if RKAIQ_HAVE_YUVME
    SmartPtr<RkAiqIspYuvmeParamsProxy>    mYuvmeParams;
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    SmartPtr<RkAiqIspRgbirParamsProxy>       mRgbirParams;
#endif
    // new struct
#if USE_NEWSTRUCT
    SmartPtr<RkAiqIspDmParamsProxy>  mDmParams;
    SmartPtr<RkAiqIspBtnrParamsProxy>  mBtnrParams;
    SmartPtr<RkAiqIspGammaParamsProxy>  mGammaParams;
    SmartPtr<RkAiqIspSharpParamsProxy>  mSharpParams;
#endif

 private:
    XCAM_DEAD_COPY (RkAiqFullParams);
};

typedef SharedItemPool<RkAiqFullParams> RkAiqFullParamsPool;
typedef SharedItemProxy<RkAiqFullParams> RkAiqFullParamsProxy;

template<class T>
struct AlgoRstShared: public XCam::VideoBuffer {
public:
    AlgoRstShared () : VideoBuffer() { }
    typedef T value_type;
    T   result;
    virtual uint8_t* map() override {
        return (uint8_t*)(&result);
    }
    virtual bool unmap () override { return false; }
    virtual int get_fd () override { return -1; }
};

using RkAiqAlgoPreResAeIntShared = AlgoRstShared<RkAiqAlgoPreResAe>;
//using RkAiqAlgoProcResAeIntShared = AlgoRstShared<RkAiqAlgoProcResAe>;
using RkAiqAlgoProcResAwbIntShared = AlgoRstShared<RkAiqAlgoProcResAwbShared_t>;
using RkAiqAlgoProcResAmdIntShared = AlgoRstShared<RkAiqAlgoProcResAmd>;
using RkAiqAlgoProcResAblcIntShared = AlgoRstShared<RkAiqAlgoProcResAblc>;
using RkAiqAlgoProcResAblcV32IntShared = AlgoRstShared<RkAiqAlgoProcResAblcV32>;
using RkAiqAlgoProcResAynrV3IntShared  = AlgoRstShared<RkAiqAlgoProcResAynrV3>;
using RkAiqAlgoProcResAynrV22IntShared = AlgoRstShared<RkAiqAlgoProcResAynrV22>;
using RkAiqAlgoProcResAynrV24IntShared = AlgoRstShared<RkAiqAlgoProcResAynrV24>;
using RkAiqAlgoProcResYnrIntShared = AlgoRstShared<RkAiqAlgoProcResYnr>;
using RkAiqAlgoAtnrV30Stats            = AlgoRstShared<Abayertnr_Stats_V30_t>;

typedef SharedItemPool<AecProcResult_t> RkAiqAeProcResultPool;
typedef SharedItemProxy<AecProcResult_t> RkAiqAeProcResultProxy;

typedef SharedItemPool<rkisp_effect_params> RkAiqIspEffParamsPool;
typedef SharedItemProxy<rkisp_effect_params> RkAiqIspEffParamsProxy;

}

#endif //RK_AIQ_POOL_H
