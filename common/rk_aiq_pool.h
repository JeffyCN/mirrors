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
#ifndef RK_AIQ_POOL_H
#define RK_AIQ_POOL_H

#include <map>

#include "rk_aiq_types_priv.h"
#include "shared_item_pool.h"

namespace RkCam {

typedef struct RKAiqAecExpInfoWrapper_s : public SharedItemData {
    RKAiqAecExpInfo_t aecExpInfo;
    RKAiqAecExpInfo_t exp_tbl[MAX_AEC_EFFECT_FNUM + 1];
    Sensor_dpcc_res_t SensorDpccInfo;
    int exp_tbl_size;
    int algo_id;
    struct RKAiqAecExpInfoWrapper_s& operator=(const struct RKAiqAecExpInfoWrapper_s& set)
    {
       this->aecExpInfo = set.aecExpInfo;
       this->SensorDpccInfo = set.SensorDpccInfo;
       this->exp_tbl_size = set.exp_tbl_size;
       this->algo_id = set.algo_id;
       memcpy(this->exp_tbl, set.exp_tbl, sizeof(set.exp_tbl));
       return *this;
    }
} RKAiqAecExpInfoWrapper_t;

typedef struct RKAiqAfInfoWrapper_s : public SharedItemData {
    struct timeval focusStartTim;
    struct timeval focusEndTim;
    struct timeval zoomStartTim;
    struct timeval zoomEndTim;
    int64_t sofTime;
    int32_t focusCode;
    int32_t zoomCode;
} RKAiqAfInfoWrapper_t;

typedef struct RkAiqPirisInfoWrapper_s : public SharedItemData {
    int             step;
    int             laststep;
    bool            update;
    struct timeval  StartTim;
    struct timeval  EndTim;
} RkAiqPirisInfoWrapper_t;

typedef struct RkAiqIrisInfoWrapper_s : public SharedItemData {
    //TODO Merge
	//RkAiqIrisType_t           IrisType;
    RkAiqPirisInfoWrapper_t   PIris;
    RkAiqDCIrisParam_t        DCIris;
    uint64_t                  sofTime;
} RkAiqIrisInfoWrapper_t;

typedef struct RKAiqCpslInfoWrapper_s : public SharedItemData {
    rk_aiq_flash_setting_t fl;
    bool update_fl;
    rk_aiq_ir_setting_t ir;
    rk_aiq_flash_setting_t fl_ir;
    bool update_ir;
} RKAiqCpslInfoWrapper_t;

typedef enum RkAiqParamsType_e {
    RK_AIQ_PARAMS_ALL,
    RK_AIQ_PARAMS_MEAS,
    RK_AIQ_PARAMS_OTHER,
} RkAiqParamsType_t;

typedef enum _cam3aResultType {
	RESULT_TYPE_INVALID = -1,
	RESULT_TYPE_EXPOSURE,
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
	RESULT_TYPE_MAX_PARAM,
} cam3aResultType;

static const char* Cam3aResultType2Str[RESULT_TYPE_MAX_PARAM] = {
    [RESULT_TYPE_EXPOSURE]           = "EXPOSURE",
    [RESULT_TYPE_AEC_PARAM]          = "AEC",
    [RESULT_TYPE_HIST_PARAM]         = "HIST",
    [RESULT_TYPE_AWB_PARAM]          = "AWB",
    [RESULT_TYPE_AWBGAIN_PARAM]      = "AWB_GAIN",
    [RESULT_TYPE_AF_PARAM]           = "AF",
    [RESULT_TYPE_DPCC_PARAM]         = "DPCC",
    [RESULT_TYPE_MERGE_PARAM]        = "MERGE",
    [RESULT_TYPE_TMO_PARAM]          = "TMO",
    [RESULT_TYPE_CCM_PARAM]          = "CCM",
    [RESULT_TYPE_LSC_PARAM]          = "LSC",
    [RESULT_TYPE_BLC_PARAM]          = "BLC",
    [RESULT_TYPE_RAWNR_PARAM]        = "RAWNR",
    [RESULT_TYPE_GIC_PARAM]          = "GIC",
    [RESULT_TYPE_DEBAYER_PARAM]      = "DEBAYER",
    [RESULT_TYPE_LDCH_PARAM]         = "LDCH",
    [RESULT_TYPE_LUT3D_PARAM]        = "LUT3D",
    [RESULT_TYPE_DEHAZE_PARAM]       = "DEHAZE",
    [RESULT_TYPE_AGAMMA_PARAM]       = "AGAMMA",
    [RESULT_TYPE_ADEGAMMA_PARAM]     = "ADEGAMMA",
    [RESULT_TYPE_WDR_PARAM]          = "WDR",
    [RESULT_TYPE_CSM_PARAM]          = "CSM",
    [RESULT_TYPE_CGC_PARAM]          = "CGC",
    [RESULT_TYPE_CONV422_PARAM]      = "CONV22",
    [RESULT_TYPE_YUVCONV_PARAM]      = "YUVCONV",
    [RESULT_TYPE_GAIN_PARAM]         = "GAIN",
    [RESULT_TYPE_CP_PARAM]           = "CP",
    [RESULT_TYPE_IE_PARAM]           = "IE",
    [RESULT_TYPE_MOTION_PARAM]       = "MOTION",
    [RESULT_TYPE_IRIS_PARAM]         = "IRIS",
    [RESULT_TYPE_CPSL_PARAM]         = "CPSL",
    [RESULT_TYPE_FLASH_PARAM]        = "FLASH",
    [RESULT_TYPE_TNR_PARAM]          = "TNR",
    [RESULT_TYPE_YNR_PARAM]          = "YNR",
    [RESULT_TYPE_UVNR_PARAM]         = "UVNR",
    [RESULT_TYPE_SHARPEN_PARAM]      = "SHARPEN",
    [RESULT_TYPE_EDGEFLT_PARAM]      = "EDGEFLT",
    [RESULT_TYPE_FEC_PARAM]          = "FEC",
    [RESULT_TYPE_ORB_PARAM]          = "ORB",
    [RESULT_TYPE_FOCUS_PARAM]        = "FOCUS",
    [RESULT_TYPE_DRC_PARAM]          = "DRC",
};

typedef SharedItemBase cam3aResult;
typedef std::list<SmartPtr< cam3aResult>> cam3aResultList;
typedef std::map<int, std::list<SmartPtr<cam3aResult>>> cam3aResultMap;

typedef RKAiqAecExpInfoWrapper_t rk_aiq_exposure_params_wrapper_t;
typedef RKAiqAfInfoWrapper_t rk_aiq_af_info_wrapper_t;
typedef RkAiqIrisInfoWrapper_t rk_aiq_iris_params_wrapper_t;

typedef SharedItemPool<rk_aiq_exposure_params_wrapper_t> RkAiqExpParamsPool;
typedef SharedItemProxy<rk_aiq_exposure_params_wrapper_t> RkAiqExpParamsProxy;
typedef SharedItemPool<rk_aiq_iris_params_wrapper_t> RkAiqIrisParamsPool;
typedef SharedItemProxy<rk_aiq_iris_params_wrapper_t> RkAiqIrisParamsProxy;
typedef SharedItemPool<rk_aiq_isp_params_base_t> RkAiqIspBasePool;
typedef SharedItemProxy<rk_aiq_isp_params_base_t> RkAiqIspBaseProxy;

typedef SharedItemPool<rk_aiq_af_info_wrapper_t> RkAiqAfInfoPool;
typedef SharedItemProxy<rk_aiq_af_info_wrapper_t> RkAiqAfInfoProxy;
typedef SharedItemPool<rk_aiq_focus_params_wrapper_t> RkAiqFocusParamsPool;
typedef SharedItemProxy<rk_aiq_focus_params_wrapper_t> RkAiqFocusParamsProxy;
typedef SharedItemPool<RKAiqCpslInfoWrapper_t> RkAiqCpslParamsPool;
typedef SharedItemProxy<RKAiqCpslInfoWrapper_t> RkAiqCpslParamsProxy;

typedef SharedItemPool<rk_aiq_isp_aec_params_v20_t>         RkAiqIspAecParamsPool;
typedef SharedItemProxy<rk_aiq_isp_aec_params_v20_t>        RkAiqIspAecParamsProxy;
typedef SharedItemPool<rk_aiq_isp_hist_params_v20_t>        RkAiqIspHistParamsPool;
typedef SharedItemProxy<rk_aiq_isp_hist_params_v20_t>       RkAiqIspHistParamsProxy;
typedef SharedItemPool<rk_aiq_isp_awb_params_v20_t>         RkAiqIspAwbParamsPool;
typedef SharedItemProxy<rk_aiq_isp_awb_params_v20_t>        RkAiqIspAwbParamsProxy;
typedef SharedItemPool<rk_aiq_isp_awb_gain_params_v20_t>    RkAiqIspAwbGainParamsPool;
typedef SharedItemProxy<rk_aiq_isp_awb_gain_params_v20_t>   RkAiqIspAwbGainParamsProxy;
typedef SharedItemPool<rk_aiq_isp_af_params_v20_t>          RkAiqIspAfParamsPool;
typedef SharedItemProxy<rk_aiq_isp_af_params_v20_t>         RkAiqIspAfParamsProxy;
typedef SharedItemPool<rk_aiq_isp_dpcc_params_v20_t>        RkAiqIspDpccParamsPool;
typedef SharedItemProxy<rk_aiq_isp_dpcc_params_v20_t>       RkAiqIspDpccParamsProxy;
typedef SharedItemPool<rk_aiq_isp_merge_params_v20_t>       RkAiqIspMergeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_merge_params_v20_t>      RkAiqIspMergeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_tmo_params_v20_t>         RkAiqIspTmoParamsPool;
typedef SharedItemProxy<rk_aiq_isp_tmo_params_v20_t>        RkAiqIspTmoParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ccm_params_v20_t>         RkAiqIspCcmParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ccm_params_v20_t>        RkAiqIspCcmParamsProxy;
typedef SharedItemPool<rk_aiq_isp_lsc_params_v20_t>         RkAiqIspLscParamsPool;
typedef SharedItemProxy<rk_aiq_isp_lsc_params_v20_t>        RkAiqIspLscParamsProxy;
typedef SharedItemPool<rk_aiq_isp_blc_params_v20_t>         RkAiqIspBlcParamsPool;
typedef SharedItemProxy<rk_aiq_isp_blc_params_v20_t>        RkAiqIspBlcParamsProxy;
typedef SharedItemPool<rk_aiq_isp_rawnr_params_v20_t>       RkAiqIspRawnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_rawnr_params_v20_t>      RkAiqIspRawnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_gic_params_v20_t>         RkAiqIspGicParamsPool;
typedef SharedItemProxy<rk_aiq_isp_gic_params_v20_t>        RkAiqIspGicParamsProxy;
typedef SharedItemPool<rk_aiq_isp_debayer_params_v20_t>     RkAiqIspDebayerParamsPool;
typedef SharedItemProxy<rk_aiq_isp_debayer_params_v20_t>    RkAiqIspDebayerParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ldch_params_v20_t>        RkAiqIspLdchParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ldch_params_v20_t>       RkAiqIspLdchParamsProxy;
typedef SharedItemPool<rk_aiq_isp_lut3d_params_v20_t>       RkAiqIspLut3dParamsPool;
typedef SharedItemProxy<rk_aiq_isp_lut3d_params_v20_t>      RkAiqIspLut3dParamsProxy;
typedef SharedItemPool<rk_aiq_isp_dehaze_params_v20_t>      RkAiqIspDehazeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_dehaze_params_v20_t>     RkAiqIspDehazeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_agamma_params_v20_t>      RkAiqIspAgammaParamsPool;
typedef SharedItemProxy<rk_aiq_isp_agamma_params_v20_t>     RkAiqIspAgammaParamsProxy;
typedef SharedItemPool<rk_aiq_isp_adegamma_params_v20_t>    RkAiqIspAdegammaParamsPool;
typedef SharedItemProxy<rk_aiq_isp_adegamma_params_v20_t>   RkAiqIspAdegammaParamsProxy;
typedef SharedItemPool<rk_aiq_isp_wdr_params_v20_t>         RkAiqIspWdrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_wdr_params_v20_t>        RkAiqIspWdrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_csm_params_v20_t>         RkAiqIspCsmParamsPool;
typedef SharedItemProxy<rk_aiq_isp_csm_params_v20_t>        RkAiqIspCsmParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cgc_params_v20_t>         RkAiqIspCgcParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cgc_params_v20_t>        RkAiqIspCgcParamsProxy;
typedef SharedItemPool<rk_aiq_isp_conv422_params_v20_t>     RkAiqIspConv422ParamsPool;
typedef SharedItemProxy<rk_aiq_isp_conv422_params_v20_t>    RkAiqIspConv422ParamsProxy;
typedef SharedItemPool<rk_aiq_isp_yuvconv_params_v20_t>     RkAiqIspYuvconvParamsPool;
typedef SharedItemProxy<rk_aiq_isp_yuvconv_params_v20_t>    RkAiqIspYuvconvParamsProxy;
typedef SharedItemPool<rk_aiq_isp_gain_params_v20_t>        RkAiqIspGainParamsPool;
typedef SharedItemProxy<rk_aiq_isp_gain_params_v20_t>       RkAiqIspGainParamsProxy;
typedef SharedItemPool<rk_aiq_isp_cp_params_v20_t>          RkAiqIspCpParamsPool;
typedef SharedItemProxy<rk_aiq_isp_cp_params_v20_t>         RkAiqIspCpParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ie_params_v20_t>          RkAiqIspIeParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ie_params_v20_t>         RkAiqIspIeParamsProxy;
typedef SharedItemPool<rk_aiq_isp_motion_params_v20_t>      RkAiqIspMotionParamsPool;
typedef SharedItemProxy<rk_aiq_isp_motion_params_v20_t>     RkAiqIspMotionParamsProxy;
typedef SharedItemPool<rk_aiq_isp_md_params_v20_t>          RkAiqIspMdParamsPool;
typedef SharedItemProxy<rk_aiq_isp_md_params_v20_t>         RkAiqIspMdParamsProxy;

typedef SharedItemPool<rk_aiq_isp_tnr_params_v20_t>         RkAiqIspTnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_tnr_params_v20_t>        RkAiqIspTnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_ynr_params_v20_t>         RkAiqIspYnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_ynr_params_v20_t>        RkAiqIspYnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_uvnr_params_v20_t>        RkAiqIspUvnrParamsPool;
typedef SharedItemProxy<rk_aiq_isp_uvnr_params_v20_t>       RkAiqIspUvnrParamsProxy;
typedef SharedItemPool<rk_aiq_isp_sharpen_params_v20_t>     RkAiqIspSharpenParamsPool;
typedef SharedItemProxy<rk_aiq_isp_sharpen_params_v20_t>    RkAiqIspSharpenParamsProxy;
typedef SharedItemPool<rk_aiq_isp_edgeflt_params_v20_t>     RkAiqIspEdgefltParamsPool;
typedef SharedItemProxy<rk_aiq_isp_edgeflt_params_v20_t>    RkAiqIspEdgefltParamsProxy;
typedef SharedItemPool<rk_aiq_isp_fec_params_v20_t>         RkAiqIspFecParamsPool;
typedef SharedItemProxy<rk_aiq_isp_fec_params_v20_t>        RkAiqIspFecParamsProxy;
typedef SharedItemPool<rk_aiq_isp_orb_params_v20_t>         RkAiqIspOrbParamsPool;
typedef SharedItemProxy<rk_aiq_isp_orb_params_v20_t>        RkAiqIspOrbParamsProxy;

//v21 pools
typedef SharedItemPool<rk_aiq_isp_awb_params_v21_t>         RkAiqIspAwbParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_awb_params_v21_t>        RkAiqIspAwbParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_drc_params_v21_t>         RkAiqIspDrcParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_drc_params_v21_t>        RkAiqIspDrcParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_blc_params_v21_t>         RkAiqIspBlcParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_blc_params_v21_t>        RkAiqIspBlcParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_gic_params_v21_t>         RkAiqIspGicParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_gic_params_v21_t>        RkAiqIspGicParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_dehaze_params_v21_t>      RkAiqIspDehazeParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_dehaze_params_v21_t>     RkAiqIspDehazeParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_baynr_params_v21_t>       RkAiqIspBaynrParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_baynr_params_v21_t>      RkAiqIspBaynrParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_bay3d_params_v21_t>       RkAiqIspBa3dParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_bay3d_params_v21_t>      RkAiqIspBa3dParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_ynr_params_v21_t>         RkAiqIspYnrParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_ynr_params_v21_t>        RkAiqIspYnrParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_cnr_params_v21_t>         RkAiqIspCnrParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_cnr_params_v21_t>        RkAiqIspCnrParamsProxyV21;
typedef SharedItemPool<rk_aiq_isp_sharpen_params_v21_t>     RkAiqIspSharpenParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_sharpen_params_v21_t>    RkAiqIspSharpenParamsProxyV21;

typedef class rk_aiq_isp_params_full_v20_s : public SharedItemData {
public:
    SmartPtr<RkAiqIspAecParamsProxy>                        ae;
    SmartPtr<RkAiqIspHistParamsProxy>                       hist;
    SmartPtr<RkAiqIspAwbParamsProxy>                        wb;
    SmartPtr<RkAiqIspAwbGainParamsProxy>                    wb_gain;
    SmartPtr<RkAiqIspAfParamsProxy>                         af;
    SmartPtr<RkAiqIspDpccParamsProxy>                       dpcc;
    SmartPtr<RkAiqIspMergeParamsProxy>                      merge;
    SmartPtr<RkAiqIspTmoParamsProxy>                        tmo;
    SmartPtr<RkAiqIspCcmParamsProxy>                        ccm;
    SmartPtr<RkAiqIspLscParamsProxy>                        lsc;
    SmartPtr<RkAiqIspBlcParamsProxy>                        blc;
    SmartPtr<RkAiqIspRawnrParamsProxy>                      rawnr;
    SmartPtr<RkAiqIspGicParamsProxy>                        gic;
    SmartPtr<RkAiqIspDebayerParamsProxy>                    debayer;
    SmartPtr<RkAiqIspLdchParamsProxy>                       ldch;
    SmartPtr<RkAiqIspLut3dParamsProxy>                      lut3d;
    SmartPtr<RkAiqIspDehazeParamsProxy>                     dehaze;
    SmartPtr<RkAiqIspAgammaParamsProxy>                     gamma;
    SmartPtr<RkAiqIspAdegammaParamsProxy>                   degamma;
    SmartPtr<RkAiqIspWdrParamsProxy>                        wdr;
    SmartPtr<RkAiqIspCsmParamsProxy>                        csm;
    SmartPtr<RkAiqIspCgcParamsProxy>                        cgc;
    SmartPtr<RkAiqIspConv422ParamsProxy>                    conv22;
    SmartPtr<RkAiqIspYuvconvParamsProxy>                    yuvcon;
    SmartPtr<RkAiqIspGainParamsProxy>                       gain;
    SmartPtr<RkAiqIspCpParamsProxy>                         cp;
    SmartPtr<RkAiqIspIeParamsProxy>                         ie;
    SmartPtr<RkAiqIspMotionParamsProxy>                     motion;
    SmartPtr<RkAiqIspMdParamsProxy>                         md;
    SmartPtr<RkAiqIspTnrParamsProxy>                        tnr;
    SmartPtr<RkAiqIspYnrParamsProxy>                        ynr;
    SmartPtr<RkAiqIspUvnrParamsProxy>                       uvnr;
    SmartPtr<RkAiqIspSharpenParamsProxy>                    sharpen;
    SmartPtr<RkAiqIspEdgefltParamsProxy>                    edgeflt;
    SmartPtr<RkAiqIspFecParamsProxy>                        fec;
    SmartPtr<RkAiqIspOrbParamsProxy>                        orb;
    void reset() {
        ae.release();
        hist.release();
        wb.release();
        wb_gain.release();
        af.release();
        dpcc.release();
        merge.release();
        tmo.release();
        ccm.release();
        lsc.release();
        blc.release();
        rawnr.release();
        gic.release();
        debayer.release();
        ldch.release();
        lut3d.release();
        dehaze.release();
        gamma.release();
        degamma.release();
        wdr.release();
        csm.release();
        cgc.release();
        conv22.release();
        yuvcon.release();
        gain.release();
        cp.release();
        ie.release();
        motion.release();
        md.release();
        tnr.release();
        ynr.release();
        uvnr.release();
        sharpen.release();
        edgeflt.release();
        fec.release();
        orb.release();
    };
} rk_aiq_isp_params_full_v20_t;

// v21 params struct
typedef struct : public SharedItemData {
    SmartPtr<RkAiqIspAecParamsProxy>                        ae;
    SmartPtr<RkAiqIspHistParamsProxy>                       hist;
    SmartPtr<RkAiqIspAwbParamsProxyV21>                     wb;
    SmartPtr<RkAiqIspAwbGainParamsProxy>                    wb_gain;
    SmartPtr<RkAiqIspAfParamsProxy>                         af;
    SmartPtr<RkAiqIspDpccParamsProxy>                       dpcc;
    SmartPtr<RkAiqIspMergeParamsProxy>                      merge;
    SmartPtr<RkAiqIspDrcParamsProxyV21>                     drc;
    SmartPtr<RkAiqIspCcmParamsProxy>                        ccm;
    SmartPtr<RkAiqIspLscParamsProxy>                        lsc;
    SmartPtr<RkAiqIspBlcParamsProxyV21>                     blc;
    SmartPtr<RkAiqIspRawnrParamsProxy>                      rawnr;
    SmartPtr<RkAiqIspGicParamsProxyV21>                     gic;
    SmartPtr<RkAiqIspDebayerParamsProxy>                    debayer;
    SmartPtr<RkAiqIspLdchParamsProxy>                       ldch;
    SmartPtr<RkAiqIspLut3dParamsProxy>                      lut3d;
    SmartPtr<RkAiqIspDehazeParamsProxyV21>                  dehaze;
    SmartPtr<RkAiqIspAgammaParamsProxy>                     gamma;
    SmartPtr<RkAiqIspAdegammaParamsProxy>                   degamma;
    SmartPtr<RkAiqIspWdrParamsProxy>                        wdr;
    SmartPtr<RkAiqIspCsmParamsProxy>                        csm;
    SmartPtr<RkAiqIspCgcParamsProxy>                        cgc;
    SmartPtr<RkAiqIspConv422ParamsProxy>                    conv22;
    SmartPtr<RkAiqIspYuvconvParamsProxy>                    yuvcon;
    SmartPtr<RkAiqIspGainParamsProxy>                       gain;
    SmartPtr<RkAiqIspCpParamsProxy>                         cp;
    SmartPtr<RkAiqIspIeParamsProxy>                         ie;
    SmartPtr<RkAiqIspMotionParamsProxy>                     motion;
    SmartPtr<RkAiqIspMdParamsProxy>                         md;
    SmartPtr<RkAiqIspBaynrParamsProxyV21>                   baynr;
    SmartPtr<RkAiqIspBa3dParamsProxyV21>                    ba3d;
    SmartPtr<RkAiqIspYnrParamsProxyV21>                     ynr;
    SmartPtr<RkAiqIspCnrParamsProxyV21>                     cnr;
    SmartPtr<RkAiqIspSharpenParamsProxyV21>                   sharp;
    void reset () {
        ae.release();
        hist.release();
        wb.release();
        wb_gain.release();
        af.release();
        dpcc.release();
        merge.release();
        drc.release();
        ccm.release();
        lsc.release();
        blc.release();
        rawnr.release();
        gic.release();
        debayer.release();
        ldch.release();
        lut3d.release();
        dehaze.release();
        gamma.release();
        degamma.release();
        wdr.release();
        csm.release();
        cgc.release();
        conv22.release();
        yuvcon.release();
        gain.release();
        cp.release();
        ie.release();
        motion.release();
        md.release();
        baynr.release();
        ba3d.release();
        ynr.release();
        cnr.release();
        sharp.release();
    }
} rk_aiq_isp_params_full_v21_t;

typedef SharedItemPool<rk_aiq_isp_params_full_v20_t>        RkAiqIspParamsPoolV20;
typedef SharedItemProxy<rk_aiq_isp_params_full_v20_t>       RkAiqIspParamsProxyV20;
typedef SharedItemPool<rk_aiq_isp_params_full_v21_t>        RkAiqIspParamsPoolV21;
typedef SharedItemProxy<rk_aiq_isp_params_full_v21_t>       RkAiqIspParamsProxyV21;


class RkAiqFullParams : public SharedItemData {
public:
    explicit RkAiqFullParams()
        : mExposureParams(NULL)
        , mFocusParams(NULL)
        , mIrisParams(NULL)
        , mCpslParams(NULL)

        , mAecParams(NULL)
        , mHistParams(NULL)
        , mAwbParams(NULL)
        , mAwbGainParams(NULL)
        , mAfParams(NULL)
        , mDpccParams(NULL)
        , mMergeParams(NULL)
        , mTmoParams(NULL)
        , mCcmParams(NULL)
        , mLscParams(NULL)
        , mBlcParams(NULL)
        , mRawnrParams(NULL)
        , mGicParams(NULL)
        , mDebayerParams(NULL)
        , mLdchParams(NULL)
        , mLut3dParams(NULL)
        , mDehazeParams(NULL)
        , mAgammaParams(NULL)
        , mWdrParams(NULL)
        , mCsmParams(NULL)
        , mCgcParams(NULL)
        , mConv422Params(NULL)
        , mYuvconvParams(NULL)
        , mGainParams(NULL)
        , mCpParams(NULL)
        , mIeParams(NULL)
        , mMotionParams(NULL)
        , mMdParams(NULL)

        , mTnrParams(NULL)
        , mYnrParams(NULL)
        , mUvnrParams(NULL)
        , mSharpenParams(NULL)
        , mEdgefltParams(NULL)
        , mFecParams(NULL)
        , mOrbParams(NULL)
        // TODO: change full params to list
        // V21 differential modules
        , mAwbV21Params(NULL)
        , mDrcV21Params(NULL)
        , mBlcV21Params(NULL)
        , mGicV21Params(NULL)
        , mDehazeV21Params(NULL)
        , mBaynrV21Params(NULL)
        // , mBa3dParams(NULL)
        , mYnrV21Params(NULL)
        , mCnrV21Params(NULL)
        , mSharpenV21Params(NULL) {
    };
    ~RkAiqFullParams() {};

    void reset() {
        mExposureParams.release();
        mFocusParams.release();
        mIrisParams.release();
        mCpslParams.release();

        mAecParams.release();
        mHistParams.release();
        mAwbParams.release();
        mAwbGainParams.release();
        mAfParams.release();
        mDpccParams.release();
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
        mAwbV21Params.release();
        mDrcV21Params.release();
        mBlcV21Params.release();
        mGicV21Params.release();
        mDehazeV21Params.release();
        mBaynrV21Params.release();
        // mBa3dParams.release();
        mYnrV21Params.release();
        mCnrV21Params.release();
        mSharpenV21Params.release();
    };
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
    SmartPtr<RkAiqIspAwbParamsProxyV21>     mAwbV21Params;
    SmartPtr<RkAiqIspDrcParamsProxyV21>     mDrcV21Params;
    SmartPtr<RkAiqIspBlcParamsProxyV21>     mBlcV21Params;
    SmartPtr<RkAiqIspGicParamsProxyV21>     mGicV21Params;
    SmartPtr<RkAiqIspDehazeParamsProxyV21>  mDehazeV21Params;
    SmartPtr<RkAiqIspBaynrParamsProxyV21>   mBaynrV21Params;
    //SmartPtr<RkAiqIspBa3dParamsProxyV21>    mBa3dParams;
    SmartPtr<RkAiqIspYnrParamsProxyV21>     mYnrV21Params;
    SmartPtr<RkAiqIspCnrParamsProxyV21>     mCnrV21Params;
    SmartPtr<RkAiqIspSharpenParamsProxyV21> mSharpenV21Params;
private:
    XCAM_DEAD_COPY (RkAiqFullParams);
};

typedef SharedItemPool<RkAiqFullParams> RkAiqFullParamsPool;
typedef SharedItemProxy<RkAiqFullParams> RkAiqFullParamsProxy;

};

#endif //RK_AIQ_POOL_H
