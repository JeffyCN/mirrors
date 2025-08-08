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

#include "aiq_CamHwIsp39.h"

#include "hwi_c/aiq_CamHwBase.h"
#include "hwi_c/aiq_ispParamsCvt.h"

static void CamHwIsp39_updateEffParams(AiqCamHwBase_t* pCamHw, void* params, void* ori_params) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_params = (struct isp39_isp_params_cfg*)params;
    uint32_t effFrmId                       = isp_params->frame_id;

#if defined(RKAIQ_HAVE_MULTIISP)
    aiq_isp_effect_params_t* latestIspParams = NULL;
    bool is_got_latest_params                = false;
    uint32_t latest_id                       = effFrmId == 0 ? 0 : effFrmId - 1;

    if (XCAM_RETURN_NO_ERROR ==
            AiqCamHw_getEffectiveIspParams(pCamHw, &latestIspParams, latest_id) &&
        latestIspParams)
        is_got_latest_params = true;
#endif

    aiqMutex_lock(&pCamHw->_isp_params_cfg_mutex);
    aiq_isp_effect_params_t* pDstEff = AiqCamHw_getParamsForEffMap(pCamHw, effFrmId);
    if (!pDstEff) {
        goto out;
    }
    if (pCamHw->_mIspParamsCvt->mAwbParams) {
        pDstEff->awb_cfg_v39 =
            *(rk_aiq_isp_awb_meas_cfg_v39_t*)(pCamHw->_mIspParamsCvt->mAwbParams->_data);
    }

    if (pCamHw->_mIspParamsCvt->mAeParams) {
#if defined(USE_NEWSTRUCT)
        pDstEff->ae_cfg_v39 = *(aeStats_cfg_t*)(pCamHw->_mIspParamsCvt->mAeParams->_data);
#endif
    }
    pDstEff->meas         = pCamHw->_mIspParamsCvt->mLatestMeasCfg;
    pDstEff->bls_cfg      = pCamHw->_mIspParamsCvt->mLatestBlsCfg;
    pDstEff->awb_gain_cfg = pCamHw->_mIspParamsCvt->mLatestWbGainCfg;

#if defined(RKAIQ_HAVE_MULTIISP)
    struct isp39_isp_params_cfg* dst_isp_params       = NULL;
    struct isp39_isp_params_cfg* lates_isp_params_ptr = NULL;
    struct isp39_isp_params_cfg* new_isp_params       = NULL;

#define UPDATE_MODULE_EFF_PARAMS(mask, new_param, latest_param, dst_param)      \
    if (isp_params->module_cfg_update & (mask)) {                               \
        dst_param = new_param;                                                  \
    } else {                                                                    \
        if (!is_got_latest_params) {                                            \
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "impossible, no effect isp params!"); \
        } else {                                                                \
            dst_param = latest_param;                                           \
        }                                                                       \
    }

#define UPDATE_EFF_PARAMS()                                                                       \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWAE0, new_isp_params->meas.rawae0,                    \
                             lates_isp_params_ptr->meas.rawae0, dst_isp_params->meas.rawae0);     \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWAE3, new_isp_params->meas.rawae3,                    \
                             lates_isp_params_ptr->meas.rawae3, dst_isp_params->meas.rawae3);     \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWHIST0, new_isp_params->meas.rawhist0,                \
                             lates_isp_params_ptr->meas.rawhist0, dst_isp_params->meas.rawhist0); \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWHIST3, new_isp_params->meas.rawhist3,                \
                             lates_isp_params_ptr->meas.rawhist3, dst_isp_params->meas.rawhist3); \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWAWB, new_isp_params->meas.rawawb,                    \
                             lates_isp_params_ptr->meas.rawawb, dst_isp_params->meas.rawawb);     \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_RAWAF, new_isp_params->meas.rawaf,                      \
                             lates_isp_params_ptr->meas.rawaf, dst_isp_params->meas.rawaf);       \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_BLS, new_isp_params->others.bls_cfg,                    \
                             lates_isp_params_ptr->others.bls_cfg,                                \
                             dst_isp_params->others.bls_cfg);                                     \
    UPDATE_MODULE_EFF_PARAMS(ISP39_MODULE_AWB_GAIN, new_isp_params->others.awb_gain_cfg,          \
                             lates_isp_params_ptr->others.awb_gain_cfg,                           \
                             dst_isp_params->others.awb_gain_cfg);

    if (g_mIsMultiIspMode) {
        dst_isp_params = &pDstEff->isp_params_v39[0];
        if (is_got_latest_params) lates_isp_params_ptr = &latestIspParams->isp_params_v39[0];
        new_isp_params = (struct isp39_isp_params_cfg*)ori_params;
        UPDATE_EFF_PARAMS();

        dst_isp_params = &pDstEff->isp_params_v39[1];
        if (is_got_latest_params) lates_isp_params_ptr = &latestIspParams->isp_params_v39[1];
        new_isp_params = (struct isp39_isp_params_cfg*)ori_params;
        UPDATE_EFF_PARAMS();
    }
#endif

out:
#if defined(RKAIQ_HAVE_MULTIISP)
    if (latestIspParams)
        AIQ_REF_BASE_UNREF(&latestIspParams->_ref_base);

#endif
    aiqMutex_unlock(&pCamHw->_isp_params_cfg_mutex);
#endif
}

bool CamHwIsp39_processTb(AiqCamHwBase_t* pCamHw, void* params) {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_params = (struct isp39_isp_params_cfg*)params;
    if (pCamHw->mTbInfo.is_fastboot) {
        if (isp_params->frame_id == 0 && pCamHw->_not_skip_first) {
            pCamHw->_not_skip_first = false;
            if (!pCamHw->_first_awb_param && pCamHw->_mIspParamsCvt->mAwbParams) {
                pCamHw->_first_awb_param = aiq_mallocz(sizeof(rk_aiq_awb_stat_cfg_v32_t));
                *((rk_aiq_awb_stat_cfg_v32_t*)pCamHw->_first_awb_param) =
                    *(rk_aiq_isp_awb_meas_cfg_v32_t*)(pCamHw->_mIspParamsCvt->mAwbParams->_data);
            }
            if (!pCamHw->_first_awb_cfg) {
                pCamHw->_first_awb_cfg = aiq_mallocz(sizeof(struct isp39_rawawb_meas_cfg));
                *((struct isp39_rawawb_meas_cfg*)pCamHw->_first_awb_cfg) = isp_params->meas.rawawb;
            }
            if (!pCamHw->_skipped_params) {
                pCamHw->_skipped_params = aiq_mallocz(sizeof(struct isp39_isp_params_cfg));
                *((struct isp39_isp_params_cfg*)pCamHw->_skipped_params) = *isp_params;
                LOGK_CAMHW("cid[%d], fid[%d], skip_en_up: 0x%llx, cfg_up:0x%llx", pCamHw->mCamPhyId,
                           isp_params->frame_id, isp_params->module_en_update,
                           isp_params->module_cfg_update);
            }
            LOGK_CAMHW("<TB> Skip config id(%d)'s isp params", isp_params->frame_id);
            return true;
        } else if (!pCamHw->_not_skip_first) {
            if (isp_params->frame_id < 10) {
                if (pCamHw->_first_awb_cfg) {
                    struct isp39_rawawb_meas_cfg* first_awb_cfg =
                        (struct isp39_rawawb_meas_cfg*)pCamHw->_first_awb_cfg;
                    first_awb_cfg->pre_wbgain_inv_r = isp_params->meas.rawawb.pre_wbgain_inv_r;
                    first_awb_cfg->pre_wbgain_inv_g = isp_params->meas.rawawb.pre_wbgain_inv_g;
                    first_awb_cfg->pre_wbgain_inv_b = isp_params->meas.rawawb.pre_wbgain_inv_b;
                    isp_params->meas.rawawb         = *first_awb_cfg;
                    pCamHw->_mIspParamsCvt->mLatestMeasCfg.rawawb = *first_awb_cfg;
                }

                aiq_isp_effect_params_t* pDstEff =
                    AiqCamHw_getParamsForEffMap(pCamHw, isp_params->frame_id);
                if (pDstEff) {
                    rk_aiq_isp_awb_meas_cfg_v39_t* awb_meas_cfg =
                        (rk_aiq_isp_awb_meas_cfg_v39_t*)pCamHw->_mIspParamsCvt->mAwbParams->_data;
                    if (pCamHw->_mIspParamsCvt->mAwbParams && pCamHw->_first_awb_param) {
                        rk_aiq_isp_awb_meas_cfg_v39_t* first_awb_param =
                            (rk_aiq_isp_awb_meas_cfg_v39_t*)pCamHw->_first_awb_param;
                        memcpy(first_awb_param->preWbgainSw, awb_meas_cfg->preWbgainSw,
                               4 * sizeof(float));
                        *awb_meas_cfg        = *first_awb_param;
                        pDstEff->awb_cfg_v39 = *awb_meas_cfg;
                    }
                    pDstEff->meas = pCamHw->_mIspParamsCvt->mLatestMeasCfg;
                }
            } else {
                if (pCamHw->_first_awb_param) {
                    aiq_free(pCamHw->_first_awb_param);
                    pCamHw->_first_awb_param = NULL;
                }
                if (pCamHw->_first_awb_cfg) {
                    aiq_free(pCamHw->_first_awb_cfg);
                    pCamHw->_first_awb_cfg = NULL;
                }
            }

            if (!pCamHw->_skipped_params) {
                return false;
            }

            struct isp39_isp_params_cfg* skip_param =
                (struct isp39_isp_params_cfg*)pCamHw->_skipped_params;
            struct isp39_isp_params_cfg* new_param  = (struct isp39_isp_params_cfg*)params;

            uint64_t skip_cfg_up = skip_param->module_cfg_update &
                                   (new_param->module_cfg_update ^ skip_param->module_cfg_update);
            uint64_t skip_en_up = skip_param->module_en_update &
                                  (new_param->module_en_update ^ skip_param->module_en_update);

            if (skip_cfg_up) {
                LOGK_CAMHW("cid[%d], fid[%d], skip_cfg_up: 0x%llx", pCamHw->mCamPhyId,
                           isp_params->frame_id, skip_cfg_up);
                if (skip_cfg_up & ISP32_MODULE_RAWAF) {
                    new_param->module_cfg_update |= ISP32_MODULE_RAWAF;
                    new_param->meas.rawaf = skip_param->meas.rawaf;
                }
            }

            if (skip_en_up) {
                LOGK_CAMHW("cid[%d], fid[%d], skip_en_up: 0x%llx", pCamHw->mCamPhyId,
                           isp_params->frame_id, skip_cfg_up);
                if (skip_cfg_up & ISP32_MODULE_RAWAF) {
                    new_param->module_en_update |= ISP32_MODULE_RAWAF;
                    new_param->module_ens &= ~ISP32_MODULE_RAWAF;
                    new_param->module_ens |= skip_param->module_ens & ISP32_MODULE_RAWAF;
                }
            }

            aiq_free(pCamHw->_skipped_params);
            pCamHw->_skipped_params = NULL;
        }

        LOGK_CAMHW("<TB> Config id(%u)'s isp params, ens 0x%llx ens_up 0x%llx, cfg_up 0x%llx",
                   isp_params->frame_id, isp_params->module_ens, isp_params->module_en_update,
                   isp_params->module_cfg_update);
        return false;
    } else if (isp_params->frame_id == 0 && pCamHw->_not_skip_first) {
        pCamHw->_not_skip_first = false;
        return true;
    } else {
        return false;
    }
#else
    return false;
#endif
}

XCamReturn AiqCamHwIsp39_init(AiqCamHwBase_t* pCamHw, const char* sns_ent_name) {
    XCamReturn ret                    = AiqCamHwBase_init(pCamHw, sns_ent_name);
    pCamHw->mVicapIspPhyLinkSupported = true;

    pCamHw->updateEffParams = CamHwIsp39_updateEffParams;
    pCamHw->processTb       = CamHwIsp39_processTb;

    return ret;
}
