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

#include "RkAiqDrcHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/drc/include/drc_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqDrcHandleInt);

void RkAiqDrcHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigDrc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdrc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResDrc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqDrcHandleInt::setAttrib(drc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_drc_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDrcHandleInt::getAttrib(drc_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_drc_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDrcHandleInt::queryStatus(drc_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mDrcParams.ptr()) {
        rk_aiq_isp_drc_params_t* drc_param = mAiqCore->mAiqCurParams->data()->mDrcParams->data().ptr();
        if (drc_param) {
            status->stMan = drc_param->result.drc_param;
            status->en = drc_param->en;
            status->bypass = drc_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ATMO("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ATMO("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqDrcHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "drc handle prepare failed");
    RkAiqAlgoConfigDrc* adrc_config_int     = (RkAiqAlgoConfigDrc*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adrc_config_int->compr_bit    = sharedCom->snsDes.compr_bit;
    adrc_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "drc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqDrcHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResDrc* drc_proc_res_int =
            (RkAiqAlgoProcResDrc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "drc handle processing failed");
    }

#if defined(ISP_HW_V39)
    RkAiqAlgoProcDrc* drc_proc_param = (RkAiqAlgoProcDrc*)mProcInParam;
    drc_proc_param->isp_ob_predgain = 1.0;
#else
    RkAiqAlgoProcDrc* drc_proc_param = (RkAiqAlgoProcDrc*)mProcInParam;
    drc_proc_param->LongFrmMode = mAeProcRes.LongFrmMode;
    drc_proc_param->blc_ob_enable = shared->res_comb.ablcV32_proc_res->blc_ob_enable;
    drc_proc_param->isp_ob_predgain = shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
#endif

    drc_proc_res_int->drcRes = &shared->fullParams->mDrcParams->data()->result.drc_param;
    // drc_proc_res_int->drcRes = &shared->fullParams->mDrcParams->data()->result.trans_param;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();
    rk_aiq_global_params_wrap_t params;
    trans_api_attrib_t trans_attr;
    params.type = RESULT_TYPE_TRANS_PARAM;
    params.man_param_size = sizeof(trans_param_t);
    params.man_param_ptr = &trans_attr.stMan;
    ret = globalParamsManager->get(&params);
    drc_proc_param->hw_transCfg_trans_mode = trans_attr.stMan.sta.hw_transCfg_trans_mode;
    drc_proc_param->hw_transCfg_transOfDrc_offset = trans_attr.stMan.sta.hw_transCfg_transOfDrc_offset;

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_DRC_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_DRC_PARAM;
        wrap_param.man_param_size = sizeof(drc_param_t);
        wrap_param.man_param_ptr = drc_proc_res_int->drcRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ATMO("get new manual params success !");
            drc_proc_res_int->res_com.en = wrap_param.en;
            drc_proc_res_int->res_com.bypass = wrap_param.bypass;
            drc_proc_res_int->res_com.cfg_update = true;
        } else {
            drc_proc_res_int->res_com.cfg_update = false;
        }
    }
    else {
        // skip processing if is group algo
        if (!mAiqCore->isGroupAlgo(getAlgoType())) {
            globalParamsManager->lockAlgoParam(RESULT_TYPE_DRC_PARAM);
            mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_DRC_PARAM);

            RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
            ret                       = des->processing(mProcInParam, mProcOutParam);
            globalParamsManager->unlockAlgoParam(RESULT_TYPE_DRC_PARAM);
        }
    }

    XCamReturn ret1 = globalParamsManager->getAndClearPending(&params);
    trans_api_attrib_t* trans_curAttr = &shared->fullParams->mDrcParams->data()->result.trans_attr;
    *trans_curAttr = trans_attr;
    if (ret1 == XCAM_RETURN_NO_ERROR) {
        drc_proc_res_int->res_com.cfg_update = true;
        LOGD_ATMO("trans params update");
    }

    RKAIQCORE_CHECK_RET(ret, "drc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqDrcHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResDrc* drc_res = (RkAiqAlgoProcResDrc*)mProcOutParam;

    rk_aiq_isp_drc_params_t* drc_param = params->mDrcParams->data().ptr();

    if (!drc_res) {
        LOGD_ANALYZER("no drc result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            drc_param->frame_id = 0;
        } else {
            drc_param->frame_id = shared->frameId;
        }

        if (drc_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            drc_param->sync_flag = mSyncFlag;
            drc_param->en = drc_res->res_com.en;
            drc_param->bypass = drc_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mDrcParams = params->mDrcParams;
            drc_param->is_update = true;
            LOGD_ATMO("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != drc_param->sync_flag) {
            drc_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mDrcParams.ptr()) {
                drc_param->result = cur_params->mDrcParams->data()->result;
                drc_param->en = cur_params->mDrcParams->data()->en;
                drc_param->bypass = cur_params->mDrcParams->data()->bypass;
                drc_param->is_update = true;
            } else {
                LOGE_ATMO("no latest params !");
                drc_param->is_update = false;
            }
            LOGD_ATMO("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            drc_param->is_update = false;
            LOGD_ATMO("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
