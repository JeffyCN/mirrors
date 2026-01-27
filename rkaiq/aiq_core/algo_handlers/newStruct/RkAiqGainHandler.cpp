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

#include "RkAiqGainHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/gain/include/gain_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqGainHandleInt);

void RkAiqGainHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new _RkAiqAlgoConfigGain());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcGain());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResGain());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqGainHandleInt::setAttrib(gain_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGainHandleInt::getAttrib(gain_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_gain_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGainHandleInt::queryStatus(gain_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mGainParams.ptr()) {
        rk_aiq_isp_gain_params_t* gain_param = mAiqCore->mAiqCurParams->data()->mGainParams->data().ptr();
        if (gain_param) {
            status->stMan = gain_param->result;
            status->en = gain_param->en;
            status->bypass = gain_param->bypass;
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

XCamReturn RkAiqGainHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "gain handle prepare failed");

    RkAiqAlgoConfigGain* gain_config_int   = (RkAiqAlgoConfigGain*)mConfig;
    gain_config_int->mem_ops_ptr   = mAiqCore->mShareMemOps;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "gain algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqGainHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "gain handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mGainParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_GAIN_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_GAIN_PARAM;
        wrap_param.man_param_size = sizeof(gain_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new gain manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_GAIN_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_GAIN_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_GAIN_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "gain algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqGainHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResGain* gain_res = (RkAiqAlgoProcResGain*)mProcOutParam;

    rk_aiq_isp_gain_params_t* gain_param = params->mGainParams->data().ptr();

    if (!gain_res) {
        LOGE_ANR("no gain result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            gain_param->frame_id = 0;
        } else {
            gain_param->frame_id = shared->frameId;
        }

        if (gain_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            gain_param->sync_flag = mSyncFlag;
            gain_param->en = gain_res->res_com.en;
            gain_param->bypass = gain_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mGainParams = params->mGainParams;

            gain_param->is_update = true;
            LOGD_ANR("Gain [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != gain_param->sync_flag) {
            gain_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mGainParams.ptr()) {
                gain_param->result = cur_params->mGainParams->data()->result;
                gain_param->en = cur_params->mGainParams->data()->en;
                gain_param->bypass = cur_params->mGainParams->data()->bypass;
                gain_param->is_update = true;
            } else {
                LOGE_ANR("Gain: no latest params !");
                gain_param->is_update = false;
            }
            LOGD_ANR("Gain [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            gain_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
