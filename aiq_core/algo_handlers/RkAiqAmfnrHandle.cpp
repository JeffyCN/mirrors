/*
 * RkAiqAmfnrHandle.h
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

void RkAiqAmfnrHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAmfnr());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAmfnr());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAmfnr());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAmfnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAmfnr());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAmfnr());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAmfnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAmfnrHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "amfnr handle prepare failed");

    // TODO config amfnr common params
    RkAiqAlgoConfigAmfnr* amfnr_config = (RkAiqAlgoConfigAmfnr*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "amfnr algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmfnrHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAmfnr* amfnr_pre = (RkAiqAlgoPreAmfnr*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "amfnr handle preProcess failed");

    // TODO config common amfnr preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "amfnr handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmfnrHandle::processing() {
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAmfnr* amfnr_pre = (RkAiqAlgoProcAmfnr*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "amfnr handle processing failed");

    // TODO config common amfnr processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "amfnr algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAmfnrHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAmfnr* amfnr_pre = (RkAiqAlgoPostAmfnr*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "amfnr handle postProcess failed");

    // TODO config common amfnr postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "amfnr algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
