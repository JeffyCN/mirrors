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

#include "RkAiqHandle.h"
#include "RkAiqCore.h"

namespace RkCam {

void RkAiqAeHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAe());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAe());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAe());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAe());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAe());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAe());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAe());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAeHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ae handle prepare failed");

    // TODO config ae common params:
    RkAiqAlgoConfigAe* ae_config                = (RkAiqAlgoConfigAe*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    /*****************AecConfig Sensor Exp related info*****************/
    ae_config->LinePeriodsPerField = (float)sharedCom->snsDes.frame_length_lines;
    ae_config->PixelPeriodsPerLine = (float)sharedCom->snsDes.line_length_pck;
    ae_config->PixelClockFreqMHZ   = (float)sharedCom->snsDes.pixel_clock_freq_mhz;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "ae algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAe* ae_pre                      = (RkAiqAlgoPreAe*)mPreInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    int groupId = mAiqCore->mAlgoTypeToGrpMaskMap[RK_AIQ_ALGO_TYPE_AE];
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = mAiqCore->mAlogsGroupSharedParamsMap[groupId];
    RkAiqIspStats* ispStats                    = shared->ispStats;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "ae handle preProcess failed");

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = (RkAiqAecStats*)shared->aecStatsBuf->map(shared->aecStatsBuf);
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGE_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGE("no aec stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common ae preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "ae handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandle::processing() {
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAe* ae_proc                    = (RkAiqAlgoProcAe*)mProcInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AE);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "ae handle processing failed");

    RkAiqAecStats* xAecStats = nullptr;
    if (shared->aecStatsBuf) {
        xAecStats = (RkAiqAecStats*)shared->aecStatsBuf->map(shared->aecStatsBuf);
        if (!xAecStats) LOGE_AEC("aec stats is null");
    } else {
        LOGE_AEC("the xcamvideobuffer of aec stats is null");
    }
    if ((!xAecStats || !xAecStats->aec_stats_valid) && !sharedCom->init) {
        LOGE("no aec stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common ae processing params
    ae_proc->aecStatsBuf = shared->aecStatsBuf;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "ae algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAeHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAe* ae_post                    = (RkAiqAlgoPostAe*)mPostInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AE);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "ae handle postProcess failed");

    if (!shared->aecStatsBuf && !sharedCom->init) {
        LOGE("no aec stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    // TODO config common ae postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "ae algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
