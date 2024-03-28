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

#include "RkAiqBtnrHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/bayertnr/include/bayertnr_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqBtnrHandleInt);

void RkAiqBtnrHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcBtnr());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResBtnr());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqBtnrHandleInt::setAttrib(btnr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_bayertnr_SetAttrib(mAlgoCtx, attr, false);
    mCfgMutex.unlock();

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqBtnrHandleInt::getAttrib(btnr_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_bayertnr_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqBtnrHandleInt::queryStatus(btnr_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mBtnrParams.ptr()) {
        rk_aiq_isp_btnr_params_t* btnr_param = mAiqCore->mAiqCurParams->data()->mBtnrParams->data().ptr();
        if (btnr_param) {
            status->stMan = btnr_param->result;
            status->en = btnr_param->en;
            status->bypass = btnr_param->bypass;
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

XCamReturn RkAiqBtnrHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "btnr handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "btnr algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqBtnrHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "btnr handle processing failed");
    }

#if defined(ISP_HW_V39)
    RkAiqAlgoProcBtnr* btnr_proc_param = (RkAiqAlgoProcBtnr*)mProcInParam;
    btnr_proc_param->blc_ob_predgain = 1.0;
#else
    AblcProc_V32_t* blc_res = shared->res_comb.ablcV32_proc_res;
    float ob_predgain = blc_res->isp_ob_predgain;
    RkAiqAlgoProcBtnr* btnr_proc_param = (RkAiqAlgoProcBtnr*)mProcInParam;
    btnr_proc_param->blc_ob_predgain = ob_predgain;
#endif

    RkAiqAlgoProcResBtnr* btnr_proc_res_int = (RkAiqAlgoProcResBtnr*)mProcOutParam;
    btnr_proc_res_int->btnrRes =  &shared->fullParams->mBtnrParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_TNR_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_TNR_PARAM;
        wrap_param.man_param_size = sizeof(btnr_param_t);
        wrap_param.man_param_ptr = btnr_proc_res_int->btnrRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new btnr manual params success !");
            btnr_proc_res_int->res_com.en = wrap_param.en;
            btnr_proc_res_int->res_com.bypass = wrap_param.bypass;
            btnr_proc_res_int->res_com.cfg_update = true;
        } else {
            btnr_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_TNR_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_TNR_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_TNR_PARAM);
        }
    }

    if (btnr_proc_res_int->res_com.cfg_update) {
        shared->res_comb.bayernr3d_en = btnr_proc_res_int->res_com.en;
    }

    RKAIQCORE_CHECK_RET(ret, "btnr algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqBtnrHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResBtnr* btnr_res = (RkAiqAlgoProcResBtnr*)mProcOutParam;

    rk_aiq_isp_btnr_params_t* btnr_param = params->mBtnrParams->data().ptr();

    if (!btnr_res) {
        LOGE_ANR("no btnr result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            btnr_param->frame_id = 0;
        } else {
            btnr_param->frame_id = shared->frameId;
        }

        if (btnr_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            btnr_param->sync_flag = mSyncFlag;
            btnr_param->en = btnr_res->res_com.en;
            btnr_param->bypass = btnr_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mBtnrParams = params->mBtnrParams;

            btnr_param->is_update = true;
            LOGD_ANR("Btnr [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != btnr_param->sync_flag) {
            btnr_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mBtnrParams.ptr()) {
                btnr_param->result = cur_params->mBtnrParams->data()->result;
                btnr_param->en = cur_params->mBtnrParams->data()->en;
                btnr_param->bypass = cur_params->mBtnrParams->data()->bypass;
                btnr_param->is_update = true;
            } else {
                LOGE_ANR("Btnr: no latest params !");
                btnr_param->is_update = false;
            }
            LOGD_ANR("Btnr [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            btnr_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
