/*
 * RkAiqA3dlutHandle.h
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

void RkAiqA3dlutHandle::init() {
    ENTER_ANALYZER_FUNCTION();

    deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigA3dlut());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreA3dlut());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResA3dlut());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcA3dlut());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResA3dlut());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostA3dlut());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResA3dlut());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqA3dlutHandle::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle prepare failed");

    // TODO config a3dlut common params
    RkAiqAlgoConfigA3dlut* a3dlut_config = (RkAiqAlgoConfigA3dlut*)mConfig;

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->prepare(mConfig);
        RKAIQCORE_CHECK_RET(ret, "a3dlut algo prepare failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandle::preProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des      = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPreA3dlut* a3dlut_pre = (RkAiqAlgoPreA3dlut*)mPreInParam;

    ret = RkAiqHandle::preProcess();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle preProcess failed");

    // TODO config common a3dlut preprocess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->pre_process(mPreInParam, mPreOutParam);
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle pre_process failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandle::processing() {
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoProcA3dlut* a3dlut_pre = (RkAiqAlgoProcA3dlut*)mProcInParam;

    ret = RkAiqHandle::processing();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle processing failed");

    // TODO config common a3dlut processing params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->processing(mProcInParam, mProcOutParam);
        RKAIQCORE_CHECK_RET(ret, "a3dlut algo processing failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandle::postProcess() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                  = XCAM_RETURN_NO_ERROR;
    RkAiqAlgoDescription* des       = (RkAiqAlgoDescription*)mDes;
    RkAiqAlgoPostA3dlut* a3dlut_pre = (RkAiqAlgoPostA3dlut*)mPostInParam;

    ret = RkAiqHandle::postProcess();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle postProcess failed");

    // TODO config common a3dlut postProcess params

    // id != 0 means the thirdparty's algo
    if (mDes->id != 0) {
        ret = des->post_process(mPostInParam, mPostOutParam);
        RKAIQCORE_CHECK_RET(ret, "a3dlut algo postProcess failed");
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

};  // namespace RkCam
