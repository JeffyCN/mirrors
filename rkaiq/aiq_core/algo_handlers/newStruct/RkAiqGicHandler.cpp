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

#include "RkAiqGicHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/gic/include/gic_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqGicHandleInt);

void RkAiqGicHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResGic());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqGicHandleInt::setAttrib(gic_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mIsUpdateGrpAttr = true;
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGicHandleInt::getAttrib(gic_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();
    ret = algo_gic_GetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqGicHandleInt::queryStatus(gic_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mGicParams.ptr()) {
        rk_aiq_isp_gic_params_t* gic_param = mAiqCore->mAiqCurParams->data()->mGicParams->data().ptr();
        if (gic_param) {
            status->stMan = gic_param->result;
            status->en = gic_param->en;
            status->bypass = gic_param->bypass;
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

XCamReturn RkAiqGicHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "gic handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "gic algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqGicHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "gic handle processing failed");
    }

    RkAiqAlgoProcResGic* gic_proc_res_int = (RkAiqAlgoProcResGic*)mProcOutParam;
    gic_proc_res_int->gicRes =  &shared->fullParams->mGicParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_GIC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_GIC_PARAM;
        wrap_param.man_param_size = sizeof(gic_param_t);
        wrap_param.man_param_ptr = gic_proc_res_int->gicRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ANR("get new gic manual params success !");
            gic_proc_res_int->res_com.en = wrap_param.en;
            gic_proc_res_int->res_com.bypass = wrap_param.bypass;
            gic_proc_res_int->res_com.cfg_update = true;
        } else {
            gic_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_GIC_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_GIC_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_GIC_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "gic algo processing failed");
    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqGicHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoProcResGic* gic_res = (RkAiqAlgoProcResGic*)mProcOutParam;

    rk_aiq_isp_gic_params_t* gic_param = params->mGicParams->data().ptr();

    if (!gic_res) {
        LOGE_ANR("no gic result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (mAiqCore->isGroupAlgo(getAlgoType())) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            gic_param->frame_id = 0;
        } else {
            gic_param->frame_id = shared->frameId;
        }

        if (gic_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            gic_param->sync_flag = mSyncFlag;
            gic_param->en = gic_res->res_com.en;
            gic_param->bypass = gic_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mGicParams = params->mGicParams;

            gic_param->is_update = true;
            LOGD_ANR("Gic [%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != gic_param->sync_flag) {
            gic_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mGicParams.ptr()) {
                gic_param->result = cur_params->mGicParams->data()->result;
                gic_param->en = cur_params->mGicParams->data()->en;
                gic_param->bypass = cur_params->mGicParams->data()->bypass;
                gic_param->is_update = true;
            } else {
                LOGE_ANR("Gic: no latest params !");
                gic_param->is_update = false;
            }
            LOGD_ANR("Gic [%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            gic_param->is_update = false;
            LOGD_ANR("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
