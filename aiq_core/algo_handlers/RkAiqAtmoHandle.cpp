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

void RkAiqAtmoHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAtmo());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAtmo());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAtmo());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAtmo());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAtmo());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAtmo());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAtmo());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAtmoHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "atmo handle prepare failed");

    // TODO config ahdr common params
    RkAiqAlgoConfigAtmo* atmo_config            = (RkAiqAlgoConfigAtmo*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ATMO);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "atmo algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des  = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAtmo* atmo_pre = (RkAiqAlgoPreAtmo*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "atmo handle preProcess failed");

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ATMO);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    if (!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->ahdr_proc_res = NULL;
        //
        // return XCAM_RETURN_BYPASS;
    }
    // TODO config common ahdr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "atmo handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandle::processing() {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAtmo* atmo_pre = (RkAiqAlgoProcAtmo*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "atmo handle processing failed");

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ATMO);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    if (!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->ahdr_proc_res = NULL;
        //
        // return XCAM_RETURN_BYPASS;
    }
    // TODO config common ahdr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "atmo algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAtmo* amerge_pre = (RkAiqAlgoPostAtmo*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "atmo handle postProcess failed");

    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_ATMO);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    if (!shared->ispStats->atmo_stats_valid && !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //         comb->ahdr_proc_res = NULL;
        //
        // return XCAM_RETURN_BYPASS;
    }
    // TODO config common ahdr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "atmo algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
