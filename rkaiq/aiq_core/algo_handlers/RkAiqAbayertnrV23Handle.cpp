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
#include "RkAiqAbayertnrV23Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAbayertnrV23HandleInt);

void RkAiqAbayertnrV23HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAbayertnrV23());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAbayertnrV23());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAbayertnrV23());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAbayertnrV23HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_abayertnrV23_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateAttLite) {
        mCurAttLite = mNewAttLite;
        rk_aiq_uapi_abayertnrV23Lite_SetAttrib(mAlgoCtx, &mCurAttLite, false);
        sendSignal(mCurAttLite.sync.sync_mode);
        updateAttLite = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_abayertnrV23_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::setAttrib(const rk_aiq_bayertnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV23_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayertnr_attrib_v23_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::getAttrib(rk_aiq_bayertnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_abayertnrV23_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV23_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            mCfgMutex.unlock();
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapi_abayertnrV23_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::setAttribLite(const rk_aiq_bayertnr_attrib_v23L_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV23Lite_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAttLite, att, sizeof(rk_aiq_bayertnr_attrib_v23L_t))) {
        mNewAttLite   = *att;
        updateAttLite = true;
        waitSignal(att->sync.sync_mode);
    }
#endif
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::getAttribLite(rk_aiq_bayertnr_attrib_v23L_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_abayertnrV23Lite_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV23Lite_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAttLite) {
            memcpy(att, &mNewAttLite, sizeof(mNewAttLite));
            mCfgMutex.unlock();
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapi_abayertnrV23Lite_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::setStrength(const rk_aiq_bayertnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayertnrV23_SetStrength(mAlgoCtx, pStrength);
#else

    if (0 != memcmp(&mCurStrength, pStrength, sizeof(mCurStrength))) {
        mNewStrength   = *pStrength;
        updateStrength = true;
        waitSignal(pStrength->sync.sync_mode);
    }
#endif


    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::getStrength(rk_aiq_bayertnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.lock();
        ret = rk_aiq_uapi_abayertnrV23_GetStrength(mAlgoCtx, pStrength);
        mCfgMutex.unlock();
#else
    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV23_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            pStrength->percent = mNewStrength.percent;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_abayertnrV23_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::getInfo(rk_aiq_bayertnr_info_v23_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayertnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_abayertnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAbayertnrV23HandleInt::prepare() {
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

XCamReturn RkAiqAbayertnrV23HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAbayertnrV23* abayertnr_pre_int = (RkAiqAlgoPreAbayertnrV23*)mPreInParam;
    RkAiqAlgoPreResAbayertnrV23* abayertnr_pre_res_int =
        (RkAiqAlgoPreResAbayertnrV23*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "arawnr handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "arawnr algo pre_process failed");
    // set result to mAiqCore

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAbayertnrV23HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAbayertnrV23* abayertnr_proc_int = (RkAiqAlgoProcAbayertnrV23*)mProcInParam;
    RkAiqAlgoProcResAbayertnrV23* abayertnr_proc_res_int =
        (RkAiqAlgoProcResAbayertnrV23*)mProcOutParam;
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
    abayertnr_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "aynr algo processing failed");

    if (!abayertnr_proc_res_int->res_com.cfg_update) {
        shared->res_comb.bayernr3d_en = mLatestEn;
    } else {
        shared->res_comb.bayernr3d_en = mLatestEn = !abayertnr_proc_res_int->stAbayertnrProcResult.st3DFix->bay3d_en ? false : true;
    }
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAbayer2dnrV23* abayertnr_post_int = (RkAiqAlgoPostAbayer2dnrV23*)mPostInParam;
    RkAiqAlgoPostResAbayer2dnrV23* abayertnr_post_res_int =
        (RkAiqAlgoPostResAbayer2dnrV23*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

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
#endif
    return ret;
}

XCamReturn RkAiqAbayertnrV23HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAbayertnrV23* atnr_rk = (RkAiqAlgoProcResAbayertnrV23*)mProcOutParam;

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
            LOGD_ANR("3d [%d] params from algo", mSyncFlag);
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
            LOGD_ANR("3d [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            tnr_param->is_update = false;
            LOGD_ANR("3d [%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
