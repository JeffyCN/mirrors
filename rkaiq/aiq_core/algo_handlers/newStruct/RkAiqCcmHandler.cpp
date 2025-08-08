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

#include "RkAiqCcmHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/ccm/include/ccm_algo_api.h"
#include "newStruct/algo_common.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqCcmHandleInt);

void RkAiqCcmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcCcm());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoResCom());

    mResultType = RESULT_TYPE_CCM_PARAM;
    mResultSize = sizeof(ccm_param_t);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqCcmHandleInt::setAttrib(ccm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_ccm_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCcmHandleInt::getAttrib(ccm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_ccm_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCcmHandleInt::queryStatus(ccm_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mCcmParams.ptr()) {
        rk_aiq_isp_ccm_params_t* ccm_param = mAiqCore->mAiqCurParams->data()->mCcmParams->data().ptr();
        if (ccm_param) {
            status->ccmStatus = ccm_param->result; 
            status->en = ccm_param->en;
            status->bypass = ccm_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACCM("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACCM("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqCcmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ccm handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ccm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqCcmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ccm handle processing failed");
    }

    mProcOutParam->algoRes =  &shared->fullParams->mCcmParams->data()->result;

    RkAiqAlgoProcCcm* ccm_proc_int        = (RkAiqAlgoProcCcm*)mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    XCamVideoBuffer* awb_proc_res = shared->res_comb.awb_proc_res;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->map(awb_proc_res);
    }
    get_illu_estm_info(&ccm_proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = RkAiqHandle::do_processing_common();

    RKAIQCORE_CHECK_RET(ret, "ccm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqCcmHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoResCom* proc_res = mProcOutParam;

    rk_aiq_isp_ccm_params_t* ccm_param = params->mCcmParams->data().ptr();

    if (!proc_res) {
        LOGD_ANALYZER("no ccm result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            ccm_param->frame_id = 0;
        } else {
            ccm_param->frame_id = shared->frameId;
        }

        if (proc_res->cfg_update) {
            mSyncFlag = shared->frameId;
            ccm_param->sync_flag = mSyncFlag;
            ccm_param->en = proc_res->en;
            ccm_param->bypass = proc_res->bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mCcmParams = params->mCcmParams;
            ccm_param->is_update = true;
            LOGD_ACCM("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != ccm_param->sync_flag) {
            ccm_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mCcmParams.ptr()) {
                ccm_param->result = cur_params->mCcmParams->data()->result;
                ccm_param->en = cur_params->mCcmParams->data()->en;
                ccm_param->bypass = cur_params->mCcmParams->data()->bypass;
                ccm_param->is_update = true;
            } else {
                LOGE_ACCM("no latest params !");
                ccm_param->is_update = false;
            }
            LOGD_ACCM("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            ccm_param->is_update = false;
            LOGD_ACCM("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
