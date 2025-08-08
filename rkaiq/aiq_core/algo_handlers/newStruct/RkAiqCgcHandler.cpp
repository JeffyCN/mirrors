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

#include "RkAiqCgcHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/cgc/include/cgc_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCgcHandleInt);

void RkAiqCgcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcCgc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResCgc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCgcHandleInt::setAttrib(cgc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCgcHandleInt::getAttrib(cgc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_cgc_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCgcHandleInt::queryStatus(cgc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCgcParams.ptr()) {
        rk_aiq_isp_cgc_params_t* cgc_param = mAiqCore->mAiqCurParams->data()->mCgcParams->data().ptr();
        if (cgc_param) {
            status->stMan = cgc_param->result;
            status->en = cgc_param->en;
            status->bypass = cgc_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACGC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACGC("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCgcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "cgc handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "cgc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCgcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cgc handle processing failed");
    }

    mProcOutParam->algoRes = &shared->fullParams->mCgcParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_CGC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_CGC_PARAM;
        wrap_param.man_param_size = sizeof(cgc_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ACGC("get new cgc manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_CGC_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_CGC_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_CGC_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "cgc algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqCgcHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResCgc* cgc_res = (RkAiqAlgoProcResCgc*)mProcOutParam;

    rk_aiq_isp_cgc_params_t* cgc_param = params->mCgcParams->data().ptr();

    if (!cgc_res) {
        LOGE_ACGC("no cgc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            cgc_param->frame_id = 0;
        } else {
            cgc_param->frame_id = shared->frameId;
        }

        if (cgc_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            cgc_param->sync_flag = mSyncFlag;
            cgc_param->en = cgc_res->res_com.en;
            cgc_param->bypass = cgc_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mCgcParams = params->mCgcParams;

            cgc_param->is_update = true;
            LOGD_ACGC("Cgc [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != cgc_param->sync_flag) {
            cgc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCgcParams.ptr()) {
                cgc_param->result = cur_params->mCgcParams->data()->result;
                cgc_param->en = cur_params->mCgcParams->data()->en;
                cgc_param->bypass = cur_params->mCgcParams->data()->bypass;
                cgc_param->is_update = true;
            } else {
                LOGE_ACGC("Cgc: no latest params !");
                cgc_param->is_update = false;
            }
            LOGD_ACGC("Cgc [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            cgc_param->is_update = false;
            LOGD_ACGC("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
