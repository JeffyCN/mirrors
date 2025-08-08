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

#include "CamHwIsp21.h"
#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

namespace RkCam {

CamHwIsp21::CamHwIsp21()
    : CamHwIsp20()
{
    mNoReadBack = true;
#ifndef ANDROID_OS
    char* valueStr = getenv("normal_no_read_back");
    if (valueStr) {
        mNoReadBack = atoi(valueStr) > 0 ? true : false;
    }
#else
    char property_value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("persist.vendor.rkisp_no_read_back", property_value, "-1");
    int val = atoi(property_value);
    if (val != -1)
        mNoReadBack = atoi(property_value) > 0 ? true : false;
#endif
}

CamHwIsp21::~CamHwIsp21()
{}

XCamReturn
CamHwIsp21::init(const char* sns_ent_name)
{
    XCamReturn ret = CamHwIsp20::init(sns_ent_name);

    return ret;
}

XCamReturn
CamHwIsp21::stop()
{
    XCamReturn ret = CamHwIsp20::stop();

    return ret;
}

bool CamHwIsp21::isOnlineByWorkingMode()
{
    return false;
}

XCamReturn
CamHwIsp21::dispatchResult(cam3aResultList& list)
{
    cam3aResultList isp_result_list;
    for (auto& result : list) {
        switch (result->getType()) {
        case RESULT_TYPE_FLASH_PARAM:
        case RESULT_TYPE_CPSL_PARAM:
        case RESULT_TYPE_IRIS_PARAM:
        case RESULT_TYPE_FOCUS_PARAM:
        case RESULT_TYPE_EXPOSURE_PARAM:
        case RESULT_TYPE_AFD_PARAM:
            CamHwIsp20::dispatchResult(result);
            break;
        default:
            isp_result_list.push_back(result);
            break;
        }
    }

    if (isp_result_list.size() > 0) {
        handleIsp3aReslut(isp_result_list);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
CamHwIsp21::dispatchResult(SmartPtr<cam3aResult> result)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (!result.ptr())
        return XCAM_RETURN_ERROR_PARAM;

    LOGD("%s enter, msg type(0x%x)", __FUNCTION__, result->getType());
    switch (result->getType())
    {
    case RESULT_TYPE_FLASH_PARAM:
    case RESULT_TYPE_CPSL_PARAM:
    case RESULT_TYPE_IRIS_PARAM:
    case RESULT_TYPE_FOCUS_PARAM:
    case RESULT_TYPE_EXPOSURE_PARAM:
        return CamHwIsp20::dispatchResult(result);
    default:
        handleIsp3aReslut(result);
        break;
    }
    return ret;
    LOGD("%s exit", __FUNCTION__);
}

void
CamHwIsp21::updateEffParams(void* params, void* ori_params) {
#ifdef ISP_HW_V21
    struct isp21_isp_params_cfg* isp_params = (struct isp21_isp_params_cfg*)params;
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
            _effecting_ispparam_map[effFrmId]->data()->result.awb_cfg_v201 = awbParams->data()->result;
        }

        if (mBlcResult) {
            RkAiqIspBlcParamsProxy* blcParams =
                dynamic_cast<RkAiqIspBlcParamsProxy*>(mBlcResult);
            _effecting_ispparam_map[effFrmId]->data()->result.blc_cfg = blcParams->data()->result;
        }

        struct isp21_isp_params_cfg* dst_isp_params = NULL;
        struct isp21_isp_params_cfg* lates_isp_params_ptr = NULL;
        struct isp21_isp_params_cfg* new_isp_params = NULL;

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

        dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v21;
        if (is_got_latest_params)
            lates_isp_params_ptr = &latestIspParams.isp_params_v21;
        new_isp_params = isp_params;
        UPDATE_EFF_PARAMS();
    }
#endif
}

bool
CamHwIsp21::processTb(void* params) {
    return false;
}

}
