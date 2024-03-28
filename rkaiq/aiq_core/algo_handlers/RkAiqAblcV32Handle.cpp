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
#include "RkAiqAblcV32Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAblcV32HandleInt);

void RkAiqAblcV32HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAblcV32());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAblcV32());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAblcV32());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAblcV32HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        rk_aiq_uapi_ablc_V32_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcV32HandleInt::setAttrib(const rk_aiq_blc_attrib_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_ablc_V32_SetAttrib(mAlgoCtx, const_cast<rk_aiq_blc_attrib_V32_t*>(att), false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && memcmp(&mCurAtt, att, sizeof(*att)))
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

XCamReturn RkAiqAblcV32HandleInt::getAttrib(rk_aiq_blc_attrib_V32_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_ablc_V32_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_ablc_V32_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_ablc_V32_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcV32HandleInt::getProcRes(AblcProc_V32_t* ProcRes) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_ablc_V32_GetProc(mAlgoCtx, ProcRes);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn  RkAiqAblcV32HandleInt::getInfo(rk_aiq_blc_info_v32_t* pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = rk_aiq_uapi_ablc_V32_GetInfo(mAlgoCtx, pInfo);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAblcV32HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ablcV32 handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ablcV32 algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAblcV32HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAblcV32* ablc_pre_int        = (RkAiqAlgoPreAblcV32*)mPreInParam;
    RkAiqAlgoPreResAblcV32* ablc_pre_res_int = (RkAiqAlgoPreResAblcV32*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ablcV32 handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablcV32 algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAblcV32HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAblcV32* ablc_proc_int        = (RkAiqAlgoProcAblcV32*)mProcInParam;

    if (mDes->id == 0) {
        mProcResShared = new RkAiqAlgoProcResAblcV32IntShared();
        if (!mProcResShared.ptr()) {
            LOGE("new BLC(v32) mProcResShared failed, bypass!");
            return XCAM_RETURN_BYPASS;
        }
    }

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    mProcResShared->result.ablcV32_proc_res = &shared->fullParams->mBlcParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret < 0) {
        LOGE_ANALYZER("ablcV32 handle processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mProcResShared = NULL;
        return ret;
    }

    ablc_proc_int->iso      = sharedCom->iso;
    ablc_proc_int->hdr_mode = sharedCom->working_mode;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, (RkAiqAlgoResCom*)(&mProcResShared->result));
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    if (ret < 0) {
        LOGE_ANALYZER("ablcV32 algo processing failed ret %d", ret);
        mProcResShared = NULL;
        return ret;
    } else if (ret == XCAM_RETURN_BYPASS) {
        LOGW_ANALYZER("%s:%d bypass !", __func__, __LINE__);
        mProcResShared = NULL;
        return ret;
    }

    if (!mProcResShared->result.res_com.cfg_update) {
        mProcResShared->result.ablcV32_proc_res = mLatestparam;
        LOGD_ABLC("[%d] copy results from latest !", shared->frameId);
    }

    if (mAiqCore->mAlogsComSharedParams.init) {
        RkAiqCore::RkAiqAlgosGroupShared_t* grpShared = nullptr;
        uint64_t grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_AWB);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                grpShared->res_comb.ablcV32_proc_res = mProcResShared->result.ablcV32_proc_res;
        }
        grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP0);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                grpShared->res_comb.ablcV32_proc_res = mProcResShared->result.ablcV32_proc_res;
        }
        grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_GRP1);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                grpShared->res_comb.ablcV32_proc_res = mProcResShared->result.ablcV32_proc_res;
        }
        grpMask = grpId2GrpMask(RK_AIQ_CORE_ANALYZE_DHAZ);
        if (!mAiqCore->getGroupSharedParams(grpMask, grpShared)) {
            if (grpShared)
                grpShared->res_comb.ablcV32_proc_res = mProcResShared->result.ablcV32_proc_res;
        }
    } else if (mPostShared) {
        mProcResShared->set_sequence(shared->frameId);
        RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_BLC_V32_PROC_RES_OK, shared->frameId, mProcResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
    }

XCamReturn RkAiqAblcV32HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAblcV32* ablc_post_int        = (RkAiqAlgoPostAblcV32*)mPostInParam;
    RkAiqAlgoPostResAblcV32* ablc_post_res_int = (RkAiqAlgoPostResAblcV32*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ablcV32 handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "ablcV32 algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAblcV32HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    LOGD_ANALYZER("handle get result!!\n");
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    if (!mProcResShared.ptr())
        return XCAM_RETURN_NO_ERROR;
    RkAiqAlgoProcResAblcV32* ablc_com             = (RkAiqAlgoProcResAblcV32*)&mProcResShared->result;

    rk_aiq_isp_blc_params_t* blc_param = params->mBlcParams->data().ptr();

    if (!ablc_com) {
        LOGE_ANALYZER("no ablcV32 result");
        mProcResShared = NULL;
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        LOGD_ABLC("yys: %s:%d output isp param start\n", __FUNCTION__, __LINE__);

        if (sharedCom->init) {
            blc_param->frame_id = 0;
        } else {
            blc_param->frame_id = shared->frameId;
        }

        if (ablc_com->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            blc_param->sync_flag = mSyncFlag;
            // copy from algo result
            // set as the latest result
            cur_params->mBlcParams = params->mBlcParams;
            mLatestparam = &cur_params->mBlcParams->data()->result;
            blc_param->is_update = true;
            LOGD_ABLC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != blc_param->sync_flag) {
            blc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mBlcParams.ptr()) {
                blc_param->result = cur_params->mBlcParams->data()->result;
                blc_param->is_update = true;
            } else {
                LOGE_ABLC("no latest params !");
                blc_param->is_update = false;
            }
            LOGD_ABLC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            blc_param->is_update = false;
            // do nothing, result in buf needn't update
            LOGD_ABLC("[%d] params needn't update", shared->frameId);
        }

        // printf("!!!!!!handle get proc result : offset:0x%x gain:0x%x max:0x%x \n",
        // ablc_com->ablcV32_proc_res.isp_ob_offset,
        // ablc_com->ablcV32_proc_res.isp_ob_predgain,ablc_com->ablcV32_proc_res.isp_ob_max);
        LOGD_ABLC("yys: %s:%d output isp param end \n", __FUNCTION__, __LINE__);
    }

    mProcResShared = NULL;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
