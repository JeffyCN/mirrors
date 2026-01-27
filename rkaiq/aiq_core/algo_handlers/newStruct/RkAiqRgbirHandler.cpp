/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */

#include "RkAiqRgbirHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/rgbir/include/rgbir_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqRgbirHandleInt);

void RkAiqRgbirHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcRgbir());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResRgbir());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqRgbirHandleInt::setAttrib(rgbir_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqRgbirHandleInt::getAttrib(rgbir_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_rgbir_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqRgbirHandleInt::queryStatus(rgbir_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mRgbirParams.ptr()) {
        rk_aiq_isp_rgbir_params_t* rgbir_param = mAiqCore->mAiqCurParams->data()->mRgbirParams->data().ptr();
        if (rgbir_param) {
            status->stMan = rgbir_param->result;
            status->en = rgbir_param->en;
            status->bypass = rgbir_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ARGBIR("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ARGBIR("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqRgbirHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "rgbir handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "rgbir algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqRgbirHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "rgbir handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mRgbirParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_RGBIR_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_RGBIR_PARAM;
        wrap_param.man_param_size = sizeof(rgbir_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ARGBIR("get new rgbir manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_RGBIR_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_RGBIR_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_RGBIR_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "rgbir algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqRgbirHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResRgbir* rgbir_res = (RkAiqAlgoProcResRgbir*)mProcOutParam;

    rk_aiq_isp_rgbir_params_t* rgbir_param = params->mRgbirParams->data().ptr();

    if (!rgbir_res) {
        LOGE_ARGBIR("no rgbir result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            rgbir_param->frame_id = 0;
        } else {
            rgbir_param->frame_id = shared->frameId;
        }

        if (rgbir_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            rgbir_param->sync_flag = mSyncFlag;
            rgbir_param->en = rgbir_res->res_com.en;
            rgbir_param->bypass = rgbir_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mRgbirParams = params->mRgbirParams;

            rgbir_param->is_update = true;
            LOGD_ARGBIR("Rgbir [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != rgbir_param->sync_flag) {
            rgbir_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mRgbirParams.ptr()) {
                rgbir_param->result = cur_params->mRgbirParams->data()->result;
                rgbir_param->en = cur_params->mRgbirParams->data()->en;
                rgbir_param->bypass = cur_params->mRgbirParams->data()->bypass;
                rgbir_param->is_update = true;
            } else {
                LOGE_ARGBIR("Rgbir: no latest params !");
                rgbir_param->is_update = false;
            }
            LOGD_ARGBIR("Rgbir [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            rgbir_param->is_update = false;
            LOGD_ARGBIR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
