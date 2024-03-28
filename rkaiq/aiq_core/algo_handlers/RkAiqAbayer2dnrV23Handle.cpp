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
#include "RkAiqAbayer2dnrV23Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAbayer2dnrV23HandleInt);

void RkAiqAbayer2dnrV23HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAbayer2dnrV23());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAbayer2dnrV23());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAbayer2dnrV23());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAbayer2dnrV23HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_abayer2dnrV23_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updateStrength) {
        mCurStrength   = mNewStrength;
        rk_aiq_uapi_abayer2dnrV23_SetStrength(mAlgoCtx, &mCurStrength);
        sendSignal(mCurStrength.sync.sync_mode);
        updateStrength = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayer2dnrV23HandleInt::setAttrib(const rk_aiq_bayer2dnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayer2dnrV23_SetAttrib(mAlgoCtx, att, false);
#else
    // if something changed
    if (0 != memcmp(&mCurAtt, att, sizeof(rk_aiq_bayer2dnr_attrib_v23_t))) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayer2dnrV23HandleInt::getAttrib(rk_aiq_bayer2dnr_attrib_v23_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_abayer2dnrV23_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayer2dnrV23_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            mCfgMutex.unlock();
            att->sync.done = false;
        } else {
            mCfgMutex.unlock();
            rk_aiq_uapi_abayer2dnrV23_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif


    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAbayer2dnrV23HandleInt::setStrength(const rk_aiq_bayer2dnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_abayer2dnrV23_SetStrength(mAlgoCtx, pStrength);
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

XCamReturn RkAiqAbayer2dnrV23HandleInt::getStrength(rk_aiq_bayer2dnr_strength_v23_t *pStrength) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
        mCfgMutex.lock();
        ret = rk_aiq_uapi_abayer2dnrV23_GetStrength(mAlgoCtx, pStrength);
        mCfgMutex.unlock();
#else
    if(pStrength->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayer2dnrV23_GetStrength(mAlgoCtx, pStrength);
        pStrength->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateStrength) {
            pStrength->percent = mNewStrength.percent;
            pStrength->sync.done = false;
        } else {
            rk_aiq_uapi_abayer2dnrV23_GetStrength(mAlgoCtx, pStrength);
            pStrength->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAbayer2dnrV23HandleInt::getInfo(rk_aiq_bayer2dnr_info_v23_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_abayer2dnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_abayer2dnrV23_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqAbayer2dnrV23HandleInt::prepare() {
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

XCamReturn RkAiqAbayer2dnrV23HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0

    RkAiqAlgoPreAbayer2dnrV23* arawnr_pre_int = (RkAiqAlgoPreAbayer2dnrV23*)mPreInParam;
    RkAiqAlgoPreResAbayer2dnrV23* arawnr_pre_res_int =
        (RkAiqAlgoPreResAbayer2dnrV23*)mPreOutParam;
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

XCamReturn RkAiqAbayer2dnrV23HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAbayer2dnrV23* arawnr_proc_int = (RkAiqAlgoProcAbayer2dnrV23*)mProcInParam;
    RkAiqAlgoProcResAbayer2dnrV23* arawnr_proc_res_int =
            (RkAiqAlgoProcResAbayer2dnrV23*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    arawnr_proc_res_int->stArawnrProcResult.st2DFix = &shared->fullParams->mBaynrParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aynr handle processing failed");
    }

    // TODO: fill procParam
    arawnr_proc_int->iso      = sharedCom->iso;
    arawnr_proc_int->hdr_mode = sharedCom->working_mode;

    arawnr_proc_int->stAblcV32_proc_res = shared->res_comb.ablcV32_proc_res;
    arawnr_proc_int->bayertnr_en = shared->res_comb.bayernr3d_en;

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

XCamReturn RkAiqAbayer2dnrV23HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAbayer2dnrV23* arawnr_post_int = (RkAiqAlgoPostAbayer2dnrV23*)mPostInParam;
    RkAiqAlgoPostResAbayer2dnrV23* arawnr_post_res_int =
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

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAbayer2dnrV23HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAbayer2dnrV23* arawnr_rk = (RkAiqAlgoProcResAbayer2dnrV23*)mProcOutParam;

    if (!arawnr_rk) {
        LOGD_ANALYZER("no arawnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ANR("oyyf: %s:%d output isp param start\n", __FUNCTION__, __LINE__);
        rk_aiq_isp_baynr_params_t* rawnr_param = params->mBaynrParams->data().ptr();
        if (sharedCom->init) {
            rawnr_param->frame_id = 0;
        } else {
            rawnr_param->frame_id = shared->frameId;
        }

        if (arawnr_rk->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            rawnr_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mBaynrParams = params->mBaynrParams;
            rawnr_param->is_update = true;
            LOGD_ANR("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != rawnr_param->sync_flag) {
            rawnr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mBaynrParams.ptr()) {
                rawnr_param->result = cur_params->mBaynrParams->data()->result;
                rawnr_param->is_update = true;
            } else {
                LOGE_ANR("no latest params !");
                rawnr_param->is_update = false;
            }
            LOGD_ANR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            rawnr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
        LOGD_ANR("oyyf: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
