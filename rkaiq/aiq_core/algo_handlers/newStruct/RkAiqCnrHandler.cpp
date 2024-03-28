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

#include "RkAiqCnrHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/cnr/include/cnr_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCnrHandleInt);

void RkAiqCnrHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcCnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResCnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCnrHandleInt::setAttrib(cnr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCnrHandleInt::getAttrib(cnr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_cnr_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCnrHandleInt::queryStatus(cnr_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCnrParams.ptr()) {
        rk_aiq_isp_cnr_params_t* cnr_param = mAiqCore->mAiqCurParams->data()->mCnrParams->data().ptr();
        if (cnr_param) {
            status->stMan = cnr_param->result;
            status->en = cnr_param->en;
            status->bypass = cnr_param->bypass;
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

XCamReturn RkAiqCnrHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "cnr handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "cnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCnrHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "cnr handle processing failed");
    }

#if defined(ISP_HW_V39)
    RkAiqAlgoProcCnr* cnr_proc_param = (RkAiqAlgoProcCnr*)mProcInParam;
    cnr_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcCnr* cnr_proc_param = (RkAiqAlgoProcCnr*)mProcInParam;
    cnr_proc_param->blc_ob_predgain = ob_predgain;
#endif

    RkAiqAlgoProcResCnr* cnr_proc_res_int = (RkAiqAlgoProcResCnr*)mProcOutParam;
    cnr_proc_res_int->cnrRes =  &shared->fullParams->mCnrParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_UVNR_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_UVNR_PARAM;
        wrap_param.man_param_size = sizeof(cnr_param_t);
        wrap_param.man_param_ptr = cnr_proc_res_int->cnrRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new cnr manual params success !");
            cnr_proc_res_int->res_com.en = wrap_param.en;
            cnr_proc_res_int->res_com.bypass = wrap_param.bypass;
            cnr_proc_res_int->res_com.cfg_update = true;
        } else {
            cnr_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_UVNR_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_UVNR_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_UVNR_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "cnr algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqCnrHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResCnr* cnr_res = (RkAiqAlgoProcResCnr*)mProcOutParam;

    rk_aiq_isp_cnr_params_t* cnr_param = params->mCnrParams->data().ptr();

    if (!cnr_res) {
        LOGE_ANR("no cnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            cnr_param->frame_id = 0;
        } else {
            cnr_param->frame_id = shared->frameId;
        }

        if (cnr_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            cnr_param->sync_flag = mSyncFlag;
            cnr_param->en = cnr_res->res_com.en;
            cnr_param->bypass = cnr_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mCnrParams = params->mCnrParams;

            cnr_param->is_update = true;
            LOGD_ANR("Cnr [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != cnr_param->sync_flag) {
            cnr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCnrParams.ptr()) {
                cnr_param->result = cur_params->mCnrParams->data()->result;
                cnr_param->en = cur_params->mCnrParams->data()->en;
                cnr_param->bypass = cur_params->mCnrParams->data()->bypass;
                cnr_param->is_update = true;
            } else {
                LOGE_ANR("Cnr: no latest params !");
                cnr_param->is_update = false;
            }
            LOGD_ANR("Cnr [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            cnr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
