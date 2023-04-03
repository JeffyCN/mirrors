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
#include "RkAiqAblcHandle.h"

#include "RkAiqCore.h"

extern XCamReturn
rk_aiq_uapi_ablc_GetProc(const RkAiqAlgoContext *ctx,
                         AblcProc_t *ProcRes);

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAblcHandleInt);

void RkAiqAblcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAblc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAblc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAblc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAblcHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_ablc_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcHandleInt::setAttrib(const rk_aiq_blc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
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
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcHandleInt::getAttrib(rk_aiq_blc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ablc_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_ablc_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAblcHandleInt::getProcRes(AblcProc_t *ProcRes) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ablc_GetProc(mAlgoCtx, ProcRes);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcHandleInt::getInfo(rk_aiq_ablc_info_t *pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ablc_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_ablc_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;

    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ablc handle prepare failed");

    RkAiqAlgoConfigAblc* ablc_config_int = (RkAiqAlgoConfigAblc*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ablc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAblcHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAblc* ablc_pre_int        = (RkAiqAlgoPreAblc*)mPreInParam;
    RkAiqAlgoPreResAblc* ablc_pre_res_int = (RkAiqAlgoPreResAblc*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ablc handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablc algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAblcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAblc* ablc_proc_int        = (RkAiqAlgoProcAblc*)mProcInParam;

    if (mDes->id == 0) {
        mProcResShared = new RkAiqAlgoProcResAblcIntShared();
        if (!mProcResShared.ptr()) {
            LOGE("new BLC mProcResShared failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("ablc handle processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mProcResShared = NULL;
        return ret;
    }

    ablc_proc_int->iso      = sharedCom->iso;
    ablc_proc_int->hdr_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, (RkAiqAlgoResCom*)(&mProcResShared->result));
    if (ret < 0) {
        LOGE_ANALYZER("ablc algo processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mProcResShared = NULL;
        return ret;
    }


    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* grpShared = nullptr;
        uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AWB);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                memcpy(&grpShared->res_comb.ablc_proc_res, &mProcResShared->result.ablc_proc_res,
                       sizeof(mProcResShared->result.ablc_proc_res));
        }
        grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP0);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                memcpy(&grpShared->res_comb.ablc_proc_res, &mProcResShared->result.ablc_proc_res,
                       sizeof(mProcResShared->result.ablc_proc_res));
        }
    } else if (mPostShared) {
        SmartPtr<BufferProxy> msg_data = new BufferProxy(mProcResShared);
        msg_data->set_sequence(shared->frameId);
        SmartPtr<XCamMessage> msg =
            new RkAiqCoreVdBufMsg(XCAM_MESSAGE_BLC_PROC_RES_OK, shared->frameId, msg_data);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAblc* ablc_post_int        = (RkAiqAlgoPostAblc*)mPostInParam;
    RkAiqAlgoPostResAblc* ablc_post_res_int = (RkAiqAlgoPostResAblc*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ablc handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablc algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAblcHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    if (!mProcResShared.ptr())
        return XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAblc* ablc_com             = (RkAiqAlgoProcResAblc*)&mProcResShared->result;
#if defined(ISP_HW_V21) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    rk_aiq_isp_blc_params_v21_t* blc_param = params->mBlcV21Params->data().ptr();
#else
    rk_aiq_isp_blc_params_v20_t* blc_param = params->mBlcParams->data().ptr();
#endif


    if (!ablc_com) {
        LOGD_ANALYZER("no ablc result");
        mProcResShared = NULL;
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAblc* ablc_rk = (RkAiqAlgoProcResAblc*)ablc_com;

        if (sharedCom->init) {
            blc_param->frame_id = 0;
        } else {
            blc_param->frame_id = shared->frameId;
        }


#if defined(ISP_HW_V21) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        memcpy(&blc_param->result.v0, &ablc_rk->ablc_proc_res, sizeof(rk_aiq_isp_blc_t));
#else
        memcpy(&blc_param->result, &ablc_rk->ablc_proc_res, sizeof(rk_aiq_isp_blc_t));
#endif
    }

#if defined(ISP_HW_V21) || defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
    cur_params->mBlcV21Params = params->mBlcV21Params;
#else
    cur_params->mBlcParams = params->mBlcParams;
#endif

    mProcResShared = NULL;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
