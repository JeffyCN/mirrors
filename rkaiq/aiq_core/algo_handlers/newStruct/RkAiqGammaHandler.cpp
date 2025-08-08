/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqGammaHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/gamma/include/gamma_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqGammaHandleInt);

void RkAiqGammaHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResGamma());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqGammaHandleInt::setAttrib(gamma_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_gamma_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGammaHandleInt::getAttrib(gamma_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_gamma_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGammaHandleInt::queryStatus(gamma_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mGammaParams.ptr()) {
        rk_aiq_isp_gamma_params_t* gamma_param = mAiqCore->mAiqCurParams->data()->mGammaParams->data().ptr();
        if (gamma_param) {
            status->stMan = gamma_param->result; 
            status->en = gamma_param->en;
            status->bypass = gamma_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_AGAMMA("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_AGAMMA("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGammaHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "gamma handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "gamma algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqGammaHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResGamma* gamma_proc_res_int =
            (RkAiqAlgoProcResGamma*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "gamma handle processing failed");
    }

    gamma_proc_res_int->gammaRes =  &shared->fullParams->mGammaParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_AGAMMA_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_AGAMMA_PARAM;
        wrap_param.man_param_size = sizeof(gamma_param_t);
        wrap_param.man_param_ptr = gamma_proc_res_int->gammaRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_AGAMMA("get new manual params success !");
            gamma_proc_res_int->res_com.en = wrap_param.en;
            gamma_proc_res_int->res_com.bypass = wrap_param.bypass;
            gamma_proc_res_int->res_com.cfg_update = true;
        } else {
            gamma_proc_res_int->res_com.cfg_update = false;
        }
    }
    else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_AGAMMA_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_AGAMMA_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_AGAMMA_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "gamma algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqGammaHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResGamma* gamma_res = (RkAiqAlgoProcResGamma*)mProcOutParam;

    rk_aiq_isp_gamma_params_t* gamma_param = params->mGammaParams->data().ptr();

    if (!gamma_res) {
        LOGD_ANALYZER("no gamma result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            gamma_param->frame_id = 0;
        } else {
            gamma_param->frame_id = shared->frameId;
        }

        if (gamma_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            gamma_param->sync_flag = mSyncFlag;
            gamma_param->en = gamma_res->res_com.en;
            gamma_param->bypass = gamma_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mGammaParams = params->mGammaParams;
            gamma_param->is_update = true;
            LOGD_AGAMMA("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != gamma_param->sync_flag) {
            gamma_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mGammaParams.ptr()) {
                gamma_param->result = cur_params->mGammaParams->data()->result;
                gamma_param->en = cur_params->mGammaParams->data()->en;
                gamma_param->bypass = cur_params->mGammaParams->data()->bypass;
                gamma_param->is_update = true;
            } else {
                LOGE_AGAMMA("no latest params !");
                gamma_param->is_update = false;
            }
            LOGD_AGAMMA("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            gamma_param->is_update = false;
            LOGD_AGAMMA("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
