/*
 * RkAiqHandle.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */
#include "RkAiqCore.h"
#include "RkAiqHandle.h"

namespace RkCam {

void RkAiqAwbHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAwb());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAwb());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAwb());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAwb());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAwb());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAwb());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAwb());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAwbHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "awb handle prepare failed");

    // TODO config awb common params
    RkAiqAlgoConfigAwb* awb_config = (RkAiqAlgoConfigAwb*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "awb algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAwb* awb_pre  = (RkAiqAlgoPreAwb*)mPreInParam;

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AWB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "awb handle preProcess failed");

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    // TODO config common awb preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "awb handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandle::processing() {
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAwb* awb_pre                   = (RkAiqAlgoProcAwb*)mProcInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AWB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "awb handle processing failed");

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    // TODO config common awb processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "awb algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAwbHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAwb* awb_pre                   = (RkAiqAlgoPostAwb*)mPostInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AWB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "awb handle postProcess failed");

    if (!sharedCom->init) {
        if (shared->awbStatsBuf == nullptr) {
            LOGE("no awb stats, ignore!");
            return XCAM_RETURN_BYPASS;
        }
    }

    // TODO config common awb postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "awb algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
