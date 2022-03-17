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

void RkAiqAfHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAf());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAf());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAf());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAf());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAf());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAf());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAf());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAfHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "af handle prepare failed");

    // TODO config af common params
    RkAiqAlgoConfigAf* af_config = (RkAiqAlgoConfigAf*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "af algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAf* af_pre                      = (RkAiqAlgoPreAf*)mPreInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AF);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "af handle preProcess failed");

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGE("the xcamvideobuffer of af stats is null");
    }
    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGE("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common af preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "af handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandle::processing() {
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAf* af_pre                     = (RkAiqAlgoProcAf*)mProcInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AF);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "af handle processing failed");

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGE("the xcamvideobuffer of af stats is null");
    }
    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGE("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common af processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "af algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAf* af_pre                     = (RkAiqAlgoPostAf*)mPostInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AF);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "af handle postProcess failed");

    RkAiqAfStats* xAfStats = nullptr;
    if (shared->afStatsBuf) {
        xAfStats = (RkAiqAfStats*)shared->afStatsBuf->map(shared->afStatsBuf);
        if (!xAfStats) LOGE("af stats is null");
    } else {
        LOGE("the xcamvideobuffer of af stats is null");
    }
    if ((!xAfStats || !xAfStats->af_stats_valid) && !sharedCom->init) {
        LOGE("no af stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common af postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "af algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
