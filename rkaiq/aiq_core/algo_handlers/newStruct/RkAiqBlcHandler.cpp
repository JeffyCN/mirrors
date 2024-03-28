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

#include "RkAiqBlcHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/blc/include/blc_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqBlcHandleInt);

void RkAiqBlcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResBlc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqBlcHandleInt::setAttrib(blc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_blc_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqBlcHandleInt::getAttrib(blc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_blc_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqBlcHandleInt::queryStatus(blc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mBlcParams.ptr()) {
        rk_aiq_isp_blc_params_t* blc_param = mAiqCore->mAiqCurParams->data()->mBlcParams->data().ptr();
        if (blc_param) {
            status->stMan = blc_param->result;
            status->en = blc_param->en;
            status->bypass = blc_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ABLC("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ABLC("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqBlcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "blc handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "blc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqBlcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResBlc* blc_proc_res_int =
            (RkAiqAlgoProcResBlc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "blc handle processing failed");
    }

    blc_proc_res_int->blcRes = &shared->fullParams->mBlcParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_BLC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_BLC_PARAM;
        wrap_param.man_param_size = sizeof(blc_param_t);
        wrap_param.man_param_ptr = blc_proc_res_int->blcRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ABLC("get new manual params success !");
            blc_proc_res_int->res_com.en = wrap_param.en;
            blc_proc_res_int->res_com.bypass = wrap_param.bypass;
            blc_proc_res_int->res_com.cfg_update = true;
        } else {
            blc_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        globalParamsManager->lockAlgoParam(RESULT_TYPE_BLC_PARAM);
        mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_BLC_PARAM);

        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        ret                       = des->processing(mProcInParam, mProcOutParam);
        globalParamsManager->unlockAlgoParam(RESULT_TYPE_BLC_PARAM);
    }

    if (blc_proc_res_int->res_com.cfg_update) {
        shared->res_comb.blc_en       = blc_proc_res_int->res_com.en;
        shared->res_comb.blc_proc_res = blc_proc_res_int->blcRes;
    } else {
        shared->res_comb.blc_en       = mLatesten;
        shared->res_comb.blc_proc_res = mLatestparam;
    }

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqBlcHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResBlc* blc_res = (RkAiqAlgoProcResBlc*)mProcOutParam;

    rk_aiq_isp_blc_params_t* blc_param = params->mBlcParams->data().ptr();

    if (!blc_res) {
        LOGD_ANALYZER("no blc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            blc_param->frame_id = 0;
        } else {
            blc_param->frame_id = shared->frameId;
        }

        if (blc_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            blc_param->sync_flag = mSyncFlag;
            blc_param->en = blc_res->res_com.en;
            blc_param->bypass = blc_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mBlcParams = params->mBlcParams;
            mLatestparam = &cur_params->mBlcParams->data()->result;
            mLatesten    = blc_param->en;
            blc_param->is_update = true;
            LOGD_ABLC("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != blc_param->sync_flag) {
            blc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mBlcParams.ptr()) {
                blc_param->result = cur_params->mBlcParams->data()->result;
                blc_param->en = cur_params->mBlcParams->data()->en;
                blc_param->bypass = cur_params->mBlcParams->data()->bypass;
                blc_param->is_update = true;
            } else {
                LOGE_ABLC("no latest params !");
                blc_param->is_update = false;
            }
            LOGD_ABLC("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            blc_param->is_update = false;
            LOGD_ABLC("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
