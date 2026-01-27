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

#include "RkAiqDpccHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/dpc/include/dpc_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqDpccHandleInt);

void RkAiqDpccHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResDpcc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqDpccHandleInt::setAttrib(dpc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_dpc_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDpccHandleInt::getAttrib(dpc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_dpc_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDpccHandleInt::queryStatus(dpc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mDpccParams.ptr()) {
        rk_aiq_isp_dpcc_params_t* dpc_param = mAiqCore->mAiqCurParams->data()->mDpccParams->data().ptr();
        if (dpc_param) {
            status->stMan = dpc_param->result;
            status->en = dpc_param->en;
            status->bypass = dpc_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADPCC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADPCC("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDpccHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "dpcc handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "dpcc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqDpccHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResDpcc* dpcc_proc_res_int =
            (RkAiqAlgoProcResDpcc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "dpcc handle processing failed");
    }

    dpcc_proc_res_int->dpcRes =  &shared->fullParams->mDpccParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_DPCC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_DPCC_PARAM;
        wrap_param.man_param_size = sizeof(dpc_param_t);
        wrap_param.man_param_ptr = dpcc_proc_res_int->dpcRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ADPCC("get new manual params success !");
            dpcc_proc_res_int->res_com.en = wrap_param.en;
            dpcc_proc_res_int->res_com.bypass = wrap_param.bypass;
            dpcc_proc_res_int->res_com.cfg_update = true;
        } else {
            dpcc_proc_res_int->res_com.cfg_update = false;
        }
    }
    else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_DPCC_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_DPCC_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_DPCC_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "Dpcc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDpccHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResDpcc* dpcc_res = (RkAiqAlgoProcResDpcc*)mProcOutParam;

    rk_aiq_isp_dpcc_params_t* dpc_param = params->mDpccParams->data().ptr();

    if (!dpcc_res) {
        LOGD_ANALYZER("no dpcc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            dpc_param->frame_id = 0;
        } else {
            dpc_param->frame_id = shared->frameId;
        }

        if (dpcc_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            dpc_param->sync_flag = mSyncFlag;
            dpc_param->en = dpcc_res->res_com.en;
            dpc_param->bypass = dpcc_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mDpccParams = params->mDpccParams;
            dpc_param->is_update = true;
            LOGD_ADPCC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != dpc_param->sync_flag) {
            dpc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDpccParams.ptr()) {
                dpc_param->result = cur_params->mDpccParams->data()->result;
                dpc_param->en = cur_params->mDpccParams->data()->en;
                dpc_param->bypass = cur_params->mDpccParams->data()->bypass;
                dpc_param->is_update = true;
            } else {
                LOGE_ADPCC("no latest params !");
                dpc_param->is_update = false;
            }
            LOGD_ADPCC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            dpc_param->is_update = false;
            LOGD_ADPCC("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam

