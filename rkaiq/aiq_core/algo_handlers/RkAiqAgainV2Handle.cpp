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
#include "RkAiqAgainV2Handle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAgainV2HandleInt);

void RkAiqAgainV2HandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAgainV2());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAgainV2());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAgainV2());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAgainV2HandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        LOGD_ANR("%s:%d\n", __FUNCTION__, __LINE__);
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_againV2_SetAttrib(mAlgoCtx, &mCurAtt, false);
        sendSignal(mCurAtt.sync.sync_mode);
        updateAtt = false;
    }

    if (updataWriteAgainInputAttr) {
        mCurWriteInputAttr = mNewWriteInputAttr;
        rk_aiq_uapiV2_againV2_WriteInput(mAlgoCtx, mCurWriteInputAttr, false);
        updataWriteAgainInputAttr = false;
        sendSignal(mCurWriteInputAttr.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::setAttrib(const rk_aiq_gain_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_againV2_SetAttrib(mAlgoCtx, att, false);
#else
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
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::getAttrib(rk_aiq_gain_attrib_v2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
    rk_aiq_uapi_againV2_GetAttrib(mAlgoCtx, att);
    mCfgMutex.unlock();
#else
    if(att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_againV2_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if(updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_againV2_GetAttrib(mAlgoCtx, att);
            att->sync.done = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::getInfo(rk_aiq_gain_info_v2_t* pInfo) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if(pInfo->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_againV2_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
        mCfgMutex.unlock();
    } else {
        rk_aiq_uapi_againV2_GetInfo(mAlgoCtx, pInfo);
        pInfo->sync.done = true;
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::writeAginIn(rk_aiq_uapiV2_again_wrtIn_attr_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V32)
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapiV2_againV2_WriteInput(mAlgoCtx, att, false);
#else
    // check if there is different between att & mCurWriteInputAttr(sync)/mNewWriteInputAttr(async)
    // if something changed, set att to mNewWriteInputAttr, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewWriteInputAttr, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurWriteInputAttr, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewWriteInputAttr = att;
        updataWriteAgainInputAttr = true;
        waitSignal(att.sync.sync_mode);
    }
#endif //DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();

#endif //defined(ISP_HW_V32)

    EXIT_ANALYZER_FUNCTION();

    return ret;
}


XCamReturn RkAiqAgainV2HandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "again handle prepare failed");

    RkAiqAlgoConfigAgainV2* again_config_int = (RkAiqAlgoConfigAgainV2*)mConfig;
    again_config_int->mem_ops_ptr = mAiqCore->mShareMemOps;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "again algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgainV2HandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAgainV2* again_pre_int        = (RkAiqAlgoPreAgainV2*)mPreInParam;
    RkAiqAlgoPreResAgainV2* again_pre_res_int = (RkAiqAlgoPreResAgainV2*)mPreOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "again handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "again algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAgainV2HandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAgainV2* again_proc_int        = (RkAiqAlgoProcAgainV2*)mProcInParam;
    RkAiqAlgoProcResAgainV2* again_proc_res_int = (RkAiqAlgoProcResAgainV2*)mProcOutParam;
 
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    again_proc_res_int->stAgainProcResult.stFix = &shared->fullParams->mGainParams->data()->result;
#if RK_GAIN_V2_ENABLE_GAIN2DDR
    RkAiqAgainStats* xAgainStats = nullptr;
    if (shared->againStatsBuf) {
        xAgainStats = shared->againStatsBuf;
        if (!xAgainStats) {
            LOGE_ANR("again stats is null");
        }
    } else {
        LOGE_ANR("the xcamvideobuffer of gain stats is null");
    }

    if (!xAgainStats) {
        LOGW_ANR("no again stats, ignore!");
        again_proc_int->stats.stats_true = false;
    } else {
        again_proc_int->stats.stats_true = true;
        again_proc_int->stats.dbginfo_fd = xAgainStats->again_stats.dbginfo_fd;
        LOGW_ANR("again stats dbginfo_fd=%d!", again_proc_int->stats.dbginfo_fd);
    }
    static int auvnr_proc_framecnt = 0;
    auvnr_proc_framecnt++;

#endif

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "again handle processing failed");
    }

    // TODO: fill procParam
    again_proc_int->iso      = sharedCom->iso;
    again_proc_int->hdr_mode = sharedCom->working_mode;

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "again algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAgainV2* again_post_int        = (RkAiqAlgoPostAgainV2*)mPostInParam;
    RkAiqAlgoPostResAgainV2* again_post_res_int = (RkAiqAlgoPostResAgainV2*)mPostOutParam;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "auvnr handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "auvnr algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAgainV2HandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAgainV2* again_rk = (RkAiqAlgoProcResAgainV2*)mProcOutParam;

    if (!again_rk) {
        LOGD_ANALYZER("no asharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (params->mGainParams.ptr()) {
            rk_aiq_isp_gain_params_t* gain_param = params->mGainParams->data().ptr();
            if (sharedCom->init) {
                gain_param->frame_id = 0;
            } else {
                gain_param->frame_id = shared->frameId;
            }
            LOGD_ANR("oyyf: %s:%d output gain  param start\n", __FUNCTION__, __LINE__);

            if (again_rk->res_com.cfg_update) {
                mSyncFlag = shared->frameId;
                gain_param->sync_flag = mSyncFlag;
                // copy from algo result
                // set as the latest result
                cur_params->mGainParams = params->mGainParams;
                gain_param->is_update = true;
                LOGD_ANR("[%d] params from algo", mSyncFlag);
            } else if (mSyncFlag != gain_param->sync_flag) {
                gain_param->sync_flag = mSyncFlag;
                // copy from latest result
                if (cur_params->mGainParams.ptr()) {
                    gain_param->result = cur_params->mGainParams->data()->result;
                    gain_param->is_update = true;
                } else {
                    LOGE_ANR("no latest params !");
                    gain_param->is_update = false;
                }
                LOGD_ANR("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
            } else {
                // do nothing, result in buf needn't update
                gain_param->is_update = false;
                LOGD_ANR("[%d] params needn't update", shared->frameId);
            }

            LOGD_ANR("oyyf: %s:%d output gain param end \n", __FUNCTION__, __LINE__);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
