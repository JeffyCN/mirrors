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
#include "RkAiqAbayertnrV2Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAbayertnrV2HandleInt);

void RkAiqAbayertnrV2HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAbayertnrV2());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAbayertnrV2());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAbayertnrV2());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAbayertnrV2());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAbayertnrV2());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAbayertnrV2());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAbayertnrV2());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAbayertnrV2HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_abayertnrV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_abayertnrV2_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::setAttrib(rk_aiq_bayertnr_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV2_SetAttrib(mAlgoCtx, att, false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::getAttrib(rk_aiq_bayertnr_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_abayertnrV2_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            mCfgMutex.unlock();
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapi_abayertnrV2_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}



XCamReturn RkAiqAbayertnrV2HandleInt::setStrength(rk_aiq_bayertnr_strength_v2_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV2_SetStrength(mAlgoCtx, pStrength);
#else
    bool isChanged = false;
    if (pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
            memcmp(&mNewStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;
    else if (pStrength->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurStrength, pStrength, sizeof(*pStrength)))
        isChanged = true;

    if (isChanged) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }
#endif


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::getStrength(rk_aiq_bayertnr_strength_v2_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_abayertnrV2_GetStrength(mAlgoCtx, pStrength);
    mCfgMutex.unlock();
#else
    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV2_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            *pStrength = mNewStrength;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_abayertnrV2_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAbayertnrV2HandleInt::getInfo(rk_aiq_bayertnr_info_v2_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV2_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_abayertnrV2_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "arawnr handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAbayertnrV2HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "arawnr handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo pre_process failed");
    // set result to mAiqCore

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAbayertnrV2HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAbayertnrV2* abayertnr_proc_int = (RkAiqAlgoProcAbayertnrV2*)mProcInParam;
    RkAiqAlgoProcResAbayertnrV2* abayertnr_proc_res_int = (RkAiqAlgoProcResAbayertnrV2*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    abayertnr_proc_res_int->stAbayertnrProcResult.st3DFix = &shared->fullParams->mTnrParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    // TODO: fill procParam
    abayertnr_proc_int->iso      = sharedCom->iso;
    abayertnr_proc_int->hdr_mode = sharedCom->working_mode;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "arawnr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo post_process failed");
    // set result to mAiqCore

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV2HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAbayertnrV2* atnr_rk = (RkAiqAlgoProcResAbayertnrV2*)mProcOutParam;

    if (!atnr_rk) {
        LOGD_ANALYZER("no abayertnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_tnr_params_t* tnr_param = params->mTnrParams->data().ptr();
        if (sharedCom->init) {
            tnr_param->frame_id = 0;
        } else {
            tnr_param->frame_id = shared->frameId;
        }

        if (atnr_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            tnr_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mTnrParams = params->mTnrParams;
            tnr_param->is_update = true;
            LOGD_ANR("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != tnr_param->sync_flag) {
            tnr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mTnrParams.ptr()) {
                tnr_param->result = cur_params->mTnrParams->data()->result;
                tnr_param->is_update = true;
            } else {
                LOGE_ANR("no latest params !");
                tnr_param->is_update = false;
            }
            LOGD_ANR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            tnr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
