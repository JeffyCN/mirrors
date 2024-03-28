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
#include "RkAiqArgbirHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqArgbirHandleInt);

XCamReturn RkAiqArgbirHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "argbir handle prepare failed");

    RkAiqAlgoConfigArgbir* argbir_config_int    = (RkAiqAlgoConfigArgbir*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    argbir_config_int->rawHeight    = sharedCom->snsDes.isp_acq_height;
    argbir_config_int->rawWidth     = sharedCom->snsDes.isp_acq_width;
    argbir_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "argbir algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqArgbirHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigArgbir());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcArgbir());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResArgbir());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqArgbirHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
        mCurAttV10 = mNewAttV10;
        rk_aiq_uapi_argbir_v10_SetAttrib(mAlgoCtx, &mCurAttV10, true);
        updateAtt = false;
        sendSignal(mCurAttV10.sync.sync_mode);
#endif
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

#if RKAIQ_HAVE_RGBIR_REMOSAIC_V10
XCamReturn RkAiqArgbirHandleInt::setAttribV10(const RgbirAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_argbir_v10_SetAttrib(mAlgoCtx, att, true);
#else
    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC &&
        memcmp(&mNewAttV10, att, sizeof(RgbirAttrV10_t)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC &&
             memcmp(&mCurAttV10, att, sizeof(RgbirAttrV10_t)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAttV10 = *att;
        updateAtt  = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqArgbirHandleInt::getAttribV10(RgbirAttrV10_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_argbir_v10_GetAttrib(mAlgoCtx, att);
    att->sync.done = true;
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_argbir_v10_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAttV10, sizeof(RgbirAttrV10_t));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_argbir_v10_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAttV10.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn RkAiqArgbirHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqArgbirHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcArgbir* argbir_proc_int        = (RkAiqAlgoProcArgbir*)mProcInParam;
    RkAiqAlgoProcResArgbir* argbir_proc_res_int = (RkAiqAlgoProcResArgbir*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "argbir handle processing failed");
    }

    argbir_proc_res_int->ArgbirProcRes = &shared->fullParams->mRgbirParams->data()->result;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "argbir algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqArgbirHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    return ret;
}

XCamReturn RkAiqArgbirHandleInt::genIspResult(RkAiqFullParams* params,
                                              RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResArgbir* argbir_com          = (RkAiqAlgoProcResArgbir*)mProcOutParam;

    if (!argbir_com) {
        LOGD_ANALYZER("no argbir result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResArgbir* argbir_rk = (RkAiqAlgoProcResArgbir*)argbir_com;

        rk_aiq_isp_rgbir_params_t* rgbir_param = params->mRgbirParams->data().ptr();
        if (sharedCom->init) {
            rgbir_param->frame_id = 0;
        } else {
            rgbir_param->frame_id = shared->frameId;
        }

        if (argbir_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            rgbir_param->sync_flag = mSyncFlag;
            // copy from algo result
            cur_params->mRgbirParams = params->mRgbirParams;
            rgbir_param->is_update = true;
            LOGD_ARGBIR("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != rgbir_param->sync_flag) {
            rgbir_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mRgbirParams.ptr()) {
                rgbir_param->result = cur_params->mRgbirParams->data()->result;
                rgbir_param->is_update = true;
            } else {
                LOGE_ARGBIR("no latest params !");
                rgbir_param->is_update = false;
            }
            LOGD_ARGBIR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            rgbir_param->is_update = false;
            LOGD_ARGBIR("[%d] params needn't update", shared->frameId);
        }
        LOGD_ARGBIR(" %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
