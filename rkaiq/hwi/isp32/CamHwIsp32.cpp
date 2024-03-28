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
#include "CamHwIsp32.h"

#ifdef ANDROID_OS
#include <cutils/properties.h>
#endif

#include "IspParamsSplitter.h"

namespace RkCam {

CamHwIsp32::CamHwIsp32() : CamHwIsp3x() {
    mVicapIspPhyLinkSupported = true;
    if (mIsMultiIspMode) {
        mNoReadBack = false;
    }
}

CamHwIsp32::~CamHwIsp32() {}

XCamReturn CamHwIsp32::init(const char* sns_ent_name) {
    XCamReturn ret = CamHwIsp3x::init(sns_ent_name);
    return ret;
}

XCamReturn CamHwIsp32::stop() {
    XCamReturn ret = CamHwIsp3x::stop();
    return ret;
}

void
CamHwIsp32::updateEffParams(void* params, void* ori_params)
{
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    struct isp32_isp_params_cfg* isp_params = (struct isp32_isp_params_cfg*)params;
    uint32_t effFrmId = isp_params->frame_id;

#if defined(RKAIQ_HAVE_MULTIISP)
    rkisp_effect_params_v20 latestIspParams;
    bool is_got_latest_params = false;
    uint32_t latest_id = effFrmId == 0 ? 0 : effFrmId - 1;

    if (XCAM_RETURN_NO_ERROR == getEffectiveIspParams(latestIspParams, latest_id))
        is_got_latest_params = true;
#endif

    SmartLock locker(_isp_params_cfg_mutex);

    if (getParamsForEffMap(effFrmId)) {
        if (mAwbParams) {
            RkAiqIspAwbParamsProxy* awbParams =
                dynamic_cast<RkAiqIspAwbParamsProxy*>(mAwbParams);
            _effecting_ispparam_map[effFrmId]->data()->result.awb_cfg_v32 = awbParams->data()->result;
        }
        _effecting_ispparam_map[effFrmId]->data()->result.meas = mLatestMeasCfg;
        _effecting_ispparam_map[effFrmId]->data()->result.bls_cfg = mLatestBlsCfg;
        _effecting_ispparam_map[effFrmId]->data()->result.awb_gain_cfg = mLatestWbGainCfg;

#if defined(RKAIQ_HAVE_MULTIISP)
        struct isp32_isp_params_cfg* dst_isp_params = NULL;
        struct isp32_isp_params_cfg* lates_isp_params_ptr = NULL;
        struct isp32_isp_params_cfg* new_isp_params = NULL;

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
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAE0, new_isp_params->meas.rawae0, \
                                 lates_isp_params_ptr->meas.rawae0, dst_isp_params->meas.rawae0); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAE1, new_isp_params->meas.rawae1, \
                                 lates_isp_params_ptr->meas.rawae1, dst_isp_params->meas.rawae1); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAE2, new_isp_params->meas.rawae2, \
                                 lates_isp_params_ptr->meas.rawae2, dst_isp_params->meas.rawae2); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAE3, new_isp_params->meas.rawae3, \
                                 lates_isp_params_ptr->meas.rawae3, dst_isp_params->meas.rawae3); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWHIST0, new_isp_params->meas.rawhist0, \
                                 lates_isp_params_ptr->meas.rawhist0, dst_isp_params->meas.rawhist0); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWHIST1, new_isp_params->meas.rawhist1, \
                                 lates_isp_params_ptr->meas.rawhist1, dst_isp_params->meas.rawhist1); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWHIST2, new_isp_params->meas.rawhist2, \
                                 lates_isp_params_ptr->meas.rawhist2, dst_isp_params->meas.rawhist2); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWHIST3, new_isp_params->meas.rawhist3, \
                                 lates_isp_params_ptr->meas.rawhist3, dst_isp_params->meas.rawhist3); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAWB, new_isp_params->meas.rawawb, \
                                 lates_isp_params_ptr->meas.rawawb, dst_isp_params->meas.rawawb); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_RAWAF, new_isp_params->meas.rawaf, \
                                 lates_isp_params_ptr->meas.rawaf, dst_isp_params->meas.rawaf);\
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_BLS, new_isp_params->others.bls_cfg, \
                                 lates_isp_params_ptr->others.bls_cfg, dst_isp_params->others.bls_cfg); \
        UPDATE_MODULE_EFF_PARAMS(ISP32_MODULE_AWB_GAIN, new_isp_params->others.awb_gain_cfg, \
                                 lates_isp_params_ptr->others.awb_gain_cfg, dst_isp_params->others.awb_gain_cfg); \

        if (mIsMultiIspMode) {
            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v32[0];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v32[0];
            new_isp_params = (struct isp32_isp_params_cfg*)ori_params;
            UPDATE_EFF_PARAMS();

            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v32[1];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v32[1];
            new_isp_params = (struct isp32_isp_params_cfg*)ori_params;
            UPDATE_EFF_PARAMS();

#if defined(ISP_HW_V32_LITE)
            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v32[2];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v32[2];
            new_isp_params = (struct isp32_isp_params_cfg*)ori_params;
            UPDATE_EFF_PARAMS();

            dst_isp_params = &_effecting_ispparam_map[effFrmId]->data()->result.isp_params_v32[3];
            if (is_got_latest_params)
                lates_isp_params_ptr = &latestIspParams.isp_params_v32[3];
            new_isp_params = (struct isp32_isp_params_cfg*)ori_params;
            UPDATE_EFF_PARAMS();
#endif
        }
#endif

    }
#endif
}

bool
CamHwIsp32::processTb(void* params)
{
#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    struct isp32_isp_params_cfg* isp_params = (struct isp32_isp_params_cfg*)params;
    if (mTbInfo.is_fastboot) {
        if (isp_params->frame_id == 0 && _not_skip_first) {
            _not_skip_first = false;
            if (!_first_awb_param && mAwbParams) {
                _first_awb_param = malloc(sizeof(rk_aiq_awb_stat_cfg_v32_t));
                RkAiqIspAwbParamsProxy* awbParams =
                    dynamic_cast<RkAiqIspAwbParamsProxy*>(mAwbParams);
                *((rk_aiq_awb_stat_cfg_v32_t*)_first_awb_param) = awbParams->data()->result;
            }
            if (!_first_awb_cfg) {
                _first_awb_cfg = malloc(sizeof(struct isp32_rawawb_meas_cfg));
                *((struct isp32_rawawb_meas_cfg*)_first_awb_cfg) = isp_params->meas.rawawb;
            }
            if (!_skipped_params) {
                _skipped_params = malloc(sizeof(struct isp32_isp_params_cfg));
                *((struct isp32_isp_params_cfg*)_skipped_params) = *isp_params;
                LOGK_CAMHW("cid[%d], fid[%d], skip_en_up: 0x%llx, cfg_up:0x%llx",
                        mCamPhyId, isp_params->frame_id,
                        isp_params->module_en_update,
                        isp_params->module_cfg_update);
            }
            LOGK_CAMHW("<TB> Skip config id(%d)'s isp params", isp_params->frame_id);
            return true;
        } else if (!_not_skip_first) {
            if (isp_params->frame_id < 10) {
                if (_first_awb_cfg) {
                    struct isp32_rawawb_meas_cfg* first_awb_cfg = (struct isp32_rawawb_meas_cfg*)_first_awb_cfg;
                    first_awb_cfg->pre_wbgain_inv_r = isp_params->meas.rawawb.pre_wbgain_inv_r;
                    first_awb_cfg->pre_wbgain_inv_g = isp_params->meas.rawawb.pre_wbgain_inv_g;
                    first_awb_cfg->pre_wbgain_inv_b = isp_params->meas.rawawb.pre_wbgain_inv_b;
                    isp_params->meas.rawawb = *first_awb_cfg;
                    mLatestMeasCfg.rawawb = *first_awb_cfg;
                }
                if (getParamsForEffMap(isp_params->frame_id)) {
                    if (mAwbParams && _first_awb_param) {
                        rk_aiq_awb_stat_cfg_v32_t *first_awb_param = (rk_aiq_awb_stat_cfg_v32_t*)_first_awb_param;
                        RkAiqIspAwbParamsProxy* awbParams =
                            dynamic_cast<RkAiqIspAwbParamsProxy*>(mAwbParams);
                        memcpy(first_awb_param->preWbgainSw, awbParams->data()->result.preWbgainSw, 4 * sizeof(float));
                        awbParams->data()->result = *first_awb_param;
                        _effecting_ispparam_map[isp_params->frame_id]->data()->result.awb_cfg_v32 = awbParams->data()->result;
                    }
                    _effecting_ispparam_map[isp_params->frame_id]->data()->result.meas = mLatestMeasCfg;
                }
            } else {
                if (_first_awb_param) {
                    free(_first_awb_param);
                    _first_awb_param = NULL;
                }
                if (_first_awb_cfg) {
                    free(_first_awb_cfg);
                    _first_awb_cfg = NULL;
                }
            }

            if (!_skipped_params) {
                return false;
            }

            struct isp32_isp_params_cfg* skip_param = (struct isp32_isp_params_cfg*)_skipped_params;
            struct isp32_isp_params_cfg* new_param = (struct isp32_isp_params_cfg*)params;

            uint64_t skip_cfg_up = skip_param->module_cfg_update & (new_param->module_cfg_update ^ skip_param->module_cfg_update);
            uint64_t skip_en_up = skip_param->module_en_update & (new_param->module_en_update ^ skip_param->module_en_update);

            if (skip_cfg_up) {
                LOGK_CAMHW("cid[%d], fid[%d], skip_cfg_up: 0x%llx",
                        mCamPhyId, isp_params->frame_id, skip_cfg_up);
                if (skip_cfg_up & ISP32_MODULE_RAWAF) {
                    new_param->module_cfg_update |= ISP32_MODULE_RAWAF;
                    new_param->meas.rawaf = skip_param->meas.rawaf;
                }
            }

            if (skip_en_up) {
                LOGK_CAMHW("cid[%d], fid[%d], skip_en_up: 0x%llx",
                        mCamPhyId, isp_params->frame_id, skip_cfg_up);
                if (skip_cfg_up & ISP32_MODULE_RAWAF) {
                    new_param->module_en_update |= ISP32_MODULE_RAWAF;
                    new_param->module_ens &= ~ISP32_MODULE_RAWAF;
                    new_param->module_ens |= skip_param->module_ens & ISP32_MODULE_RAWAF;
                }
            }

            free(_skipped_params);
            _skipped_params = NULL;
        }

        LOGK_CAMHW("<TB> Config id(%u)'s isp params, ens 0x%llx ens_up 0x%llx, cfg_up 0x%llx", isp_params->frame_id,
                        isp_params->module_ens,
                        isp_params->module_en_update,
                        isp_params->module_cfg_update);
        return false;
    } else if (isp_params->frame_id == 0 && _not_skip_first) {
        _not_skip_first = false;
        return true;
    } else {
       return false;
    }
#else
    return false;
#endif
}

}  // namespace RkCam
