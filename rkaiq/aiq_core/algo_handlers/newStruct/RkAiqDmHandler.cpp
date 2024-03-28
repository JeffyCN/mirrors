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

#include "RkAiqDmHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/demosaic/include/demosaic_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqDmHandleInt);

void RkAiqDmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResDm());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqDmHandleInt::setAttrib(dm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_demosaic_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDmHandleInt::getAttrib(dm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_demosaic_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDmHandleInt::queryStatus(dm_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mDmParams.ptr()) {
        rk_aiq_isp_dm_params_t* dm_param = mAiqCore->mAiqCurParams->data()->mDmParams->data().ptr();
        if (dm_param) {
            status->stMan = dm_param->result; 
            status->en = dm_param->en;
            status->bypass = dm_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADEBAYER("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADEBAYER("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adebayer handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adebayer algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqDmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResDm* dm_proc_res_int =
            (RkAiqAlgoProcResDm*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adebayer handle processing failed");
    }

    dm_proc_res_int->dmRes =  &shared->fullParams->mDmParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_DEBAYER_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_DEBAYER_PARAM;
        wrap_param.man_param_size = sizeof(dm_param_t);
        wrap_param.man_param_ptr = dm_proc_res_int->dmRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ADEBAYER("get new manual params success !");
            dm_proc_res_int->res_com.en = wrap_param.en;
            dm_proc_res_int->res_com.bypass = wrap_param.bypass;
            dm_proc_res_int->res_com.cfg_update = true;
        } else {
            dm_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        globalParamsManager->lockAlgoParam(RESULT_TYPE_DEBAYER_PARAM);
        mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_DEBAYER_PARAM);

        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        ret                       = des->processing(mProcInParam, mProcOutParam);
        globalParamsManager->unlockAlgoParam(RESULT_TYPE_DEBAYER_PARAM);
    }

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqDmHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResDm* dm_res = (RkAiqAlgoProcResDm*)mProcOutParam;

    rk_aiq_isp_dm_params_t* dm_param = params->mDmParams->data().ptr();

    if (!dm_res) {
        LOGD_ANALYZER("no adebayer result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            dm_param->frame_id = 0;
        } else {
            dm_param->frame_id = shared->frameId;
        }

        if (dm_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            dm_param->sync_flag = mSyncFlag;
            dm_param->en = dm_res->res_com.en;
            dm_param->bypass = dm_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mDmParams = params->mDmParams;
            dm_param->is_update = true;
            LOGD_ADEBAYER("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != dm_param->sync_flag) {
            dm_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDmParams.ptr()) {
                dm_param->result = cur_params->mDmParams->data()->result;
                dm_param->en = cur_params->mDmParams->data()->en;
                dm_param->bypass = cur_params->mDmParams->data()->bypass;
                dm_param->is_update = true;
            } else {
                LOGE_ADEBAYER("no latest params !");
                dm_param->is_update = false;
            }
            LOGD_ADEBAYER("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            dm_param->is_update = false;
            LOGD_ADEBAYER("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
