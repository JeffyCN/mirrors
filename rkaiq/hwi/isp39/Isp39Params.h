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
#include "rk_aiq_isp39_modules.h"

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
                                        const rk_aiq_wb_gain_v32_t& awb_gain,  bool awb_gain_update);
#if RKAIQ_HAVE_AWB_V32
    void convertAiqAwbToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_awb_meas_cfg_v32_t& awb_meas,
                                    bool awb_cfg_udpate);
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
    void convertAiqCacToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_cac_v32_t& cac_cfg);
#endif
#if RKAIQ_HAVE_DEBAYER_V3
    void convertAiqAdebayerToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                         rk_aiq_isp_debayer_v39_t & debayer);
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
    void convertAiqRgbirToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_rgbir_v39_t& adrc_data);
#endif
#if RKAIQ_HAVE_MERGE_V12
    void convertAiqMergeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_merge_v32_t& amerge_data);
#endif
#if RKAIQ_HAVE_CCM_V3
    void convertAiqCcmToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                    const rk_aiq_ccm_cfg_v2_t& ccm);
#endif
    void convertAiqAldchToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_ldch_v21_t& ldch_cfg);
    void convertAiqExpIspDgainToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, RKAiqAecExpInfo_t ae_exp);
#if RKAIQ_HAVE_YUVME_V1
    void convertAiqYuvmeToIsp39Params(struct isp39_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_yuvme_v39_t& yuvme);
#endif
#if RKAIQ_HAVE_DPCC_V2
#ifdef USE_NEWSTRUCT
    void convertAiqDpccToIsp39Params(struct isp39_isp_params_cfg& isp_cfg, rk_aiq_isp_dpcc_params_t* dpcc_attr);
#endif
#endif
};

}  // namespace RkCam
#endif
