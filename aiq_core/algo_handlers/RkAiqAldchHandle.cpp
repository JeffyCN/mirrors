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
#include "RkAiqAldchHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAldchHandleInt);

void RkAiqAldchHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAldch());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAldch());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAldch());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAldch());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAldch());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAldch());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAldch());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAldchHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aldch handle prepare failed");

    RkAiqAlgoConfigAldch* aldch_config_int   = (RkAiqAlgoConfigAldch*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // memcpy(&aldch_config_int->aldch_calib_cfg, &shared->calib->aldch, sizeof(CalibDb_LDCH_t));
    aldch_config_int->resource_path = sharedCom->resourcePath;
    aldch_config_int->mem_ops_ptr   = mAiqCore->mShareMemOps;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    ret                             = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aldch algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldchHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAldch* aldch_pre_int        = (RkAiqAlgoPreAldch*)mPreInParam;
    RkAiqAlgoPreResAldch* aldch_pre_res_int = (RkAiqAlgoPreResAldch*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAldchHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAldch* aldch_proc_int        = (RkAiqAlgoProcAldch*)mProcInParam;
    RkAiqAlgoProcResAldch* aldch_proc_res_int = (RkAiqAlgoProcResAldch*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAldch* aldch_post_int        = (RkAiqAlgoPostAldch*)mPostInParam;
    RkAiqAlgoPostResAldch* aldch_post_res_int = (RkAiqAlgoPostResAldch*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "aldch handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "aldch algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        // TODO
        rk_aiq_uapi_aldch_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::setAttrib(rk_aiq_ldch_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::getAttrib(rk_aiq_ldch_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_aldch_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAldchHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAldch* aldch_com         = (RkAiqAlgoProcResAldch*)mProcOutParam;
    rk_aiq_isp_ldch_params_v20_t* ldch_param = params->mLdchParams->data().ptr();

    if (!aldch_com) {
        LOGD_ANALYZER("no aldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAldch* aldch_rk = (RkAiqAlgoProcResAldch*)aldch_com;

    if (sharedCom->init) {
        ldch_param->frame_id = 0;
    } else {
        ldch_param->frame_id = shared->frameId;
    }

    if (aldch_rk->ldch_result.update) {
        ldch_param->update_mask |= RKAIQ_ISP_LDCH_ID;
        ldch_param->result.ldch_en = aldch_rk->ldch_result.sw_ldch_en;
        if (ldch_param->result.ldch_en) {
            ldch_param->result.lut_h_size = aldch_rk->ldch_result.lut_h_size;
            ldch_param->result.lut_v_size = aldch_rk->ldch_result.lut_v_size;
            ldch_param->result.lut_size   = aldch_rk->ldch_result.lut_map_size;
            ldch_param->result.lut_mem_fd = aldch_rk->ldch_result.lut_mapxy_buf_fd;
        }
    } else {
        ldch_param->update_mask &= ~RKAIQ_ISP_LDCH_ID;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAldch* aldch_rk = (RkAiqAlgoProcResAldch*)aldch_com;
    }

    cur_params->mLdchParams = params->mLdchParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
