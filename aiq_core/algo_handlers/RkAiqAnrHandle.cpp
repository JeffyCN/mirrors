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

void RkAiqAnrHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAnr());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAnr());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAnr());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAnr());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAnr());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAnrHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "anr handle prepare failed");

    // TODO config anr common params
    RkAiqAlgoConfigAnr* anr_config = (RkAiqAlgoConfigAnr*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "anr algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAnrHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAnr* anr_pre  = (RkAiqAlgoPreAnr*)mPreInParam;

    int groupId = mAiqCore->mAlgoTypeToGrpMaskMap[RK_AIQ_ALGO_TYPE_ANR];

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "anr handle preProcess failed");

    // TODO config common anr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "anr handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAnrHandle::processing() {
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAnr* anr_pre = (RkAiqAlgoProcAnr*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "anr handle processing failed");

    // TODO config common anr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "anr algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAnrHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAnr* anr_pre = (RkAiqAlgoPostAnr*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "anr handle postProcess failed");

    // TODO config common anr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "anr algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
