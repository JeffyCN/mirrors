/*
 * RkAiqAdrcHandle.h
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

namespace RkCam {

void RkAiqAdrcHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdrc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdrc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdrc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdrc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdrc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdrc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdrcHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adrc handle prepare failed");

    // TODO config adrc common params
    RkAiqAlgoConfigAdrc* adrc_config = (RkAiqAlgoConfigAdrc*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "adrc algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret             = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des  = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAdrc* adrc_pre = (RkAiqAlgoPreAdrc*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "adrc handle preProcess failed");

    // TODO config common adrc preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "adrc handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandle::processing() {
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAdrc* adrc_pre = (RkAiqAlgoProcAdrc*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "adrc handle processing failed");

    // TODO config common adrc processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "adrc algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdrcHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des   = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAdrc* adrc_pre = (RkAiqAlgoPostAdrc*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "adrc handle postProcess failed");

    // TODO config common adrc postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "adrc algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
