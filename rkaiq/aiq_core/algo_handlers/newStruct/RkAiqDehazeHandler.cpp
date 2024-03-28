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

#include "RkAiqDehazeHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/dehaze/include/dehaze_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqDehazeHandleInt);

void RkAiqDehazeHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcDehaze());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResDehaze());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqDehazeHandleInt::setAttrib(dehaze_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_dehaze_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDehazeHandleInt::getAttrib(dehaze_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_dehaze_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDehazeHandleInt::queryStatus(dehaze_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mDehazeParams.ptr()) {
        rk_aiq_isp_dehaze_params_t* dehaze_param = mAiqCore->mAiqCurParams->data()->mDehazeParams->data().ptr();
        if (dehaze_param) {
            status->stMan = dehaze_param->result; 
            status->en = dehaze_param->en;
            status->bypass = dehaze_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ADEHAZE("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ADEHAZE("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDehazeHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "dehaze handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "dehaze algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqDehazeHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResDehaze* dehaze_proc_res_int =
            (RkAiqAlgoProcResDehaze*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "dehaze handle processing failed");
    }

    RkAiqAlgoProcDehaze* dehaze_proc_param = (RkAiqAlgoProcDehaze*)mProcInParam;
    RkAiqAdehazeStats* xDehazeStats = nullptr;
    if (shared->adehazeStatsBuf) {
        xDehazeStats = shared->adehazeStatsBuf;
        if (!xDehazeStats) LOGE_ADEHAZE("dehaze stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }
    if (!xDehazeStats || !xDehazeStats->adehaze_stats_valid) {
        LOGW_ADEHAZE("no adehaze stats, ignore!");
        dehaze_proc_param->stats_true = false;
    }
    else {
        dehaze_proc_param->stats_true = true;
#if RKAIQ_HAVE_DEHAZE_V12
        dehaze_proc_param->dehaze_stats_v12 = &xDehazeStats->adehaze_stats.dehaze_stats_v12;
#endif
#if RKAIQ_HAVE_DEHAZE_V14
        dehaze_proc_param->dehaze_stats_v14 = &xDehazeStats->adehaze_stats.dehaze_stats_v14;
#endif
    }

    if (shared->res_comb.aynrV22_proc_res || shared->res_comb.aynrV24_proc_res) {
        for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
#if RKAIQ_HAVE_YNR_V22
            dehaze_proc_param->sigma[i] = shared->res_comb.ynr_proc_res->dyn.loNrPost.hw_ynrC_luma2LoSgm_curve.val[i];
#elif RKAIQ_HAVE_YNR_V24
            dehaze_proc_param->sigma[i] = shared->res_comb.aynrV24_proc_res->stSelect->sigma[i];
#endif
    }
    else {
        for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
            dehaze_proc_param->sigma[i] = 0.0f;
    }
#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
    dehaze_proc_param->blc_ob_enable   = shared->res_comb.ablcV32_proc_res->blc_ob_enable;
    dehaze_proc_param->isp_ob_predgain = shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
#else
    dehaze_proc_param->blc_ob_enable   = false;
    dehaze_proc_param->isp_ob_predgain = 1.0f;
#endif

    dehaze_proc_res_int->dehazeRes = &shared->fullParams->mDehazeParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_DEHAZE_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_DEHAZE_PARAM;
        wrap_param.man_param_size = sizeof(dehaze_param_t);
        wrap_param.man_param_ptr = dehaze_proc_res_int->dehazeRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ADEHAZE("get new manual params success !");
            dehaze_proc_res_int->res_com.en = wrap_param.en;
            dehaze_proc_res_int->res_com.bypass = wrap_param.bypass;
            dehaze_proc_res_int->res_com.cfg_update = true;
        } else {
            dehaze_proc_res_int->res_com.cfg_update = false;
        }
    }
    else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_DEHAZE_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_DEHAZE_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_DEHAZE_PARAM);
        }
    }

    RKAIQCORE_CHECK_RET(ret, "dehaze algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqDehazeHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResDehaze* dehaze_res = (RkAiqAlgoProcResDehaze*)mProcOutParam;

    rk_aiq_isp_dehaze_params_t* dehaze_param = params->mDehazeParams->data().ptr();

    if (!dehaze_res) {
        LOGD_ANALYZER("no dehaze result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            dehaze_param->frame_id = 0;
        } else {
            dehaze_param->frame_id = shared->frameId;
        }

        if (dehaze_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            dehaze_param->sync_flag = mSyncFlag;
            dehaze_param->en = dehaze_res->res_com.en;
            dehaze_param->bypass = dehaze_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mDehazeParams = params->mDehazeParams;
            dehaze_param->is_update = true;
            LOGD_ADEHAZE("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != dehaze_param->sync_flag) {
            dehaze_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDehazeParams.ptr()) {
                dehaze_param->result = cur_params->mDehazeParams->data()->result;
                dehaze_param->en = cur_params->mDehazeParams->data()->en;
                dehaze_param->bypass = cur_params->mDehazeParams->data()->bypass;
                dehaze_param->is_update = true;
            } else {
                LOGE_ADEHAZE("no latest params !");
                dehaze_param->is_update = false;
            }
            LOGD_ADEHAZE("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            dehaze_param->is_update = false;
            LOGD_ADEHAZE("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
