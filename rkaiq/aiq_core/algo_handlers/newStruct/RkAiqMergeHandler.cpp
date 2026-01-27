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

#include "RkAiqMergeHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/merge/include/merge_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqMergeHandleInt);

void RkAiqMergeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcMerge());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResMerge());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqMergeHandleInt::setAttrib(mge_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_merge_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqMergeHandleInt::getAttrib(mge_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_merge_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqMergeHandleInt::queryStatus(mge_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mMergeParams.ptr()) {
        rk_aiq_isp_merge_params_t* merge_param = mAiqCore->mAiqCurParams->data()->mMergeParams->data().ptr();
        if (merge_param) {
            status->stMan= merge_param->result;
            status->en = merge_param->en;
            status->bypass = merge_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_AMERGE("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_AMERGE("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqMergeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "merge handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "merge algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqMergeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "merge handle processing failed");
    }

    RkAiqAlgoProcMerge* merge_proc_param = (RkAiqAlgoProcMerge*)mProcInParam;
    merge_proc_param->LongFrmMode = mAeProcRes.LongFrmMode;
    mProcOutParam->algoRes = &shared->fullParams->mMergeParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_MERGE_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_MERGE_PARAM;
        wrap_param.man_param_size = sizeof(mge_param_t);
        wrap_param.man_param_ptr = mProcOutParam->algoRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_AMERGE("get new manual params success !");
            mProcOutParam->en = wrap_param.en;
            mProcOutParam->bypass = wrap_param.bypass;
            mProcOutParam->cfg_update = true;
        } else {
            mProcOutParam->cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_MERGE_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_MERGE_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_MERGE_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "merge algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqMergeHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResMerge* merge_res = (RkAiqAlgoProcResMerge*)mProcOutParam;

    rk_aiq_isp_merge_params_t* merge_param = params->mMergeParams->data().ptr();

    if (!merge_res) {
        LOGD_ANALYZER("no merge result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            merge_param->frame_id = 0;
        } else {
            merge_param->frame_id = shared->frameId;
        }

        if (merge_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            merge_param->sync_flag = mSyncFlag;
            merge_param->en = merge_res->res_com.en;
            merge_param->bypass = merge_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mMergeParams = params->mMergeParams;
            merge_param->is_update = true;
            LOGD_AMERGE("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != merge_param->sync_flag) {
            merge_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mMergeParams.ptr()) {
                merge_param->result = cur_params->mMergeParams->data()->result;
                merge_param->en = cur_params->mMergeParams->data()->en;
                merge_param->bypass = cur_params->mMergeParams->data()->bypass;
                merge_param->is_update = true;
            } else {
                LOGE_AMERGE("no latest params !");
                merge_param->is_update = false;
            }
            LOGD_AMERGE("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            merge_param->is_update = false;
            LOGD_AMERGE("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam