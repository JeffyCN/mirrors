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

#include "RkAiqAmdHandle.h"

#include "RkAiqCore.h"
#include "common/media_buffer/media_buffer.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAmdHandleInt);

XCamReturn RkAiqAmdHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amd handle prepare failed");

    RkAiqAlgoConfigAmd* amd_config_int = (RkAiqAlgoConfigAmd*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    amd_config_int->spWidth    = sharedCom->spWidth;
    amd_config_int->spHeight   = sharedCom->spHeight;
    amd_config_int->spAlignedW = sharedCom->spAlignedWidth;
    amd_config_int->spAlignedH = sharedCom->spAlignedHeight;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "amd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqAmdHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig      = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmd());
    mProcInParam = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmd());
    // mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmd());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAmdHandleInt::updateConfig(bool needSync) { return XCAM_RETURN_NO_ERROR; }

XCamReturn RkAiqAmdHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPreAmd* amd_pre_int        = (RkAiqAlgoPreAmd*)mPreInParam;
    RkAiqAlgoPreResAmd* amd_pre_res_int = (RkAiqAlgoPreResAmd*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amd handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "amd algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAmdHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAmd* amd_proc_int = (RkAiqAlgoProcAmd*)mProcInParam;

    mProcResShared = new RkAiqAlgoProcResAmdIntShared();
    if (!mProcResShared.ptr()) {
        LOGE("new amd mProcOutParam failed, bypass!");
        return XCAM_RETURN_BYPASS;
    }
    RkAiqAlgoProcResAmd* amd_proc_res_int = &mProcResShared->result;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amd handle processing failed");
    }

    memset(&amd_proc_res_int->amd_proc_res, 0,
           sizeof(amd_proc_res_int->amd_proc_res));
    amd_proc_int->stats.spImage = shared->sp;
    amd_proc_int->stats.ispGain = shared->ispGain;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    ret                         = des->processing(mProcInParam, (RkAiqAlgoResCom*)amd_proc_res_int);
    RKAIQCORE_CHECK_RET(ret, "amd algo processing failed");

    MediaBuffer_t* mbuf = amd_proc_res_int->amd_proc_res.st_ratio;
    if (mbuf) {
        MotionBufMetaData_t* metadata = (MotionBufMetaData_t*)mbuf->pMetaData;
        mProcResShared->set_sequence(shared->frameId);
        RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_AMD_PROC_RES_OK, metadata->frame_id, mProcResShared);
        mAiqCore->post_message(msg);
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmdHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAmd* amd_post_int        = (RkAiqAlgoPostAmd*)mPostInParam;
    RkAiqAlgoPostResAmd* amd_post_res_int = (RkAiqAlgoPostResAmd*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "amd handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "amd algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAmdHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAmd* amd_com = &mProcResShared->result;

    rk_aiq_isp_md_params_t* md_param = params->mMdParams->data().ptr();
    if (sharedCom->init) {
        md_param->frame_id = 0;
    } else {
        md_param->frame_id = shared->frameId;
    }

    if (!amd_com) {
        LOGD_ANALYZER("no amd result");
        return XCAM_RETURN_NO_ERROR;
    }

    md_param->result = amd_com->amd_proc_res;

    cur_params->mMdParams = params->mMdParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
