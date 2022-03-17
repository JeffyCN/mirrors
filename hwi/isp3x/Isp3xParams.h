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
protected:
    virtual bool convert3aResultsToIspCfg(SmartPtr<cam3aResult> &result, void* isp_cfg_p);
    void fixedAwbOveflowToIsp3xParams(void* isp_cfg_p,bool is_dual_isp);
private:
    XCAM_DEAD_COPY(Isp3xParams);
    void convertAiqAwbToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    const rk_aiq_isp_awb_meas_cfg_v3x_t& awb_meas,
                                    bool awb_cfg_udpate);
    void convertAiqRawnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                      rk_aiq_isp_baynr_v3x_t& rawnr);
    void convertAiqTnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_tnr_v3x_t& tnr);
    void convertAiqUvnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_cnr_v3x_t& uvnr);
    void convertAiqYnrToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_ynr_v3x_t& ynr);
    void convertAiqSharpenToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                        rk_aiq_isp_sharp_v3x_t& sharp);
    void convertAiqDrcToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    rk_aiq_isp_drc_v3x_t& adrc_data);
    void convertAiqAfToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                   const rk_aiq_isp_af_meas_v3x_t& af_data, bool af_cfg_udpate);
    void convertAiqMergeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                      const rk_aiq_isp_merge_v3x_t& amerge_data);
    void convertAiqAgammaToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                       const rk_aiq_isp_goc_v3x_t& gamma_out_cfg);
    void convertAiqCacToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                    struct isp3x_isp_params_cfg& isp_cfg_right,
                                    const rk_aiq_isp_cac_v3x_t& cac_cfg);
    void convertAiqAdehazeToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                        const rk_aiq_isp_dehaze_v3x_t& dhaze);
    void convertAiqGainToIsp3xParams(struct isp3x_isp_params_cfg& isp_cfg,
                                     rk_aiq_isp_gain_v3x_t& gain);
};

};
#endif
