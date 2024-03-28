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

#include "RkAiqYnrHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/ynr/include/ynr_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqYnrHandleInt);

void RkAiqYnrHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcYnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResYnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqYnrHandleInt::setAttrib(ynr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqYnrHandleInt::getAttrib(ynr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_ynr_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqYnrHandleInt::queryStatus(ynr_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mYnrParams.ptr()) {
        rk_aiq_isp_ynr_params_t* ynr_param = mAiqCore->mAiqCurParams->data()->mYnrParams->data().ptr();
        if (ynr_param) {
            status->stMan = ynr_param->result;
            status->en = ynr_param->en;
            status->bypass = ynr_param->bypass;
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

XCamReturn RkAiqYnrHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ynr handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ynr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqYnrHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ynr handle processing failed");
    }

#if defined(ISP_HW_V39)
    RkAiqAlgoProcYnr* ynr_proc_param = (RkAiqAlgoProcYnr*)mProcInParam;
    ynr_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcYnr* ynr_proc_param = (RkAiqAlgoProcYnr*)mProcInParam;
    ynr_proc_param->blc_ob_predgain = ob_predgain;
#endif

    RkAiqAlgoProcResYnr* ynr_proc_res_int = (RkAiqAlgoProcResYnr*)mProcOutParam;
    ynr_proc_res_int->ynrRes =  &shared->fullParams->mYnrParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_YNR_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_YNR_PARAM;
        wrap_param.man_param_size = sizeof(ynr_param_t);
        wrap_param.man_param_ptr = ynr_proc_res_int->ynrRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new ynr manual params success !");
            ynr_proc_res_int->res_com.en = wrap_param.en;
            ynr_proc_res_int->res_com.bypass = wrap_param.bypass;
            ynr_proc_res_int->res_com.cfg_update = true;
        } else {
            ynr_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_YNR_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_YNR_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_YNR_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "ynr algo processing failed");

    int frame_id = shared->frameId;
    SmartPtr<RkAiqAlgoProcResYnrIntShared> bp = new RkAiqAlgoProcResYnrIntShared;
    bp->set_sequence(frame_id);
    bp->result.ynrRes = ynr_proc_res_int->ynrRes;
    RkAiqCoreVdBufMsg msg(XCAM_MESSAGE_YNR_PROC_RES_OK, frame_id, bp);
    mAiqCore->post_message(msg);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqYnrHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResYnr* ynr_res = (RkAiqAlgoProcResYnr*)mProcOutParam;

    rk_aiq_isp_ynr_params_t* ynr_param = params->mYnrParams->data().ptr();

    if (!ynr_res) {
        LOGE_ANR("no ynr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            ynr_param->frame_id = 0;
        } else {
            ynr_param->frame_id = shared->frameId;
        }

        if (ynr_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            ynr_param->sync_flag = mSyncFlag;
            ynr_param->en = ynr_res->res_com.en;
            ynr_param->bypass = ynr_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mYnrParams = params->mYnrParams;

            ynr_param->is_update = true;
            LOGD_ANR("Ynr [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != ynr_param->sync_flag) {
            ynr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mYnrParams.ptr()) {
                ynr_param->result = cur_params->mYnrParams->data()->result;
                ynr_param->en = cur_params->mYnrParams->data()->en;
                ynr_param->bypass = cur_params->mYnrParams->data()->bypass;
                ynr_param->is_update = true;
            } else {
                LOGE_ANR("Ynr: no latest params !");
                ynr_param->is_update = false;
            }
            LOGD_ANR("Ynr [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            ynr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
