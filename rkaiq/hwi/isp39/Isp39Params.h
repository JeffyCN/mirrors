/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
#ifndef _CAM_HW_ISP39_PARAMS_H_
#define _CAM_HW_ISP39_PARAMS_H_

#include "isp3x/Isp3xParams.h"
#include "isp32/Isp32Params.h"
#include "rk_aiq_pool.h"
#include "rk-isp39-config.h"

#if USE_NEWSTRUCT
#include "rk_aiq_isp39_modules.h"
#endif

namespace RkCam {

class Isp39Params : virtual public Isp32Params {
public:
    explicit Isp39Params() : Isp32Params() {};
    virtual ~Isp39Params() {};

protected:
    virtual bool convert3aResultsToIspCfg(SmartPtr<cam3aResult>& result, void* isp_cfg_p, bool is_multi_isp) override;

private:
    XCAM_DEAD_COPY(Isp39Params);

#if (RKAIQ_HAVE_BAYERTNR_V30)
#if USE_NEWSTRUCT
    void convertAiqBtnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_btnr_params_t *btnr_attr);
#else
    void convertAiqTnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_tnr_v39_t& tnr);
#endif
#endif
#if RKAIQ_HAVE_YNR_V24
#if USE_NEWSTRUCT
    void convertAiqYnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ynr_params_t *ynr_attr);
#else
    void convertAiqYnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ynr_v39_t& ynr);
#endif
#endif
#if (RKAIQ_HAVE_CNR_V31)
#if USE_NEWSTRUCT
    void convertAiqCnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_cnr_params_t *attr);
#else
    void convertAiqUvnrToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_cnr_v39_t& uvnr);
#endif
#endif
    void convertAiqAwbGainToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
            const rk_aiq_wb_gain_v32_t& awb_gain_ori, bool awb_gain_update);
#if RKAIQ_HAVE_AWB_V39
    void convertAiqAwbToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
        const rk_aiq_isp_awb_meas_cfg_v39_t& awb_meas_priv,bool awb_cfg_udpate);
#endif
#if (RKAIQ_HAVE_SHARP_V34)
#if USE_NEWSTRUCT
    void convertAiqSharpToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_sharp_params_t *attr);
#else
    void convertAiqSharpenToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_sharp_v39_t& sharp);
#endif
#endif
#if USE_NEWSTRUCT
    void convertAiqCsmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_csm_params_t* csm_attr);

    void convertAiq3dlutToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_lut3d_params_t* lut3d_attr);

    void convertAiqBlcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_blc_params_t* blc_attr);
#else
    void convertAiqBlcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_blc_v32_t& blc);
#endif

#if RKAIQ_HAVE_AF_V33 || RKAIQ_ONLY_AF_STATS_V33
#ifdef USE_NEWSTRUCT
    void convertAiqAfToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                   const afStats_cfg_t& af_data, bool af_cfg_udpate);
#else
    void convertAiqAfToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                   const rk_aiq_isp_af_v33_t& af_data, bool af_cfg_udpate);
#endif
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
    void convertAiqAfLiteToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                       const rk_aiq_isp_af_v32_t& af_data, bool af_cfg_udpate);
#endif
#if RKAIQ_HAVE_CAC_V11
#ifdef USE_NEWSTRUCT
    void convertAiqCacToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, struct isp39_isp_params_cfg& isp_cfg_right,
                                    rk_aiq_isp_cac_params_t* cac_attr, bool is_multi_isp);
#else
    void convertAiqCacToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_cac_v32_t& cac_cfg);
#endif
#endif
#if RKAIQ_HAVE_DEBAYER_V3
#ifdef USE_NEWSTRUCT
    void convertAiqDmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_dm_params_t* dm_attr);
#else
    void convertAiqAdebayerToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                         rk_aiq_isp_debayer_v39_t & debayer);
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V14
#ifdef USE_NEWSTRUCT
    void convertAiqDehazeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                       rk_aiq_isp_dehaze_params_t* dehaze_attr);
#else
    void convertAiqAdehazeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                        const rk_aiq_isp_dehaze_v39_t& dhaze);
#endif
#endif
#if RKAIQ_HAVE_DRC_V20
#ifdef USE_NEWSTRUCT
    void convertAiqDrcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_params_t* drc_attr);
#else
    void convertAiqDrcToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_v39_t& adrc_data);
#endif
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
#ifdef USE_NEWSTRUCT
    void convertAiqRgbirToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_rgbir_params_t* rgbir_attr);
#else
    void convertAiqRgbirToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_rgbir_v39_t& adrc_data);
#endif
#endif
#if RKAIQ_HAVE_MERGE_V12
#ifdef USE_NEWSTRUCT
    void convertAiqMergeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_merge_params_t* merge_attr);
#else
    void convertAiqMergeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_merge_v32_t& amerge_data);
#endif
#endif
#if RKAIQ_HAVE_CCM_V3
#if USE_NEWSTRUCT
    void convertAiqCcmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ccm_params_t* ccm_attr);
#else
    void convertAiqCcmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    const rk_aiq_ccm_cfg_v2_t& ccm);
#endif
#endif
#if RKAIQ_HAVE_LDCH_V21
#ifdef USE_NEWSTRUCT
    void convertAiqLdchToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, struct isp39_isp_params_cfg& isp_cfg_right,
                                     rk_aiq_isp_ldch_params_t* ldch_attr, bool is_multi_isp);
#else
    void convertAiqAldchToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,

                                      const rk_aiq_isp_ldch_v21_t& ldch_cfg);
#endif
#endif
#if RKAIQ_HAVE_LDC
    void convertAiqAldchToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      const rkaiq_ldch_v22_hw_param_t& ldch_cfg, bool enable);
    void convertAiqAldcvToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      const rkaiq_ldcv_v22_hw_param_t& ldcv_cfg, bool enable);
#endif
    void convertAiqExpIspDgainToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, RKAiqAecExpInfo_t ae_exp);
#if RKAIQ_HAVE_YUVME_V1
#if USE_NEWSTRUCT
    void convertAiqYmeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_yme_params_t *yme_attr);
#else
    void convertAiqYuvmeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_yuvme_v39_t& yuvme);
#endif
#endif
#if RKAIQ_HAVE_DPCC_V2
#ifdef USE_NEWSTRUCT
    void convertAiqDpccToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_dpcc_params_t* dpcc_attr);
#endif
#endif

#if RKAIQ_HAVE_GAMMA_V11
#ifdef USE_NEWSTRUCT
    void convertAiqGammaToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_gamma_params_t* gamma_attr);
#endif
#endif
#if RKAIQ_HAVE_LSC_V3 && USE_NEWSTRUCT
    void convertAiqLscToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_lsc_params_t* lsc_attr);
#endif
#if RKAIQ_HAVE_GIC_V2
void convertAiqAgicToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                 const rk_aiq_isp_gic_v21_t& agic);
#endif
private:
    rk_aiq_wb_gain_v32_t awb_gain_final;

};

}  // namespace RkCam
#endif
