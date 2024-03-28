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
#include "RkAiqA3dlutHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqA3dlutHandleInt);

void RkAiqA3dlutHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigA3dlut());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcA3dlut());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResA3dlut());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqA3dlutHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt = mNewAtt;
        // TODO
        rk_aiq_uapi_a3dlut_SetAttrib(mAlgoCtx, &mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();
#endif
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::setAttrib(const rk_aiq_lut3d_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();

    // check if there is different between att & mCurAtt(sync)/mNewAtt(async)
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_a3dlut_SetAttrib(mAlgoCtx, att, false);
#else
    bool isChanged = false;
    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_ASYNC && \
        memcmp(&mNewAtt, att, sizeof(*att)))
        isChanged = true;
    else if (att->sync.sync_mode != RK_AIQ_UAPI_MODE_ASYNC && \
             memcmp(&mCurAtt, att, sizeof(*att)))
        isChanged = true;

    // if something changed
    if (isChanged) {
        mNewAtt   = *att;
        updateAtt = true;
        waitSignal(att->sync.sync_mode);
    }
#endif

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::getAttrib(rk_aiq_lut3d_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(att != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifdef DISABLE_HANDLE_ATTRIB
      mCfgMutex.lock();
      ret = rk_aiq_uapi_a3dlut_GetAttrib(mAlgoCtx, att);
      mCfgMutex.unlock();
      return ret;
#else

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_a3dlut_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(mNewAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_a3dlut_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::query3dlutInfo(rk_aiq_lut3d_querry_info_t* lut3d_querry_info) {
    ENTER_ANALYZER_FUNCTION();

    XCAM_ASSERT(lut3d_querry_info != nullptr);

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_a3dlut_Query3dlutInfo(mAlgoCtx, lut3d_querry_info);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "a3dlut handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqA3dlutHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle preProcess failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqA3dlutHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcA3dlut* a3dlut_proc_int        = (RkAiqAlgoProcA3dlut*)mProcInParam;
    RkAiqAlgoProcResA3dlut* a3dlut_proc_res_int = (RkAiqAlgoProcResA3dlut*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    if (!shared->fullParams || !shared->fullParams->mLut3dParams.ptr()) {
        LOGE_A3DLUT("[%d]: no 3dlut buf !", shared->frameId);
        return XCAM_RETURN_BYPASS;
    }

    a3dlut_proc_res_int->lut3d_hw_conf = &shared->fullParams->mLut3dParams->data()->result;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle processing failed");
    }

    XCamVideoBuffer* xCamAwbProcRes = shared->res_comb.awb_proc_res;
    if (xCamAwbProcRes) {
        RkAiqAlgoProcResAwbShared_t* awb_res =
            (RkAiqAlgoProcResAwbShared_t*)xCamAwbProcRes->map(xCamAwbProcRes);
        if (awb_res) {
            if (awb_res->awb_gain_algo.grgain < DIVMIN || awb_res->awb_gain_algo.gbgain < DIVMIN) {
                LOGE("get wrong awb gain from AWB module ,use default value ");
            } else {
                a3dlut_proc_int->awbGain[0] =
                    awb_res->awb_gain_algo.rgain / awb_res->awb_gain_algo.grgain;

                a3dlut_proc_int->awbGain[1] =
                    awb_res->awb_gain_algo.bgain / awb_res->awb_gain_algo.gbgain;
            }
            a3dlut_proc_int->awbIIRDampCoef = awb_res->awb_smooth_factor;
            a3dlut_proc_int->awbConverged   = awb_res->awbConverged;
        } else {
            LOGW("fail to get awb gain form AWB module,use default value ");
        }
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    if (pCurExp) {
        if ((rk_aiq_working_mode_t)sharedCom->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            a3dlut_proc_int->sensorGain = pCurExp->LinearExp.exp_real_params.analog_gain *
                                          pCurExp->LinearExp.exp_real_params.digital_gain *
                                          pCurExp->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for a3dlut");
            a3dlut_proc_int->sensorGain = pCurExp->HdrExp[1].exp_real_params.analog_gain *
                                          pCurExp->HdrExp[1].exp_real_params.digital_gain *
                                          pCurExp->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for a3dlut");
            a3dlut_proc_int->sensorGain = pCurExp->HdrExp[2].exp_real_params.analog_gain *
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

#if RKAIQ_HAVE_BLC_V32 && !USE_NEWSTRUCT
    if (shared->res_comb.ablcV32_proc_res->blc_ob_enable) {
        if (shared->res_comb.ablcV32_proc_res->isp_ob_predgain >= 1.0f) {
            a3dlut_proc_int->sensorGain *=  shared->res_comb.ablcV32_proc_res->isp_ob_predgain;
        }
    }
#endif
    if(colorConstFlag==true){
        memcpy(a3dlut_proc_int->awbGain,colorSwInfo.awbGain,sizeof(colorSwInfo.awbGain));
        a3dlut_proc_int->sensorGain = colorSwInfo.sensorGain;
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if 0
    RkAiqAlgoPostA3dlut* a3dlut_post_int        = (RkAiqAlgoPostA3dlut*)mPostInParam;
    RkAiqAlgoPostResA3dlut* a3dlut_post_res_int = (RkAiqAlgoPostResA3dlut*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "a3dlut handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "a3dlut algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqA3dlutHandleInt::genIspResult(RkAiqFullParams* params,
                                              RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResA3dlut* a3dlut_com = (RkAiqAlgoProcResA3dlut*)mProcOutParam;
    rk_aiq_isp_lut3d_params_t* lut3d_param = params->mLut3dParams->data().ptr();

    if (sharedCom->init) {
        lut3d_param->frame_id = 0;
    } else {
        lut3d_param->frame_id = shared->frameId;
    }

    if (!a3dlut_com) {
        LOGD_ANALYZER("no a3dlut result");
        return XCAM_RETURN_NO_ERROR;
    }

#if 0//moved to processing out params
    RkAiqAlgoProcResA3dlut* a3dlut_rk = (RkAiqAlgoProcResA3dlut*)a3dlut_com;
    if (!a3dlut_rk->lut3d_hw_conf.enable || a3dlut_rk->lut3d_hw_conf.bypass_en) {
        lut3d_param->result.enable = a3dlut_rk->lut3d_hw_conf.enable;
        lut3d_param->result.bypass_en = a3dlut_rk->lut3d_hw_conf.bypass_en;
    } else
        lut3d_param->result               = a3dlut_rk->lut3d_hw_conf;
#endif
    if (a3dlut_com->res_com.cfg_update) {
        mSyncFlag = shared->frameId;
        lut3d_param->sync_flag = mSyncFlag;
        // copy from algo result
        // set as the latest result
        cur_params->mLut3dParams = params->mLut3dParams;
        lut3d_param->is_update = true;
        LOGD_A3DLUT("[%d] params from algo", mSyncFlag);
    } else if (mSyncFlag != lut3d_param->sync_flag) {
        lut3d_param->sync_flag = mSyncFlag;
        // copy from latest result
        if (cur_params->mLut3dParams.ptr()) {
            lut3d_param->result = cur_params->mLut3dParams->data()->result;
            lut3d_param->is_update = true;
        } else {
            LOGE_A3DLUT("no latest params !");
            lut3d_param->is_update = false;
        }
        LOGD_A3DLUT("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
   } else {
        // do nothing, result in buf needn't update
        lut3d_param->is_update = false;
        LOGD_A3DLUT("[%d] params needn't update", shared->frameId);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}
XCamReturn RkAiqA3dlutHandleInt::setAcolorSwInfo(rk_aiq_color_info_t aColor_sw_info) {
    ENTER_ANALYZER_FUNCTION();


    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGV_A3DLUT("%s sensor gain = %f, wbgain=[%f,%f] ",__FUNCTION__,aColor_sw_info.sensorGain,
      aColor_sw_info.awbGain[0],aColor_sw_info.awbGain[1]);
    colorSwInfo = aColor_sw_info;
    colorConstFlag=true;


    EXIT_ANALYZER_FUNCTION();
    return ret;
}

}  // namespace RkCam
