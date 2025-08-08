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

#include "RkAiqIeHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/ie/include/ie_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqIeHandleInt);

void RkAiqIeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcYnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResIe());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqIeHandleInt::setAttrib(ie_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqIeHandleInt::getAttrib(ie_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_ie_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqIeHandleInt::queryStatus(ie_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mIeParams.ptr()) {
        rk_aiq_isp_ie_params_t* ie_param = mAiqCore->mAiqCurParams->data()->mIeParams->data().ptr();
        if (ie_param) {
            status->stMan = ie_param->result;
            status->en = ie_param->en;
            status->bypass = ie_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqIeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ie handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ie algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqIeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ie handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mIeParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_IE_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_IE_PARAM;
        wrap_param.man_param_size = sizeof(ie_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new ie manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_IE_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_IE_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_IE_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "ie algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqIeHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResIe* ie_res = (RkAiqAlgoProcResIe*)mProcOutParam;

    rk_aiq_isp_ie_params_t* ie_param = params->mIeParams->data().ptr();

    if (!ie_res) {
        LOGE_ANR("no ie result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            ie_param->frame_id = 0;
        } else {
            ie_param->frame_id = shared->frameId;
        }

        if (ie_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            ie_param->sync_flag = mSyncFlag;
            ie_param->en = ie_res->res_com.en;
            ie_param->bypass = ie_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mIeParams = params->mIeParams;

            ie_param->is_update = true;
            LOGD_ANR("Ie [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != ie_param->sync_flag) {
            ie_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mIeParams.ptr()) {
                ie_param->result = cur_params->mIeParams->data()->result;
                ie_param->en = cur_params->mIeParams->data()->en;
                ie_param->bypass = cur_params->mIeParams->data()->bypass;
                ie_param->is_update = true;
            } else {
                LOGE_ANR("Ie: no latest params !");
                ie_param->is_update = false;
            }
            LOGD_ANR("Ie [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            ie_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
