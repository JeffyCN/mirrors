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

void RkAiqAdhazHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdhaz());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdhaz());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdhaz());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdhaz());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdhaz());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdhaz());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdhaz());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdhazHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle prepare failed");

    // TODO config adhaz common params
    RkAiqAlgoConfigAdhaz* adhaz_config = (RkAiqAlgoConfigAdhaz*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "adhaz algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret               = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des    = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreAdhaz* adhaz_pre = (RkAiqAlgoPreAdhaz*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle preProcess failed");

    // TODO config common adhaz preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "adhaz handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandle::processing() {
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcAdhaz* adhaz_pre = (RkAiqAlgoProcAdhaz*)mProcInParam;

    int groupId = mAiqCore->mAlgoTypeToGrpMaskMap[RK_AIQ_ALGO_TYPE_ADHAZ];

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle processing failed");

    // TODO config common adhaz processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "adhaz algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des     = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostAdhaz* adhaz_pre = (RkAiqAlgoPostAdhaz*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle postProcess failed");

    // TODO config common adhaz postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "adhaz algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
