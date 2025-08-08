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

#include "RkAiqCpHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/cp/include/cp_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCpHandleInt);

void RkAiqCpHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcCp());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResCp());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCpHandleInt::setAttrib(cp_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCpHandleInt::getAttrib(cp_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_cp_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCpHandleInt::queryStatus(cp_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCpParams.ptr()) {
        rk_aiq_isp_cp_params_t* cp_param = mAiqCore->mAiqCurParams->data()->mCpParams->data().ptr();
        if (cp_param) {
            status->stMan = cp_param->result;
            status->en = cp_param->en;
            status->bypass = cp_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACP("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACP("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCpHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "cp handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "cp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCpHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cp handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mCpParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_CP_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_CP_PARAM;
        wrap_param.man_param_size = sizeof(cp_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ACP("get new cp manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_CP_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_CP_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_CP_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "cp algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqCpHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResCp* cp_res = (RkAiqAlgoProcResCp*)mProcOutParam;

    rk_aiq_isp_cp_params_t* cp_param = params->mCpParams->data().ptr();

    if (!cp_res) {
        LOGE_ACP("no cp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            cp_param->frame_id = 0;
        } else {
            cp_param->frame_id = shared->frameId;
        }

        if (cp_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            cp_param->sync_flag = mSyncFlag;
            cp_param->en = cp_res->res_com.en;
            cp_param->bypass = cp_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mCpParams = params->mCpParams;

            cp_param->is_update = true;
            LOGD_ACP("Cp [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != cp_param->sync_flag) {
            cp_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCpParams.ptr()) {
                cp_param->result = cur_params->mCpParams->data()->result;
                cp_param->en = cur_params->mCpParams->data()->en;
                cp_param->bypass = cur_params->mCpParams->data()->bypass;
                cp_param->is_update = true;
            } else {
                LOGE_ACP("Cp: no latest params !");
                cp_param->is_update = false;
            }
            LOGD_ACP("Cp [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            cp_param->is_update = false;
            LOGD_ACP("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
