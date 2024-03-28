/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef _CAM_HW_ISP32_PARAMS_H_
#define _CAM_HW_ISP32_PARAMS_H_

#include "isp3x/Isp3xParams.h"
#include "isp32/Isp32Params.h"
#include "rk_aiq_pool.h"
#include "rkisp32-config.h"

#if USE_NEWSTRUCT
#include "rk_aiq_isp32_modules.h"
#endif

namespace RkCam {

class Isp32Params : virtual public Isp3xParams {
public:
    explicit Isp32Params() : Isp3xParams() {};
    virtual ~Isp32Params() {};

protected:
    virtual bool convert3aResultsToIspCfg(SmartPtr<cam3aResult>& result, void* isp_cfg_p, bool is_multi_isp) override;

private:
    XCAM_DEAD_COPY(Isp32Params);


#if RKAIQ_HAVE_BAYER2DNR_V23
    void convertAiqRawnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_baynr_v32_t& rawnr);
#endif
#if (RKAIQ_HAVE_BAYERTNR_V23 || RKAIQ_HAVE_BAYERTNR_V23_LITE)
#ifdef USE_NEWSTRUCT
    void convertAiqBtnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_btnr_params_t* btnr_attr);
#else
    void convertAiqTnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_tnr_v32_t& tnr);
#endif
#endif
#if RKAIQ_HAVE_YNR_V22
#ifdef USE_NEWSTRUCT
    void convertAiqYnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_ynr_params_t* ynr_attr);
#else
    void convertAiqYnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ynr_v32_t& ynr);
#endif
#endif
#if (RKAIQ_HAVE_CNR_V30 || RKAIQ_HAVE_CNR_V30_LITE)
#if USE_NEWSTRUCT
    void convertAiqCnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_cnr_params_t *attr);
#else
    void convertAiqUvnrToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_cnr_v32_t& uvnr);
#endif
#endif
#if RKAIQ_HAVE_AWB_V32 || RKAIQ_HAVE_AWB_V32LT
    void convertAiqAwbGainToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                        const rk_aiq_wb_gain_v32_t& awb_gain,  bool awb_gain_update);
#endif
#if RKAIQ_HAVE_AWB_V32
    void convertAiqAwbToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_awb_meas_cfg_v32_t& awb_meas,
                                    bool awb_cfg_udpate);
#endif
#if (RKAIQ_HAVE_SHARP_V33 || RKAIQ_HAVE_SHARP_V33_LITE)
#ifdef USE_NEWSTRUCT
    void convertAiqSharpToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_sharp_params_t* sharp_attr);
#else
    void convertAiqSharpenToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_sharp_v32_t& sharp);
#endif
#endif
#if RKAIQ_HAVE_BLC_V32
    void convertAiqBlcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_blc_v32_t& blc);
#endif
#if RKAIQ_HAVE_AF_V31 || RKAIQ_ONLY_AF_STATS_V31
    void convertAiqAfToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                   const rk_aiq_isp_af_v31_t& af_data, bool af_cfg_udpate);
#endif
#if RKAIQ_HAVE_AF_V32_LITE || RKAIQ_ONLY_AF_STATS_V32_LITE
    void convertAiqAfLiteToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                   const rk_aiq_isp_af_v32_t& af_data, bool af_cfg_udpate);
#endif
#if RKAIQ_HAVE_CAC_V11
   void convertAiqCacToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
            struct isp32_isp_params_cfg& isp_cfg_right,
            const rk_aiq_isp_cac_v32_t& cac_cfg, bool is_multi_isp);
#endif
#if RKAIQ_HAVE_DEBAYER_V2 || RKAIQ_HAVE_DEBAYER_V2_LITE
#ifdef USE_NEWSTRUCT
    void convertAiqDmToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_dm_params_t* dm_attr);
#else
    void convertAiqAdebayerToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                         rk_aiq_isp_debayer_v32_t & debayer);
#endif
#endif
#if RKAIQ_HAVE_DEHAZE_V12
#ifdef USE_NEWSTRUCT
    void convertAiqDehazeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_dehaze_params_t* dehaze_attr);
#else
    void convertAiqAdehazeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                        const rk_aiq_isp_dehaze_v32_t& dhaze);
#endif
#endif
#if RKAIQ_HAVE_DRC_V12 || RKAIQ_HAVE_DRC_V12_LITE
#ifdef USE_NEWSTRUCT
    void convertAiqDrcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_params_t* drc_attr);
#else
    void convertAiqDrcToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_v32_t& adrc_data);
#endif
#endif
#if RKAIQ_HAVE_MERGE_V12
    void convertAiqMergeToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_merge_v32_t& amerge_data);
#endif
#if RKAIQ_HAVE_CCM_V2
    void convertAiqCcmToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                    const rk_aiq_ccm_cfg_v2_t& ccm);
#endif
#if RKAIQ_HAVE_LDCH_V21
    void convertAiqAldchToIsp32Params(struct isp32_isp_params_cfg& isp_cfg,
                                      struct isp32_isp_params_cfg& isp_cfg_right,
                                      const rk_aiq_isp_ldch_v21_t& ldch_cfg,
                                      bool is_multi_isp);
#endif
    void convertAiqExpIspDgainToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, RKAiqAecExpInfo_t ae_exp);

#ifdef RKAIQ_HAVE_GAMMA_V11
    #ifdef USE_NEWSTRUCT
    void convertAiqGammaToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_gamma_params_t* gamma_attr);
    #endif
#endif
#if RKAIQ_HAVE_DPCC_V1
#ifdef USE_NEWSTRUCT
    void convertAiqDpccToIsp32Params(struct isp32_isp_params_cfg& isp_cfg, rk_aiq_isp_dpcc_params_t* dpcc_attr);
#endif
#endif
};

}  // namespace RkCam
#endif
