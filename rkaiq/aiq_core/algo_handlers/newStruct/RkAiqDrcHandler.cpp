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

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "drc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

void RkAiqDrcHandleInt::DrcProchelper(RkAiqAlgoProcDrc* drc_proc_param) {

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    FrameNumber_t FrameNumber;
    unsigned char compr_bit = sharedCom->snsDes.compr_bit;
    if (sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        FrameNumber = LINEAR_NUM;
    else if (sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        FrameNumber = HDR_2X_NUM;
    else
        FrameNumber = HDR_3X_NUM;
    if (compr_bit) {
        FrameNumber = SENSOR_MGE;
        if (compr_bit > ISP_HDR_BIT_NUM_MAX)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) > %d!!!\n", __FUNCTION__, compr_bit,
                      ISP_HDR_BIT_NUM_MAX);
        if (compr_bit < ISP_HDR_BIT_NUM_MIN)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) < %d!!!\n", __FUNCTION__, compr_bit,
                      ISP_HDR_BIT_NUM_MIN);
        compr_bit =
            CLIP(compr_bit, ISP_HDR_BIT_NUM_MAX, ISP_HDR_BIT_NUM_MIN);
    }
    drc_proc_param->FrameNumber = FrameNumber;
    shared->fullParams->mDrcParams->data()->result.compr_bit = compr_bit;

    drc_proc_param->NextData.AEData.LongFrmMode = drc_proc_param->LongFrmMode;
    if (FrameNumber == LINEAR_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.MExpo = drc_proc_param->NextData.AEData.SExpo;
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.SExpo;
    }
    else if(FrameNumber == HDR_2X_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.MExpo;
    }
    else if(FrameNumber == HDR_3X_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.LExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.LExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
        }
    } else if (FrameNumber == SENSOR_MGE) {
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.MExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.MExpo;
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->NextData.AEData.MExpo /
            pow(2.0f, (float)(compr_bit - ISP_HDR_BIT_NUM_MIN));
    }
    if (drc_proc_param->NextData.AEData.SExpo > FLT_EPSILON) {
        drc_proc_param->NextData.AEData.L2S_Ratio =
            drc_proc_param->NextData.AEData.LExpo / drc_proc_param->NextData.AEData.SExpo;
        if (drc_proc_param->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.L2S_Ratio);
            drc_proc_param->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
        }
        drc_proc_param->NextData.AEData.M2S_Ratio =
            drc_proc_param->NextData.AEData.MExpo / drc_proc_param->NextData.AEData.SExpo;
        if (drc_proc_param->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.M2S_Ratio);
            drc_proc_param->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
        }
    }
    else
        LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.SExpo);
    if (drc_proc_param->NextData.AEData.MExpo > FLT_EPSILON) {
        drc_proc_param->NextData.AEData.L2M_Ratio =
            drc_proc_param->NextData.AEData.LExpo / drc_proc_param->NextData.AEData.MExpo;
        if (drc_proc_param->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.L2M_Ratio);
            drc_proc_param->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.MExpo);
    //clip for long frame mode
    if (drc_proc_param->NextData.AEData.LongFrmMode) {
        drc_proc_param->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
        drc_proc_param->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
        drc_proc_param->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
    }
    // clip L2M_ratio to 32x
    if (drc_proc_param->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
        LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2M_Ratio);
        drc_proc_param->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
    }
    // clip L2S_ratio
    if (drc_proc_param->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2S_Ratio);
        drc_proc_param->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
    }
    // clip L2M_ratio and M2S_Ratio
    if (drc_proc_param->NextData.AEData.L2M_Ratio * drc_proc_param->NextData.AEData.M2S_Ratio >
        AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n",
                    __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2M_Ratio * drc_proc_param->NextData.AEData.M2S_Ratio);
        drc_proc_param->NextData.AEData.M2S_Ratio =
            AE_RATIO_MAX / drc_proc_param->NextData.AEData.L2M_Ratio;
    }
    shared->fullParams->mDrcParams->data()->result.L2S_Ratio = drc_proc_param->NextData.AEData.L2S_Ratio;
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

    DrcProchelper(drc_proc_param);
    drc_proc_res_int->drcRes = &shared->fullParams->mDrcParams->data()->result.drc_param;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();
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

    rk_aiq_global_params_wrap_t params;
    trans_api_attrib_t trans_attr;
    params.type = RESULT_TYPE_TRANS_PARAM;
    params.man_param_size = sizeof(trans_param_t);
    params.man_param_ptr = &trans_attr.stMan;
    ret = globalParamsManager->get(&params);
    trans_attr.en = params.en;
    trans_attr.bypass = params.bypass;
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
