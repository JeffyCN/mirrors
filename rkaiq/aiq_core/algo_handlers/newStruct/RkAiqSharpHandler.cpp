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

#include "RkAiqSharpHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/sharp/include/sharp_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqSharpHandleInt);

void RkAiqSharpHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcSharp());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResSharp());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqSharpHandleInt::setAttrib(sharp_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_sharp_SetAttrib(mAlgoCtx, attr, false);
    mCfgMutex.unlock();

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqSharpHandleInt::getAttrib(sharp_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_sharp_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqSharpHandleInt::queryStatus(sharp_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mSharpParams.ptr()) {
        rk_aiq_isp_sharp_params_t* sharp_param = mAiqCore->mAiqCurParams->data()->mSharpParams->data().ptr();
        if (sharp_param) {
            status->stMan = sharp_param->result;
            status->en = sharp_param->en;
            status->bypass = sharp_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqSharpHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "sharp handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "sharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqSharpHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "sharp handle processing failed");
    }

#if defined(ISP_HW_V39)
    RkAiqAlgoProcSharp* sharp_proc_param = (RkAiqAlgoProcSharp*)mProcInParam;
    sharp_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcSharp* sharp_proc_param = (RkAiqAlgoProcSharp*)mProcInParam;
    sharp_proc_param->blc_ob_predgain = ob_predgain;
#endif

    RkAiqAlgoProcResSharp* sharp_proc_res_int = (RkAiqAlgoProcResSharp*)mProcOutParam;
    sharp_proc_res_int->sharpRes =  &shared->fullParams->mSharpParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_SHARPEN_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_SHARPEN_PARAM;
        wrap_param.man_param_size = sizeof(sharp_param_t);
        wrap_param.man_param_ptr = sharp_proc_res_int->sharpRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ASHARP("get new sharp manual params success !");
            sharp_proc_res_int->res_com.en = wrap_param.en;
            sharp_proc_res_int->res_com.bypass = wrap_param.bypass;
            sharp_proc_res_int->res_com.cfg_update = true;
        } else {
            sharp_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_SHARPEN_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_SHARPEN_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_SHARPEN_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "sharp algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqSharpHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResSharp* sharp_res = (RkAiqAlgoProcResSharp*)mProcOutParam;

    rk_aiq_isp_sharp_params_t* sharp_param = params->mSharpParams->data().ptr();

    if (!sharp_res) {
        LOGE_ASHARP("no sharp result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            sharp_param->frame_id = 0;
        } else {
            sharp_param->frame_id = shared->frameId;
        }

        if (sharp_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            sharp_param->sync_flag = mSyncFlag;
            sharp_param->en = sharp_res->res_com.en;
            sharp_param->bypass = sharp_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mSharpParams = params->mSharpParams;

            sharp_param->is_update = true;
            LOGD_ASHARP("Sharp [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != sharp_param->sync_flag) {
            sharp_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mSharpParams.ptr()) {
                sharp_param->result = cur_params->mSharpParams->data()->result;
                sharp_param->en = cur_params->mSharpParams->data()->en;
                sharp_param->bypass = cur_params->mSharpParams->data()->bypass;
                sharp_param->is_update = true;
            } else {
                LOGE_ASHARP("Sharp: no latest params !");
                sharp_param->is_update = false;
            }
            LOGD_ASHARP("Sharp [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            sharp_param->is_update = false;
            LOGD_ASHARP("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
