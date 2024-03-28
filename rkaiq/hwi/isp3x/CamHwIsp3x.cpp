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

#include "CamHwIsp3x.h"
#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

#include "IspParamsSplitter.h"

namespace RkCam {

CamHwIsp3x::CamHwIsp3x()
    : CamHwIsp21()
{
    mVicapIspPhyLinkSupported = true;
}

CamHwIsp3x::~CamHwIsp3x()
{}

XCamReturn
CamHwIsp3x::init(const char* sns_ent_name)
{
    XCamReturn ret = CamHwIsp21::init(sns_ent_name);

    return ret;
}

XCamReturn
CamHwIsp3x::stop()
{
    XCamReturn ret = CamHwIsp21::stop();

    return ret;
}

void
CamHwIsp3x::updateEffParams(void* params, void* ori_params) {
#if defined(ISP_HW_V30)
    struct isp3x_isp_params_cfg* isp_params = (struct isp3x_isp_params_cfg*)params;
    uint32_t effFrmId = isp_params->frame_id;

    rkisp_effect_params_v20 latestIspParams;
    bool is_got_latest_params = false;
    uint32_t latest_id = effFrmId == 0 ? 0 : effFrmId - 1;

    if (XCAM_RETURN_NO_ERROR == getEffectiveIspParams(latestIspParams, latest_id))
        is_got_latest_params = true;

    SmartLock locker(_isp_params_cfg_mutex);

    if (getParamsForEffMap(effFrmId)) {
        if (mAwbParams) {
            RkAiqIspAwbParamsProxy* awbParams =
                dynamic_cast<RkAiqIspAwbParamsProxy*>(mAwbParams);
            _effecting_ispparam_map[effFrmId]->data()->result.awb_cfg_v3x = awbParams->data()->result;
        }

        if (mBlcResult) {
            RkAiqIspBlcParamsProxy* blcParams =
                dynamic_cast<RkAiqIspBlcParamsProxy*>(mBlcResult);
            _effecting_ispparam_map[effFrmId]->data()->result.blc_cfg = blcParams->data()->result;
        }

        struct isp3x_isp_params_cfg* dst_isp_params = NULL;
        struct isp3x_isp_params_cfg* lates_isp_params_ptr = NULL;
        struct isp3x_isp_params_cfg* new_isp_params = NULL;

#define UPDATE_MODULE_EFF_PARAMS(mask, new_param, latest_param, dst_param) \
        if (isp_params->module_cfg_update & (mask)) { \
            dst_param = new_param; \
        } else { \
           if (!is_got_latest_params) { \
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "impossible, no effect isp params!"); \
           } else { \
                dst_param = latest_param; \
           } \
        } \

#define UPDATE_EFF_PARAMS() \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAE0, new_isp_params->meas.rawae0, \
                                 lates_isp_params_ptr->meas.rawae0, dst_isp_params->meas.rawae0); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAE1, new_isp_params->meas.rawae1, \
                                 lates_isp_params_ptr->meas.rawae1, dst_isp_params->meas.rawae1); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAE2, new_isp_params->meas.rawae2, \
                                 lates_isp_params_ptr->meas.rawae2, dst_isp_params->meas.rawae2); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAE3, new_isp_params->meas.rawae3, \
                                 lates_isp_params_ptr->meas.rawae3, dst_isp_params->meas.rawae3); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWHIST0, new_isp_params->meas.rawhist0, \
                                 lates_isp_params_ptr->meas.rawhist0, dst_isp_params->meas.rawhist0); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWHIST1, new_isp_params->meas.rawhist1, \
                                 lates_isp_params_ptr->meas.rawhist1, dst_isp_params->meas.rawhist1); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWHIST2, new_isp_params->meas.rawhist2, \
                                 lates_isp_params_ptr->meas.rawhist2, dst_isp_params->meas.rawhist2); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWHIST3, new_isp_params->meas.rawhist3, \
                                 lates_isp_params_ptr->meas.rawhist3, dst_isp_params->meas.rawhist3); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAWB, new_isp_params->meas.rawawb, \
                                 lates_isp_params_ptr->meas.rawawb, dst_isp_params->meas.rawawb); \
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_RAWAF, new_isp_params->meas.rawaf, \
                                 lates_isp_params_ptr->meas.rawaf, dst_isp_params->meas.rawaf);\
        UPDATE_MODULE_EFF_PARAMS(ISP2X_MODULE_BLS, new_isp_params->others.bls_cfg, \
                                 lates_isp_params_ptr->others.bls_cfg, dst_isp_params->others.bls_cfg); \

        if (mIsMultiIspMode) {
            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v3x[0];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v3x[0];
            new_isp_params = (struct isp3x_isp_params_cfg*)ori_params;
            UPDATE_EFF_PARAMS();

            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v3x[1];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v3x[1];
            new_isp_params = &isp_params[0];
            UPDATE_EFF_PARAMS();

            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v3x[2];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v3x[2];
            new_isp_params = &isp_params[1];
            UPDATE_EFF_PARAMS();
        } else {
            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v3x[0];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v3x[0];
            new_isp_params = isp_params;
            UPDATE_EFF_PARAMS();
        }
    }
#endif
}

bool
CamHwIsp3x::processTb(void* params) {
    return false;
}

}
