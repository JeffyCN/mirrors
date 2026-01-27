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
#ifndef _AIQ_ISP_PARAMS_CVT_H_
#define _AIQ_ISP_PARAMS_CVT_H_

#include <stdbool.h>
#include <stdint.h>

#include "c_base/aiq_list.h"
#include "c_base/aiq_mutex.h"
#include "common/rk_aiq_types_priv_c.h"
#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/info/include/st_string.h"
#endif
#include "include/xcore/base/xcam_defs.h"
#include "modules/rk_aiq_isp32_modules.h"
#if (defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)) && (USE_NEWSTRUCT)
#include "modules/rk_aiq_isp39_modules.h"
#endif

XCAM_BEGIN_DECLARE

#define ISP20PARAM_SUBM (0x2)

typedef struct AiqIspParamsCvt_s AiqIspParamsCvt_t;

typedef struct AiqIspDrvParams_info_s {
    bool bEn;
    void* pLatestCfgaddr;
} AiqIspDrvParams_info_t;

typedef struct AntiTmoFlicker_s {
    int preFrameNum;
    bool FirstChange;
    int FirstChangeNum;
    bool FirstChangeDone;
    int FirstChangeDoneNum;
} AntiTmoFlicker_t;

struct IspParamsCvt_Ops {
    XCamReturn (*ConvertAeParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                  const rk_aiq_isp_aec_params_t* aec_meas);
    XCamReturn (*ConvertHistParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_hist_params_t* hist_meas);
    XCamReturn (*ConvertAwbParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                   const rk_aiq_isp_awb_params_t* awb_meas);
    XCamReturn (*ConvertAwbGainParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                       const rk_aiq_isp_awb_gain_params_t* awb_gain,
                                       const rk_aiq_isp_blc_t* blc);
    XCamReturn (*ConvertMergeParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                     const rk_aiq_isp_merge_params_t* amerge_data);
#if RKAIQ_HAVE_DEHAZE_V14
    XCamReturn (*ConvertAdehazeParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                       const rk_aiq_isp_dehaze_params_t* dhaze);
#endif
    XCamReturn (*ConvertAgammaParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                      const rk_aiq_isp_gamma_params_t* gamma_out_cfg);
    XCamReturn (*ConvertAdegammagParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                         const rk_aiq_isp_adegamma_params_t* degamma_cfg);
    XCamReturn (*ConvertAdemosaicgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                          const rk_aiq_isp_dm_params_t* demosaic);
    XCamReturn (*ConvertLscgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_lsc_params_t* lsc);
    XCamReturn (*ConvertBlcToIsp20Params)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                          rk_aiq_isp_blc_params_t* blc);
    XCamReturn (*ConvertDpccgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                     rk_aiq_isp_dpcc_params_t* dpcc);
    XCamReturn (*ConvertCcmgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_ccm_params_t* ccm);
#if RKAIQ_HAVE_3DLUT
    XCamReturn (*ConvertA3dlutgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                       const rk_aiq_isp_lut3d_params_t* lut3d_cfg);
#endif
    XCamReturn (*ConvertCpgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                   const rk_aiq_isp_cp_params_t* lut3d_cfg);
    XCamReturn (*ConvertIegParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                   const rk_aiq_isp_ie_params_t* ie_cfg);
    XCamReturn (*ConvertYnrgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_ynr_params_t* ynr);
    XCamReturn (*ConvertCnrgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_cnr_params_t* cnr);
    XCamReturn (*ConvertBaynrgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                      rk_aiq_isp_baynr_params_t* baynr);
    XCamReturn (*ConvertBay3dgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                      rk_aiq_isp_btnr_params_t* bay3d);
    XCamReturn (*ConvertSharpengParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                        rk_aiq_isp_sharp_params_t* sharp);
    XCamReturn (*ConvertAfgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                   const rk_aiq_isp_af_params_t* af_data);
    XCamReturn (*ConvertGaingParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                     rk_aiq_isp_gain_params_t* gain);
    XCamReturn (*ConvertGicgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_gic_params_t* gic_cfg);
    XCamReturn (*ConvertCacgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_cac_params_t* cac);
    XCamReturn (*ConvertDrcgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                    const rk_aiq_isp_drc_params_t* drc);
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    XCamReturn (*ConvertRgbirgParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                      const rk_aiq_isp_rgbir_params_t* rgbir);
#endif
#if RKAIQ_HAVE_YUVME
    XCamReturn(*ConvertYuvmegParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
                                     const rk_aiq_isp_yme_params_t* yme);
#endif
    XCamReturn (*Merge_results)(AiqIspParamsCvt_t* pCvt, AiqList_t* results, void* isp_cfg);
    bool (*Convert3aResultsToIspCfg)(AiqIspParamsCvt_t* pCvt, aiq_params_base_t* result,
                                     void* isp_cfg_p, bool is_multi_isp);
    XCamReturn (*CheckIspParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg);
    XCamReturn (*FixedAwbOveflowToIsp3xParams)(AiqIspParamsCvt_t* pCvt, void* isp_cfg,
            bool multiIspMode);

    void (*update)(void* src, void* dst);
    void (*Dump)(AiqIspParamsCvt_t* pCvt, uint64_t modules, int fd);
};

/**
 * @brief - Convert the params to hardware params for isp modules
 *
 * @mCvtedIsp33Prams: the latest converted params.
 */
struct AiqIspParamsCvt_s {
    int32_t _CamPhyId;
    uint32_t sensor_output_width;
    uint32_t sensor_output_height;
    uint32_t _last_pp_module_init_ens;
    uint64_t _force_isp_module_ens;
    uint64_t _force_module_flags;
    int _working_mode;
    AntiTmoFlicker_t AntiTmoFlicker;
    AiqMutex_t _mutex;
    aiq_params_base_t* mBlcResult;
    aiq_params_base_t* mAeParams;
    aiq_params_base_t* mAwbParams;
    aiq_params_base_t* mAfParams;
    aiq_params_base_t* mAibnrParams;
    aiq_params_base_t* mAiynrParams;
    bool _lsc_en;
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    struct isp32_isp_meas_cfg mLatestMeasCfg;
    struct isp32_bls_cfg mLatestBlsCfg;
    struct isp32_awb_gain_cfg mLatestWbGainCfg;
#elif defined(ISP_HW_V39)
    struct isp39_isp_meas_cfg mLatestMeasCfg;
    struct isp32_bls_cfg mLatestBlsCfg;
    struct isp32_awb_gain_cfg mLatestWbGainCfg;
    rk_aiq_wb_gain_v32_t awb_gain_final;
    struct isp3x_gain_cfg mLatestGainCfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_meas_cfg mLatestMeasCfg;
    struct isp32_bls_cfg mLatestBlsCfg;
    struct isp32_awb_gain_cfg mLatestWbGainCfg;
    rk_aiq_wb_gain_v32_t awb_gain_final;
    struct isp3x_gain_cfg mLatestGainCfg;
#elif defined(ISP_HW_V35)
    struct isp35_isp_meas_cfg mLatestMeasCfg;
    struct isp35_bls_cfg mLatestBlsCfg;
    struct isp32_awb_gain_cfg mLatestWbGainCfg;
    rk_aiq_wb_gain_v32_t awb_gain_final;
    struct isp3x_gain_cfg mLatestGainCfg;
#endif
#if defined(ISP_HW_V30) || defined(ISP_HW_V21)
    struct isp21_awb_gain_cfg mLatestWbGainCfg;
#endif
#if (defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)) && (USE_NEWSTRUCT)
    btnr_cvt_info_t mBtnrInfo;
    void *btnr_attrib;
    void *btnr_cis_info;
#endif
    float mLatestIspDgain;
#if defined(ISP_HW_V39) && (USE_NEWSTRUCT)
    cac_cvt_info_t mCacInfo;
#endif
    common_cvt_info_t mCommonCvtInfo;
    blc_cvt_info_t mBlcInfo;
    isp_params_t isp_params;
    AiqIspDrvParams_info_t mLatestCfgArray[ISP2X_ID_MAX];
#if (defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)) && (USE_NEWSTRUCT)
    mergeLuma2Wgt_t mergeLuma2Wgt;
#endif


    union {
        struct isp33_isp_params_cfg* mCvtedIsp33Prams;
        struct isp39_isp_params_cfg* mCvtedIsp39Prams;
        struct isp35_isp_params_cfg* mCvtedIsp35Prams;
    };

    struct IspParamsCvt_Ops mIspParamsCvtOps;
};

XCamReturn AiqIspParamsCvt_init(AiqIspParamsCvt_t* pCvt);
void AiqIspParamsCvt_deinit(AiqIspParamsCvt_t* pCvt);
void AiqAutoblc_deinit(AiqIspParamsCvt_t* pCvt);
XCamReturn AiqIspParamsCvt_merge_isp_results(AiqIspParamsCvt_t* pCvt, AiqList_t* results,
        void* isp_cfg, bool is_multi_isp, bool use_aiisp, bool _airms_en, bool _aiynr_en);
void AiqIspParamsCvt_setCamPhyId(AiqIspParamsCvt_t* pCvt, int phyId);
void AiqIspParamsCvt_set_working_mode(AiqIspParamsCvt_t* pCvt, int mode);
void AiqIspParamsCvt_setModuleStatus(AiqIspParamsCvt_t* pCvt, rk_aiq_module_id_t mId, bool en);
void AiqIspParamsCvt_getModuleStatus(AiqIspParamsCvt_t* pCvt, rk_aiq_module_id_t mId, bool* en);
bool AiqIspParamsCvt_getModuleForceFlag(AiqIspParamsCvt_t* pCvt, int module_id);
void AiqIspParamsCvt_setModuleForceFlagInverse(AiqIspParamsCvt_t* pCvt, int module_id);
bool AiqIspParamsCvt_getModuleForceEn(AiqIspParamsCvt_t* pCvt, int module_id);
void AiqIspParamsCvt_updateIspModuleForceEns(AiqIspParamsCvt_t* pCvt, u64 module_ens);
aiq_params_base_t* AiqIspParamsCvt_get_3a_result(AiqIspParamsCvt_t* pCvt, AiqList_t* results,
        int32_t type);
void AiqIspParamsCvt_getCommonCvtInfo(AiqIspParamsCvt_t* pCvt, AiqList_t* results, bool use_aiisp, bool _airms_en, bool _aiynr_en);
void AiqIspParamsCvt_setCalib(AiqIspParamsCvt_t* pCvt, const CamCalibDbV2Context_t* calibv2);
void AiqIspParamsCvt_setStastDelayCnt(AiqIspParamsCvt_t* pCvt, int delayCnt);

XCAM_END_DECLARE
#endif
