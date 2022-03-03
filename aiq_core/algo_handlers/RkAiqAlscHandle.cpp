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
#include "RkAiqAlscHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAlscHandleInt);

void RkAiqAlscHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAlsc());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAlsc());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAlsc());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAlsc());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAlsc());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAlsc());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAlsc());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAlscHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_alsc_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAlscHandleInt::setAttrib(rk_aiq_lsc_attrib_t att) {
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

XCamReturn RkAiqAlscHandleInt::getAttrib(rk_aiq_lsc_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
      mCfgMutex.lock();
      rk_aiq_uapi_alsc_GetAttrib(mAlgoCtx, att);
      att->sync.done = true;
      mCfgMutex.unlock();
    } else {
      if (updateAtt) {
        memcpy(att, &mNewAtt, sizeof(updateAtt));
        att->sync.done = false;
      } else {
        rk_aiq_uapi_alsc_GetAttrib(mAlgoCtx, att);
        att->sync.sync_mode = mNewAtt.sync.sync_mode;
        att->sync.done      = true;
      }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAlscHandleInt::queryLscInfo(rk_aiq_lsc_querry_info_t* lsc_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_alsc_QueryLscInfo(mAlgoCtx, lsc_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAlscHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "alsc handle prepare failed");

    RkAiqAlgoConfigAlsc* alsc_config_int = (RkAiqAlgoConfigAlsc*)mConfig;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "alsc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAlscHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAlsc* alsc_pre_int        = (RkAiqAlgoPreAlsc*)mPreInParam;
    RkAiqAlgoPreResAlsc* alsc_pre_res_int = (RkAiqAlgoPreResAlsc*)mPreOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "alsc handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAlscHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAlsc* alsc_proc_int        = (RkAiqAlgoProcAlsc*)mProcInParam;
    RkAiqAlgoProcResAlsc* alsc_proc_res_int = (RkAiqAlgoProcResAlsc*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "alsc handle processing failed");
    }

    alsc_proc_int->tx             = shared->tx;
    XCamVideoBuffer* awb_proc_res = shared->res_comb.awb_proc_res;
    if (awb_proc_res) {
        RkAiqAlgoProcResAwb* awb_res =
            (RkAiqAlgoProcResAwb*)awb_proc_res->map(awb_proc_res);
        if (awb_res) {
            if (awb_res->awb_gain_algo.grgain < DIVMIN || awb_res->awb_gain_algo.gbgain < DIVMIN) {
                LOGW("get wrong awb gain from AWB module ,use default value ");
            } else {
                alsc_proc_int->alsc_sw_info.awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                alsc_proc_int->alsc_sw_info.awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            alsc_proc_int->alsc_sw_info.awbIIRDampCoef = awb_res->awb_smooth_factor;
            alsc_proc_int->alsc_sw_info.varianceLuma   = awb_res->varianceLuma;
            alsc_proc_int->alsc_sw_info.awbConverged   = awb_res->awbConverged;
        } else {
            LOGW("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }

    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    if (pCurExp) {
        if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            alsc_proc_int->alsc_sw_info.sensorGain =
                pCurExp->LinearExp.exp_real_params.analog_gain *
                pCurExp->LinearExp.exp_real_params.digital_gain *
                pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain =
                pCurExp->HdrExp[1].exp_real_params.analog_gain *
                pCurExp->HdrExp[1].exp_real_params.digital_gain *
                pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for alsc");
            alsc_proc_int->alsc_sw_info.sensorGain =
                pCurExp->HdrExp[2].exp_real_params.analog_gain *
                pCurExp->HdrExp[2].exp_real_params.digital_gain *
                pCurExp->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW(
                "working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default "
                "value ",
                sharedCom->working_mode);
        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAlscHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAlsc* alsc_post_int        = (RkAiqAlgoPostAlsc*)mPostInParam;
    RkAiqAlgoPostResAlsc* alsc_post_res_int = (RkAiqAlgoPostResAlsc*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "alsc handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "alsc algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAlscHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAlsc* alsc_com              = (RkAiqAlgoProcResAlsc*)mProcOutParam;
    rk_aiq_isp_lsc_params_v20_t* lsc_param      = params->mLscParams->data().ptr();

    if (!alsc_com) {
        LOGD_ANALYZER("no alsc result");
        return XCAM_RETURN_NO_ERROR;
    }

    RkAiqAlgoProcResAlsc* alsc_rk = (RkAiqAlgoProcResAlsc*)alsc_com;
    if (sharedCom->init) {
        lsc_param->frame_id = 0;
    } else {
        lsc_param->frame_id = shared->frameId;
    }
    lsc_param->result = alsc_rk->alsc_hw_conf;
    if (sharedCom->sns_mirror) {
        for (int i = 0; i < LSC_DATA_TBL_V_SIZE; i++) {
            for (int j = 0; j < LSC_DATA_TBL_H_SIZE; j++) {
                SWAP(unsigned short, lsc_param->result.r_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .r_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
                SWAP(unsigned short, lsc_param->result.b_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .b_data_tbl[i * LSC_DATA_TBL_H_SIZE + (LSC_DATA_TBL_H_SIZE - 1 - j)]);
            }
        }
    }
    if (sharedCom->sns_flip) {
        for (int i = 0; i < LSC_DATA_TBL_V_SIZE; i++) {
            for (int j = 0; j < LSC_DATA_TBL_H_SIZE; j++) {
                SWAP(unsigned short, lsc_param->result.r_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .r_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i) * LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.gr_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .gr_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i) * LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.gb_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .gb_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i) * LSC_DATA_TBL_H_SIZE + j]);
                SWAP(unsigned short, lsc_param->result.b_data_tbl[i * LSC_DATA_TBL_H_SIZE + j],
                     lsc_param->result
                         .b_data_tbl[(LSC_DATA_TBL_V_SIZE - 1 - i) * LSC_DATA_TBL_H_SIZE + j]);
            }
        }
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAlsc* alsc_rk_int = (RkAiqAlgoProcResAlsc*)alsc_com;

        if (sharedCom->init) {
            lsc_param->frame_id = 0;
        } else {
            lsc_param->frame_id = shared->frameId;
        }
    }

    cur_params->mLscParams = params->mLscParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
