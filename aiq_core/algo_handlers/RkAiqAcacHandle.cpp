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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    int groupId                                = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ACAC);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    int groupId                                = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ACAC);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    int groupId = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ACAC);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    RKAiqAecExpInfo_t* aeCurExp = &shared->curExp;
    if (aeCurExp != NULL) {
        if((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            acac_proc_int->iso = aeCurExp->LinearExp.exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:NORMAL:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->LinearExp.exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
            acac_proc_int->iso = aeCurExp->HdrExp[1].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR2:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[1].exp_real_params.analog_gain);
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
            acac_proc_int->iso = aeCurExp->HdrExp[2].exp_real_params.analog_gain * 50;
            LOGD_ACAC("%s:HDR3:iso=%d,again=%f\n", __FUNCTION__, acac_proc_int->iso,
                      aeCurExp->HdrExp[2].exp_real_params.analog_gain);
        }
    } else {
        acac_proc_int->iso = 50;
        LOGE_ACAC("%s: pAEPreRes is NULL, so use default instead \n", __FUNCTION__);
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
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;

    int groupId = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ACAC);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

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
        rk_aiq_uapi_acac_SetAttrib(mAlgoCtx, mCurAtt, false);
        sendSignal();
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::setAttrib(rk_aiq_cac_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    if (0 != memcmp(&mCurAtt, &att, sizeof(rk_aiq_cac_attrib_t))) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal();
    }
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAcacHandleInt::getAttrib(rk_aiq_cac_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_acac_GetAttrib(mAlgoCtx, att);

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
        memcpy(&cac_param->result.cfg[0], &cac_rk->config[0], sizeof(cac_rk->config[0]));
        memcpy(&cac_param->result.cfg[1], &cac_rk->config[1], sizeof(cac_rk->config[1]));
    }

    cur_params->mCacV3xParams = params->mCacV3xParams;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
