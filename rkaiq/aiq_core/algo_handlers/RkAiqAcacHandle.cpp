/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqAcacHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAcacHandleInt);

XCamReturn RkAiqAcacHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "acac handle prepare failed");

    RkAiqAlgoConfigAcac* acac_config_int = (RkAiqAlgoConfigAcac*)mConfig;
    RkAiqAlgoDescription* des               = (RkAiqAlgoDescription*)mDes;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    acac_config_int->mem_ops = mAiqCore->mShareMemOps;
    acac_config_int->width = sharedCom->snsDes.isp_acq_width;
    acac_config_int->height = sharedCom->snsDes.isp_acq_height;
    acac_config_int->is_multi_sensor = sharedCom->is_multi_sensor;
    acac_config_int->is_multi_isp = sharedCom->is_multi_isp_mode;
    acac_config_int->multi_isp_extended_pixel = sharedCom->multi_isp_extended_pixels;

    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "acac algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAcacHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAcac());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAcac());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAcac());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAcac());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAcac());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAcac());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAcac());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAcacHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAcac* acac_pre_int          = (RkAiqAlgoPreAcac*)mPreInParam;
    RkAiqAlgoPreResAcac* acac_pre_res_int   = (RkAiqAlgoPreResAcac*)mPreOutParam;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acac handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "acac algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAcacHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAcac* acac_proc_int        = (RkAiqAlgoProcAcac*)mProcInParam;
    RkAiqAlgoProcResAcac* acac_proc_res_int = (RkAiqAlgoProcResAcac*)mProcOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    if (aeCurExp != NULL) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            acac_proc_int->hdr_ratio = 1;
            acac_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            acac_proc_int->hdr_ratio = (aeCurExp->HdrExp[1].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[1].exp_real_params.integration_time) /
                                       (aeCurExp->HdrExp[0].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[0].exp_real_params.integration_time);
            acac_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            acac_proc_int->hdr_ratio = (aeCurExp->HdrExp[2].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[2].exp_real_params.integration_time) /
                                       (aeCurExp->HdrExp[0].exp_real_params.analog_gain *
                                        aeCurExp->HdrExp[0].exp_real_params.integration_time);
            acac_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        acac_proc_int->iso = 50;
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
    }

    acac_proc_int->hdr_mode = sharedCom->working_mode;
    switch (sharedCom->snsDes.sensor_pixelformat) {
        case V4L2_PIX_FMT_SBGGR14:
        case V4L2_PIX_FMT_SGBRG14:
        case V4L2_PIX_FMT_SGRBG14:
        case V4L2_PIX_FMT_SRGGB14:
            acac_proc_int->raw_bits = 14;
            break;
        case V4L2_PIX_FMT_SBGGR12:
        case V4L2_PIX_FMT_SGBRG12:
        case V4L2_PIX_FMT_SGRBG12:
        case V4L2_PIX_FMT_SRGGB12:
            acac_proc_int->raw_bits = 12;
            break;
        case V4L2_PIX_FMT_SBGGR10:
        case V4L2_PIX_FMT_SGBRG10:
        case V4L2_PIX_FMT_SGRBG10:
        case V4L2_PIX_FMT_SRGGB10:
            acac_proc_int->raw_bits = 10;
            break;
        default:
            acac_proc_int->raw_bits = 8;
    }

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acac handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "acac algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAcac* acac_post_int        = (RkAiqAlgoPostAcac*)mPostInParam;
    RkAiqAlgoPostResAcac* acac_post_res_int = (RkAiqAlgoPostResAcac*)mPostOutParam;
    auto* shared = (RkAiqCore::RkAiqAlgosGroupShared_t*)getGroupShared();
    if (!shared) return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "acac handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "acac algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_acac_v10_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal();
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::setAttrib(const rkaiq_cac_v10_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    if (att == nullptr) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::getAttrib(rkaiq_cac_v10_api_attr_t* att) {
    ENTER_ANALYZER_FUNCTION();

    if (att == nullptr) return XCAM_RETURN_ERROR_PARAM;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_acac_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_acac_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAcac* cac_com               = (RkAiqAlgoProcResAcac*)mProcOutParam;

    rk_aiq_isp_cac_params_v3x_t* cac_param = params->mCacV3xParams->data().ptr();

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAcac* cac_rk = (RkAiqAlgoProcResAcac*)cac_com;
        if (sharedCom->init) {
            cac_param->frame_id = 0;
        } else {
            cac_param->frame_id = shared->frameId;
        }
        cac_param->result.enable = cac_rk->enable;
        memcpy(&cac_param->result.cfg[0], &cac_rk->config[0], sizeof(cac_rk->config[0]));
        memcpy(&cac_param->result.cfg[1], &cac_rk->config[1], sizeof(cac_rk->config[1]));
    }

    cur_params->mCacV3xParams = params->mCacV3xParams;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
