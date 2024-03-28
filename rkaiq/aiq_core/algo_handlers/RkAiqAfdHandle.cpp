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

#include "RkAiqAfdHandle.h"
#include "RkAiqAeHandle.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAfdHandleInt);

void RkAiqAfdHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAfd());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAfd());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAfd());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAfd());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAfd());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAfdHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if 0
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }
    if (needSync) mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
#endif

    return ret;
}

XCamReturn RkAiqAfdHandleInt::getScaleRatio(int* scale_ratio) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Uapi_AfdAttr_t AfdAttr;
    rk_aiq_uapi_afd_v2_GetAttr(mAlgoCtx, &AfdAttr);
    *scale_ratio = AfdAttr.scale_ratio;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfdHandleInt::getAfdEn(bool* en) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    Uapi_AfdAttr_t AfdAttr;
    rk_aiq_uapi_afd_v2_GetAttr(mAlgoCtx, &AfdAttr);
    *en = AfdAttr.enable;

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfdHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "afd handle prepare failed");

    // TODO config ae common params:
    RkAiqAlgoConfigAfd* afd_config = (RkAiqAlgoConfigAfd*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    /*****************AfdConfig Sensor Exp related info*****************/
    afd_config->LinePeriodsPerField = (float)sharedCom->snsDes.frame_length_lines;
    afd_config->PixelPeriodsPerLine = (float)sharedCom->snsDes.line_length_pck;
    afd_config->PixelClockFreqMHZ = (float) sharedCom->snsDes.pixel_clock_freq_mhz;

    /*****************AecConfig pic-info params*****************/
    afd_config->RawWidth  = sharedCom->snsDes.isp_acq_width;
    afd_config->RawHeight = sharedCom->snsDes.isp_acq_height;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "afd algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;

}

XCamReturn RkAiqAfdHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAfd* afd_pre_int        = (RkAiqAlgoPreAfd*)mPreInParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    afd_pre_int->aeProcRes = &mAeProcRes;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "afd handle preProcess failed");
    }

    if(sharedCom->init)
    {
        LOGV("init: no thumb data, ignore!");
        return XCAM_RETURN_NO_ERROR;
    }
    else
    {
        if(shared->scaleRawInfo.raw_s) {
            afd_pre_int->thumbStatsS = shared->scaleRawInfo.raw_s;
            afd_pre_int->thumbH = shared->scaleRawInfo.raw_s->info.height;
            afd_pre_int->thumbW = shared->scaleRawInfo.raw_s->info.width;
        } else {
            afd_pre_int->thumbStatsS = NULL;
        }

        if((rk_aiq_working_mode_t)sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2) {
            if(shared->scaleRawInfo.raw_l) {
                afd_pre_int->thumbStatsL = shared->scaleRawInfo.raw_l;
            } else {
                afd_pre_int->thumbStatsL = NULL;
            }
        }
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "afd algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAfdHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "afd handle processing failed");
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "afd algo processing failed");

#if RKAIQ_HAVE_AFD_V2
    RkAiqAlgoProcResAfd* afd_res = (RkAiqAlgoProcResAfd*)mProcOutParam;
    SmartPtr<RkAiqHandle>* ae_handle = mAiqCore->getCurAlgoTypeHandle(RK_AIQ_ALGO_TYPE_AE);

    if (ae_handle) {
        int algo_id = (*ae_handle)->getAlgoId();
        if (algo_id == 0) {
            RkAiqAeHandleInt* ae_algo = dynamic_cast<RkAiqAeHandleInt*>(ae_handle->ptr());
            ae_algo->getAfdResForAE(afd_res->afdRes.PeakRes);
        }
    }
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAfdHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if 0
    RkAiqAlgoPostAfd* afd_post_int = (RkAiqAlgoPostAfd*)mPostInParam;
    RkAiqAlgoPostResAfd* afd_post_res_int =
        (RkAiqAlgoPostResAfd*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "afd handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "afd algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
#endif
    return ret;
}

XCamReturn RkAiqAfdHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqAlgoProcResAfd* afd_com = (RkAiqAlgoProcResAfd*)mProcOutParam;
    rk_aiq_isp_afd_params_t* afd_param = params->mAfdParams->data().ptr();
    afd_param->result.enable = mAeProcRes.IsAutoAfd;
    getScaleRatio(&afd_param->result.ratio);
    cur_params->mAfdParams = params->mAfdParams;
    if (!afd_com) {
        LOGD_ANALYZER("no afd result");
        return XCAM_RETURN_NO_ERROR;
    }
    if (mAfdSyncAly != afd_param->result.enable) {
        mAfdSyncAly = afd_param->result.enable;
        mAiqCore->setVicapScaleFlag(mAfdSyncAly);
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
