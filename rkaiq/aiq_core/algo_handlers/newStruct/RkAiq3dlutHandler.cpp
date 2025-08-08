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

#include "RkAiq3dlutHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/3dlut/include/3dlut_algo_api.h"
#include "newStruct/algo_common.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiq3dlutHandleInt);

void RkAiq3dlutHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcLut3d());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoResCom());

    mResultType = RESULT_TYPE_LUT3D_PARAM;
    mResultSize = sizeof(lut3d_param_t);

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiq3dlutHandleInt::setAttrib(lut3d_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_lut3d_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiq3dlutHandleInt::getAttrib(lut3d_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_lut3d_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiq3dlutHandleInt::queryStatus(lut3d_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mLut3dParams.ptr()) {
        rk_aiq_isp_lut3d_params_t* lut3d_param = mAiqCore->mAiqCurParams->data()->mLut3dParams->data().ptr();
        if (lut3d_param) {
            status->stMan = lut3d_param->result; 
            status->en = lut3d_param->en;
            status->bypass = lut3d_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACSM("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACSM("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiq3dlutHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "lut3d handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "lut3d algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiq3dlutHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "lut3d handle processing failed");
    }

    proc_res->algoRes =  &shared->fullParams->mLut3dParams->data()->result;

    RkAiqAlgoProcLut3d* proc_int        = (RkAiqAlgoProcLut3d*)mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    XCamVideoBuffer* awb_proc_res = shared->res_comb.awb_proc_res;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->map(awb_proc_res);
    }
    get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = RkAiqHandle::do_processing_common();

    RKAIQCORE_CHECK_RET(ret, "lut3d algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiq3dlutHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoResCom* proc_res = mProcOutParam;

    rk_aiq_isp_lut3d_params_t* lut3d_param = params->mLut3dParams->data().ptr();

    if (!proc_res) {
        LOGD_ANALYZER("no lut3d result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            lut3d_param->frame_id = 0;
        } else {
            lut3d_param->frame_id = shared->frameId;
        }

        if (proc_res->cfg_update) {
            mSyncFlag = shared->frameId;
            lut3d_param->sync_flag = mSyncFlag;
            lut3d_param->en = proc_res->en;
            lut3d_param->bypass = proc_res->bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mLut3dParams = params->mLut3dParams;
            lut3d_param->is_update = true;
            LOGD_ACSM("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != lut3d_param->sync_flag) {
            lut3d_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mLut3dParams.ptr()) {
                lut3d_param->result = cur_params->mLut3dParams->data()->result;
                lut3d_param->en = cur_params->mLut3dParams->data()->en;
                lut3d_param->bypass = cur_params->mLut3dParams->data()->bypass;
                lut3d_param->is_update = true;
            } else {
                LOGE_ACSM("no latest params !");
                lut3d_param->is_update = false;
            }
            LOGD_ACSM("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            lut3d_param->is_update = false;
            LOGD_ACSM("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
