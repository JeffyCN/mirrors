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

#ifndef _CAM_HW_ISP3X_PARAMS_H_
#define _CAM_HW_ISP3X_PARAMS_H_

#include "rk_aiq_pool.h"
#include "isp21/Isp21Params.h"
#include "rkisp3-config.h"

namespace RkCam {

class Isp3xParams : virtual public Isp21Params {
public:
    explicit Isp3xParams() : Isp21Params() {};
    virtual ~Isp3xParams() {};
    void fixedAwbOveflowToIsp3xParams(void* isp_cfg_p, bool is_dual_isp);
protected:
    virtual bool convert3aResultsToIspCfg(SmartPtr<cam3aResult> &result, void* isp_cfg_p, bool is_multi_isp) override;
#if RKAIQ_HAVE_GAIN_V2
    template <class T>
    void convertAiqGainToIsp3xParams(T& isp_cfg, rk_aiq_isp_gain_v3x_t& gain);
#endif
#if RKAIQ_HAVE_GAMMA_V11
    template <class T>
    void convertAiqAgammaToIsp3xParams(T& isp_cfg, const rk_aiq_isp_goc_v3x_t& gamma_out_cfg);
#endif
private:
    XCAM_DEAD_COPY(Isp3xParams);
#if RKAIQ_HAVE_AWB_V21
    void convertAiqAwbToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_awb_meas_cfg_v3x_t& awb_meas,
                                    bool awb_cfg_udpate);
#endif
#if RKAIQ_HAVE_BAYER2DNR_V2
    void convertAiqRawnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_baynr_v3x_t& rawnr);
#endif
#if RKAIQ_HAVE_BAYERTNR_V2
    void convertAiqTnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_tnr_v3x_t& tnr);
#endif
#if RKAIQ_HAVE_CNR_V2
    void convertAiqUvnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_cnr_v3x_t& uvnr);
#endif
#if RKAIQ_HAVE_YNR_V3
    void convertAiqYnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ynr_v3x_t& ynr);
#endif
#if RKAIQ_HAVE_SHARP_V4
    void convertAiqSharpenToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_sharp_v3x_t& sharp);
#endif
#if RKAIQ_HAVE_DRC_V11
    void convertAiqDrcToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_v3x_t& adrc_data);
#endif
#if RKAIQ_HAVE_AF_V30 || RKAIQ_ONLY_AF_STATS_V30
    void convertAiqAfToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                   const rk_aiq_isp_af_meas_v3x_t& af_data, bool af_cfg_udpate);
#endif
#if RKAIQ_HAVE_MERGE_V11
    void convertAiqMergeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_merge_v3x_t& amerge_data);
#endif
#if RKAIQ_HAVE_CAC_V03 || RKAIQ_HAVE_CAC_V10
    void convertAiqCacToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    struct isp3x_isp_params_cfg& isp_cfg_right,
                                    const rk_aiq_isp_cac_v3x_t& cac_cfg, bool is_multi_isp);
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    void convertAiqAdehazeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                        const rk_aiq_isp_dehaze_v3x_t& dhaze);
#endif
#if defined(ISP_HW_V30)
void convertAiqLdchToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
        struct isp3x_isp_params_cfg& isp_cfg_right,
        const rk_aiq_isp_ldch_t& ldch_cfg, bool is_multi_isp);
#endif
};

}
#endif
