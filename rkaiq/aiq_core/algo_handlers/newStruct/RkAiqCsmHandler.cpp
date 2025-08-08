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

#include "RkAiqCsmHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/csm/include/csm_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCsmHandleInt);

void RkAiqCsmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResCsm());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCsmHandleInt::setAttrib(csm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_csm_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCsmHandleInt::getAttrib(csm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_csm_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCsmHandleInt::queryStatus(csm_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCsmParams.ptr()) {
        rk_aiq_isp_csm_params_t* csm_param = mAiqCore->mAiqCurParams->data()->mCsmParams->data().ptr();
        if (csm_param) {
            status->stMan = csm_param->result; 
            status->en = csm_param->en;
            status->bypass = csm_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACSM("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACSM("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCsmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "csm handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "csm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCsmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "csm handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mCsmParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_CSM_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_CSM_PARAM;
        wrap_param.man_param_size = sizeof(csm_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ACSM("get new manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        globalParamsManager->lockAlgoParam(RESULT_TYPE_CSM_PARAM);
        mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_CSM_PARAM);
        ret                       = des->processing(mProcInParam, mProcOutParam);
        globalParamsManager->unlockAlgoParam(RESULT_TYPE_CSM_PARAM);
    }

    RKAIQCORE_CHECK_RET(ret, "csm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqCsmHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResCsm* csm_res = (RkAiqAlgoProcResCsm*)mProcOutParam;

    rk_aiq_isp_csm_params_t* csm_param = params->mCsmParams->data().ptr();

    if (!csm_res) {
        LOGD_ANALYZER("no csm result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            csm_param->frame_id = 0;
        } else {
            csm_param->frame_id = shared->frameId;
        }

        if (csm_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            csm_param->sync_flag = mSyncFlag;
            csm_param->en = csm_res->res_com.en;
            csm_param->bypass = csm_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mCsmParams = params->mCsmParams;
            csm_param->is_update = true;
            LOGD_ACSM("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != csm_param->sync_flag) {
            csm_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCsmParams.ptr()) {
                csm_param->result = cur_params->mCsmParams->data()->result;
                csm_param->en = cur_params->mCsmParams->data()->en;
                csm_param->bypass = cur_params->mCsmParams->data()->bypass;
                csm_param->is_update = true;
            } else {
                LOGE_ACSM("no latest params !");
                csm_param->is_update = false;
            }
            LOGD_ACSM("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            csm_param->is_update = false;
            LOGD_ACSM("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
