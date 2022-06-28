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
#include "RkAiqAccmHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAccmHandleInt);

void RkAiqAccmHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAccm());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAccm());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAccm());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAccm());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAccm());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAccm());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAccm());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAccmHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        // TODO
        rk_aiq_uapi_accm_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::setAttrib(rk_aiq_ccm_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
    bool isChanged = false;
    if (att.sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, &att, sizeof(att)))
        isChanged = true;
    else if (att.sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, &att, sizeof(att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal(att.sync.sync_mode);
    }

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::getAttrib(rk_aiq_ccm_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_accm_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::queryCcmInfo(rk_aiq_ccm_querry_info_t* ccm_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_accm_QueryCcmInfo(mAlgoCtx, ccm_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "accm handle prepare failed");

    RkAiqAlgoConfigAccm* accm_config_int = (RkAiqAlgoConfigAccm*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "accm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAccmHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAccm* accm_pre_int        = (RkAiqAlgoPreAccm*)mPreInParam;
    RkAiqAlgoPreResAccm* accm_pre_res_int = (RkAiqAlgoPreResAccm*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAccmHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAccm* accm_proc_int        = (RkAiqAlgoProcAccm*)mProcInParam;
    RkAiqAlgoProcResAccm* accm_proc_res_int = (RkAiqAlgoProcResAccm*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle processing failed");
    }

    // TODO should check if the rk awb algo used
    XCamVideoBuffer* xCamAwbProcRes = shared->res_comb.awb_proc_res;
    if (xCamAwbProcRes) {
        RkAiqAlgoProcResAwb* awb_res =
            (RkAiqAlgoProcResAwb*)xCamAwbProcRes->map(xCamAwbProcRes);
        if (awb_res) {
            if (awb_res->awb_gain_algo.grgain < DIVMIN || awb_res->awb_gain_algo.gbgain < DIVMIN) {
                LOGW("get wrong awb gain from AWB module ,use default value ");
            } else {
                accm_proc_int->accm_sw_info.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                accm_proc_int->accm_sw_info.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            accm_proc_int->accm_sw_info.awbIIRDampCoef = awb_res->awb_smooth_factor;
            accm_proc_int->accm_sw_info.varianceLuma   = awb_res->varianceLuma;
            accm_proc_int->accm_sw_info.awbConverged   = awb_res->awbConverged;
        } else {
            LOGW("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    if (pCurExp) {
        if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->LinearExp.exp_real_params.analog_gain *
                pCurExp->LinearExp.exp_real_params.digital_gain *
                pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->HdrExp[1].exp_real_params.analog_gain *
                pCurExp->HdrExp[1].exp_real_params.digital_gain *
                pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for accm");
            accm_proc_int->accm_sw_info.sensorGain =
                pCurExp->HdrExp[2].exp_real_params.analog_gain *
                pCurExp->HdrExp[2].exp_real_params.digital_gain *
                pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGE(
                "working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default "
                "value ",
                sharedCom->working_mode);
        }
    } else {
        LOGE("fail to get sensor gain form AE module,use default value ");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAccm* accm_post_int        = (RkAiqAlgoPostAccm*)mPostInParam;
    RkAiqAlgoPostResAccm* accm_post_res_int = (RkAiqAlgoPostResAccm*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "accm handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "accm algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAccmHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAccm* accm_com = (RkAiqAlgoProcResAccm*)mProcOutParam;
    rk_aiq_isp_ccm_params_v20_t* ccm_param = params->mCcmParams->data().ptr();

    if (!accm_com) {
        LOGD_ANALYZER("no accm result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAccm* accm_rk = (RkAiqAlgoProcResAccm*)accm_com;

    if (sharedCom->init) {
        ccm_param->frame_id = 0;
    } else {
        ccm_param->frame_id = shared->frameId;
    }
    ccm_param->result = accm_rk->accm_hw_conf;

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAccm* accm_rk_int = (RkAiqAlgoProcResAccm*)accm_com;
    }

    cur_params->mCcmParams = params->mCcmParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
