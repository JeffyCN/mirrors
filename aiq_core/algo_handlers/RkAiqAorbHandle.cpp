/*
 * RkAiqAorbHandle.cpp
 *
 *  Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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
#include "RkAiqHandleInt.h"

namespace RkCam {

void RkAiqAorbHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqAorbHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAorbInt());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAorbInt());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAorbInt());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAorbInt());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAorbInt());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAorbInt());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAorbInt());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAorbHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqAorbHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aorb handle prepare failed");

    RkAiqAlgoConfigAorbInt* aorb_config_int     = (RkAiqAlgoConfigAorbInt*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    CalibDbV2_Orb_t* orb =
        (CalibDbV2_Orb_t*)CALIBDBV2_GET_MODULE_PTR((void*)(sharedCom->calibv2), orb);
    if (orb) {
        aorb_config_int->orb_calib_cfg.param.orb_en = orb->param.orb_en;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "aorb algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAorbHandleInt::preProcess() { return XCAM_RETURN_NO_ERROR; }

XCamReturn RkAiqAorbHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAorbInt* aorb_proc_int        = (RkAiqAlgoProcAorbInt*)mProcInParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared = nullptr;
    int groupId                                = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AORB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    RkAiqProcResComb* comb       = &shared->procResComb;
    aorb_proc_int->orb_stats_buf = shared->orbStats;

    ret = RkAiqAorbHandle::processing();
    if (ret) {
        comb->aorb_proc_res = NULL;
        RKAIQCORE_CHECK_RET(ret, "aorb handle processing failed");
    }

    comb->aorb_proc_res = NULL;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "aorb algo processing failed");

    comb->aorb_proc_res = (RkAiqAlgoProcResAorb*)mProcOutParam;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAorbHandleInt::postProcess() { return XCAM_RETURN_NO_ERROR; }

void RkAiqAorbHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAorb());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAorb());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAorb());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAorb());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAorb());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAorb());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAorb());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAorbHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "aorb handle prepare failed");

    RkAiqAlgoConfigAorb* aorb_config = (RkAiqAlgoConfigAorb*)mConfig;

    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "aorb algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAorbHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAorb* aorb_pre                  = (RkAiqAlgoPreAorb*)mPreInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AORB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "aorb handle preProcess failed");

    if (!shared->orbStats && !sharedCom->init) {
        LOGE_AORB("no orb stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "aorb handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAorbHandle::processing() {
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAorb* aorb_pre                 = (RkAiqAlgoProcAorb*)mProcInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AORB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;
    RkAiqIspStats* ispStats = shared->ispStats;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "aorb handle processing failed");

    if (!shared->orbStats && !sharedCom->init) {
        LOGE_AORB("no orb stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "aorb algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAorbHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des                   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAorb* aorb_pre                 = (RkAiqAlgoPostAorb*)mPostInParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared  = nullptr;
    int groupId                                 = mAiqCore->getGroupId(RK_AIQ_ALGO_TYPE_AORB);
    if (groupId >= 0) {
        if (mAiqCore->getGroupSharedParams(groupId, shared) != XCAM_RETURN_NO_ERROR)
            return XCAM_RETURN_BYPASS;
    } else
        return XCAM_RETURN_BYPASS;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "aorb handle postProcess failed");

    if (!shared->orbStats && !sharedCom->init) {
        LOGE_AORB("no orb stats, ignore!");
        return XCAM_RETURN_BYPASS;
    }

    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "aorb algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
