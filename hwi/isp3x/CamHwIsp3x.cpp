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
    xcam_mem_clear (_full_active_isp3x_params);

    XCamReturn ret = CamHwIsp21::init(sns_ent_name);

    return ret;
}

XCamReturn
CamHwIsp3x::stop()
{

    XCamReturn ret = CamHwIsp21::stop();
    xcam_mem_clear (_full_active_isp3x_params);

    return ret;
}

void
CamHwIsp3x::gen_full_isp_params(const struct isp3x_isp_params_cfg* update_params,
                                struct isp3x_isp_params_cfg* full_params,
                                uint64_t* module_en_update_partial,
                                uint64_t* module_cfg_update_partial)
{
    XCAM_ASSERT (update_params);
    XCAM_ASSERT (full_params);
    int i = 0;

    ENTER_CAMHW_FUNCTION();
    for (; i <= RK_ISP2X_MAX_ID; i++)
        if (update_params->module_en_update & (1LL << i)) {
            if ((full_params->module_ens & (1LL << i)) !=
                    (update_params->module_ens & (1LL << i)))
                *module_en_update_partial |= 1LL << i;
            full_params->module_en_update |= 1LL << i;
            // clear old bit value
            full_params->module_ens &= ~(1LL << i);
            // set new bit value
            full_params->module_ens |= update_params->module_ens & (1LL << i);
        }

    for (i = 0; i <= RK_ISP2X_MAX_ID; i++) {
        if (update_params->module_cfg_update & (1LL << i)) {

#define CHECK_UPDATE_PARAMS(dst, src) \
            if (memcmp(&dst, &src, sizeof(dst)) == 0) { \
                continue; \
            } \
            *module_cfg_update_partial |= 1LL << i; \
            dst = src; \

            full_params->module_cfg_update |= 1LL << i;
            switch (i) {
            case RK_ISP2X_RAWAE0_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawae0, update_params->meas.rawae0);
                break;
            case RK_ISP2X_RAWAE1_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawae1, update_params->meas.rawae1);
                break;
            case RK_ISP2X_RAWAE2_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawae2, update_params->meas.rawae2);
                break;
            case RK_ISP2X_RAWAE3_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawae3, update_params->meas.rawae3);
                break;
            case RK_ISP2X_RAWHIST0_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawhist0, update_params->meas.rawhist0);
                break;
            case RK_ISP2X_RAWHIST1_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawhist1, update_params->meas.rawhist1);
                break;
            case RK_ISP2X_RAWHIST2_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawhist2, update_params->meas.rawhist2);
                break;
            case RK_ISP2X_RAWHIST3_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawhist3, update_params->meas.rawhist3);
                break;
            case RK_ISP2X_YUVAE_ID:
                //CHECK_UPDATE_PARAMS(full_params->meas.yuvae, update_params->meas.yuvae);
                break;
            case RK_ISP2X_SIHST_ID:
                //CHECK_UPDATE_PARAMS(full_params->meas.sihst, update_params->meas.sihst);
                break;
            case RK_ISP2X_SIAWB_ID:
                //CHECK_UPDATE_PARAMS(full_params->meas.siawb, update_params->meas.siawb);
                break;
            case RK_ISP2X_RAWAWB_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawawb, update_params->meas.rawawb);
                break;
            case RK_ISP2X_AWB_GAIN_ID:
                CHECK_UPDATE_PARAMS(full_params->others.awb_gain_cfg, update_params->others.awb_gain_cfg);
                break;
            case RK_ISP2X_RAWAF_ID:
                CHECK_UPDATE_PARAMS(full_params->meas.rawaf, update_params->meas.rawaf);
                break;
            case RK_ISP2X_HDRMGE_ID:
                CHECK_UPDATE_PARAMS(full_params->others.hdrmge_cfg, update_params->others.hdrmge_cfg);
                break;
            /* case RK_ISP2X_HDRTMO_ID: */
            /*     full_params->others.hdrtmo_cfg = update_params->others.hdrtmo_cfg; */
            /*     break; */
            case RK_ISP2X_CTK_ID:
                CHECK_UPDATE_PARAMS(full_params->others.ccm_cfg, update_params->others.ccm_cfg);
                break;
            case RK_ISP2X_LSC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.lsc_cfg, update_params->others.lsc_cfg);
                break;
            case RK_ISP2X_GOC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.gammaout_cfg, update_params->others.gammaout_cfg);
                break;
            case RK_ISP2X_3DLUT_ID:
                CHECK_UPDATE_PARAMS(full_params->others.isp3dlut_cfg, update_params->others.isp3dlut_cfg);
                break;
            case RK_ISP2X_DPCC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.dpcc_cfg, update_params->others.dpcc_cfg);
                break;
            case RK_ISP2X_BLS_ID:
                CHECK_UPDATE_PARAMS(full_params->others.bls_cfg, update_params->others.bls_cfg);
                break;
            case RK_ISP2X_DEBAYER_ID:
                CHECK_UPDATE_PARAMS(full_params->others.debayer_cfg, update_params->others.debayer_cfg);
                break;
            case RK_ISP2X_DHAZ_ID:
                CHECK_UPDATE_PARAMS(full_params->others.dhaz_cfg, update_params->others.dhaz_cfg);
                break;
            /* case RK_ISP2X_RAWNR_ID: */
            /*     full_params->others.rawnr_cfg = update_params->others.rawnr_cfg; */
            /*     break; */
            case RK_ISP2X_GAIN_ID:
                CHECK_UPDATE_PARAMS(full_params->others.gain_cfg, update_params->others.gain_cfg);
                break;
            case RK_ISP2X_LDCH_ID:
                CHECK_UPDATE_PARAMS(full_params->others.ldch_cfg, update_params->others.ldch_cfg);
                break;
            case RK_ISP2X_GIC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.gic_cfg, update_params->others.gic_cfg);
                break;
            case RK_ISP2X_CPROC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.cproc_cfg, update_params->others.cproc_cfg);
                break;
            case Rk_ISP21_BAYNR_ID:
                CHECK_UPDATE_PARAMS(full_params->others.baynr_cfg, update_params->others.baynr_cfg);
                break;
            case Rk_ISP21_BAY3D_ID:
                CHECK_UPDATE_PARAMS(full_params->others.bay3d_cfg, update_params->others.bay3d_cfg);
                break;
            case Rk_ISP21_YNR_ID:
                CHECK_UPDATE_PARAMS(full_params->others.ynr_cfg, update_params->others.ynr_cfg);
                break;
            case Rk_ISP21_CNR_ID:
                CHECK_UPDATE_PARAMS(full_params->others.cnr_cfg, update_params->others.cnr_cfg);
                break;
            case Rk_ISP21_SHARP_ID:
                CHECK_UPDATE_PARAMS(full_params->others.sharp_cfg, update_params->others.sharp_cfg);
                break;
            case Rk_ISP21_DRC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.drc_cfg, update_params->others.drc_cfg);
                break;
            case RK_ISP2X_SDG_ID:
                CHECK_UPDATE_PARAMS(full_params->others.sdg_cfg, update_params->others.sdg_cfg);
                break;
            case Rk_ISP3x_CAC_ID:
                CHECK_UPDATE_PARAMS(full_params->others.cac_cfg, update_params->others.cac_cfg);
                break;
            case Rk_ISP2x_CSM_ID:
                CHECK_UPDATE_PARAMS(full_params->others.csm_cfg, update_params->others.csm_cfg);
                break;
            default:
                break;
            }
        }
    }
    EXIT_CAMHW_FUNCTION();
}

XCamReturn
CamHwIsp3x::setIspConfig()
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    SmartPtr<V4l2Buffer> v4l2buf;
    uint32_t frameId = -1;

    {
        SmartLock locker (_isp_params_cfg_mutex);
        while (_effecting_ispparam_map.size() > 4)
            _effecting_ispparam_map.erase(_effecting_ispparam_map.begin());
    }
    if (mIspParamsDev.ptr()) {
        ret = mIspParamsDev->get_buffer(v4l2buf);
        if (ret) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "Can not get isp params buffer\n");
            return XCAM_RETURN_ERROR_PARAM;
        }
    } else
        return XCAM_RETURN_BYPASS;

    cam3aResultList ready_results;
    ret = mParamsAssembler->deQueOne(ready_results, frameId);
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGI_CAMHW_SUBM(ISP20HW_SUBM, "deque isp ready parameter failed\n");
        mIspParamsDev->return_buffer_to_pool (v4l2buf);
        return XCAM_RETURN_ERROR_PARAM;
    }

    LOGD_ANALYZER("----------%s, start config id(%d)'s isp params", __FUNCTION__, frameId);

    struct isp3x_isp_params_cfg update_params[2];
    memset(update_params, 0, sizeof(struct isp3x_isp_params_cfg)*2);

    update_params[0].module_en_update = 0;
    update_params[0].module_ens = 0;
    update_params[0].module_cfg_update = 0;
    if (_state == CAM_HW_STATE_STOPPED || _state == CAM_HW_STATE_PREPARED || _state == CAM_HW_STATE_PAUSED) {
        // update all ens
        _full_active_isp3x_params.module_en_update = ~0;
        // just re-config the enabled moddules
        _full_active_isp3x_params.module_cfg_update = _full_active_isp3x_params.module_ens;
    } else {
        _full_active_isp3x_params.module_en_update = 0;
        // use module_ens to store module status, so we can use it to restore
        // the init params for re-start and re-prepare
        /* _full_active_isp3x_params.module_ens = 0; */
        _full_active_isp3x_params.module_cfg_update = 0;
    }

    if (frameId >= 0) {
        SmartPtr<cam3aResult> awb_res = get_3a_module_result(ready_results, RESULT_TYPE_AWB_PARAM);
        SmartPtr<RkAiqIspAwbParamsProxyV3x> awbParams;
        if (awb_res.ptr()) {
            awbParams = awb_res.dynamic_cast_ptr<RkAiqIspAwbParamsProxyV3x>();
            {
                SmartLock locker (_isp_params_cfg_mutex);
                _effecting_ispparam_map[frameId].awb_cfg_v3x = awbParams->data()->result;

            }
        } else {
            /* use the latest */
            SmartLock locker (_isp_params_cfg_mutex);
            if (!_effecting_ispparam_map.empty()) {
                _effecting_ispparam_map[frameId].awb_cfg_v3x = (_effecting_ispparam_map.rbegin())->second.awb_cfg_v3x;
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "use frame %d awb params for frame %d !\n",
                                frameId, (_effecting_ispparam_map.rbegin())->first);
            } else {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get awb params from 3a result failed for frame %d !\n", frameId);
            }
        }

        SmartPtr<cam3aResult> blc_res = get_3a_module_result(ready_results, RESULT_TYPE_BLC_PARAM);
        SmartPtr<RkAiqIspBlcParamsProxyV21> blcParams;
        if (blc_res.ptr()) {
            blcParams = blc_res.dynamic_cast_ptr<RkAiqIspBlcParamsProxyV21>();
            {
                SmartLock locker (_isp_params_cfg_mutex);
                _effecting_ispparam_map[frameId].blc_cfg = blcParams->data()->result;

            }
        } else {
            /* use the latest */
            SmartLock locker (_isp_params_cfg_mutex);
            if (!_effecting_ispparam_map.empty()) {
                _effecting_ispparam_map[frameId].blc_cfg = (_effecting_ispparam_map.rbegin())->second.blc_cfg;
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "use frame %d blc params for frame %d !\n",
                                frameId, (_effecting_ispparam_map.rbegin())->first);
            } else {
                LOGW_CAMHW_SUBM(ISP20HW_SUBM, "get blc params from 3a result failed for frame %d !\n", frameId);
            }
        }

    }

    // TODO: merge_isp_results would cause the compile warning: reference to merge_isp_results is ambiguous
    // now use Isp21Params::merge_isp_results instead
    if (Isp3xParams::merge_isp_results(ready_results, update_params) != XCAM_RETURN_NO_ERROR)
        LOGE_CAMHW_SUBM(ISP20HW_SUBM, "ISP parameter translation error\n");
    if(mIsMultiIspMode == false) {
        Isp3xParams::fixedAwbOveflowToIsp3xParams(update_params, mIsMultiIspMode);
    }
    uint64_t module_en_update_partial = 0;
    uint64_t module_cfg_update_partial = 0;
    gen_full_isp_params(update_params, &_full_active_isp3x_params,
                        &module_en_update_partial, &module_cfg_update_partial);

    if (_state == CAM_HW_STATE_STOPPED)
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "ispparam ens 0x%llx, en_up 0x%llx, cfg_up 0x%llx",
                        _full_active_isp3x_params.module_ens,
                        _full_active_isp3x_params.module_en_update,
                        _full_active_isp3x_params.module_cfg_update);
#ifdef RUNTIME_MODULE_DEBUG
    _full_active_isp3x_params.module_en_update &= ~g_disable_isp_modules_en;
    _full_active_isp3x_params.module_ens |= g_disable_isp_modules_en;
    _full_active_isp3x_params.module_cfg_update &= ~g_disable_isp_modules_cfg_update;
    module_en_update_partial = _full_active_isp3x_params.module_en_update;
    module_cfg_update_partial = _full_active_isp3x_params.module_cfg_update;
#endif


    if (v4l2buf.ptr()) {
        struct isp3x_isp_params_cfg* isp_params;
        int buf_index = v4l2buf->get_buf().index;
        bool isMultiIsp = mIsMultiIspMode;
        bool extened_pixel = mMultiIspExtendedPixel;

        isp_params = (struct isp3x_isp_params_cfg*)v4l2buf->get_buf().m.userptr;
        *isp_params = _full_active_isp3x_params;
        isp_params->module_en_update = module_en_update_partial;
        isp_params->module_cfg_update = module_cfg_update_partial;
        //TODO: isp driver has bug now, lsc cfg_up should be set along with
        //en_up
        if (isp_params->module_cfg_update & ISP2X_MODULE_LSC)
            isp_params->module_en_update |= ISP2X_MODULE_LSC;
        isp_params->frame_id = frameId;

#if 0 //TODO: isp21 params has no exposure info field
        SmartPtr<SensorHw> mSensorSubdev = mSensorDev.dynamic_cast_ptr<SensorHw>();
        if (mSensorSubdev.ptr()) {
            memset(&isp_params->exposure, 0, sizeof(isp_params->exposure));
            SmartPtr<RkAiqExpParamsProxy> expParam;

            if (mSensorSubdev->getEffectiveExpParams(expParam, frameId) < 0) {
                LOGE_CAMHW_SUBM(ISP20HW_SUBM, "frame_id(%d), get exposure failed!!!\n", frameId);
            } else {
                if (RK_AIQ_HDR_GET_WORKING_MODE(_hdr_mode) == RK_AIQ_WORKING_MODE_NORMAL) {
                    isp_params->exposure.linear_exp.analog_gain_code_global = \
                            expParam->data()->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global;
                    isp_params->exposure.linear_exp.coarse_integration_time = \
                            expParam->data()->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time;
                } else {
                    isp_params->exposure.hdr_exp[0].analog_gain_code_global = \
                            expParam->data()->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global;
                    isp_params->exposure.hdr_exp[0].coarse_integration_time = \
                            expParam->data()->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time;
                    isp_params->exposure.hdr_exp[1].analog_gain_code_global = \
                            expParam->data()->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global;
                    isp_params->exposure.hdr_exp[1].coarse_integration_time = \
                            expParam->data()->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time;
                    isp_params->exposure.hdr_exp[2].analog_gain_code_global = \
                            expParam->data()->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global;
                    isp_params->exposure.hdr_exp[2].coarse_integration_time = \
                            expParam->data()->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time;
                }
            }
        }
#endif

        if (isMultiIsp) {
            mParamsSplitter->SplitIspParams(&_full_active_isp3x_params, isp_params);
            Isp3xParams::fixedAwbOveflowToIsp3xParams((void*)isp_params, mIsMultiIspMode);
            memcpy(&((isp_params + 1)->others.cac_cfg), &update_params[1].others.cac_cfg,
                   sizeof(struct isp3x_cac_cfg));
        }

        {
            SmartLock locker(_isp_params_cfg_mutex);
            if (frameId < 0) {
                _effecting_ispparam_map[0].isp_params_v3x[0] = _full_active_isp3x_params;
            } else {
                _effecting_ispparam_map[frameId].isp_params_v3x[0] = _full_active_isp3x_params;
            }
            if (isMultiIsp) {
                if (frameId < 0) {
                    _effecting_ispparam_map[0].isp_params_v3x[1] = isp_params[0];
                    _effecting_ispparam_map[0].isp_params_v3x[2] = isp_params[1];
                } else {
                    _effecting_ispparam_map[frameId].isp_params_v3x[1] = isp_params[0];
                    _effecting_ispparam_map[frameId].isp_params_v3x[2] = isp_params[1];
                }
            }
        }
        if (mIspParamsDev->queue_buffer (v4l2buf) != 0) {
            LOGE_CAMHW_SUBM(ISP20HW_SUBM, "RKISP1: failed to ioctl VIDIOC_QBUF for index %d, %d %s.\n",
                            buf_index, errno, strerror(errno));
            mIspParamsDev->return_buffer_to_pool (v4l2buf);
            return XCAM_RETURN_ERROR_IOCTL;
        }

        ispModuleEns = _full_active_isp3x_params.module_ens;
        LOGD_CAMHW_SUBM(ISP20HW_SUBM, "camId:%d, frameId:%d, ispparam ens 0x%llx, en_up 0x%llx, cfg_up 0x%llx",
                         mCamPhyId, frameId,
                        _full_active_isp3x_params.module_ens,
                        isp_params->module_en_update,
                        isp_params->module_cfg_update);

        LOG1_CAMHW_SUBM(ISP20HW_SUBM, "device(%s) queue buffer index %d, queue cnt %d, check exit status again[exit: %d]",
                        XCAM_STR (mIspParamsDev->get_device_name()),
                        buf_index, mIspParamsDev->get_queued_bufcnt(), _is_exit);
        if (_is_exit)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    EXIT_CAMHW_FUNCTION();
    return ret;
}

};
