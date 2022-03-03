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
#include "RkAiqAdhazHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAdhazHandleInt);

void RkAiqAdhazHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAdhaz());
    mPreInParam   = (RkAiqAlgoCom*)(new RkAiqAlgoPreAdhaz());
    mPreOutParam  = (RkAiqAlgoResCom*)(new RkAiqAlgoPreResAdhaz());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAdhaz());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAdhaz());
    mPostInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoPostAdhaz());
    mPostOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoPostResAdhaz());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqAdhazHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "adhaz handle prepare failed");

    RkAiqAlgoConfigAdhaz* adhaz_config_int   = (RkAiqAlgoConfigAdhaz*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_config_int->calib = sharedCom->calib;
#endif

    adhaz_config_int->working_mode      = sharedCom->working_mode;
    adhaz_config_int->is_multi_isp_mode = sharedCom->is_multi_isp_mode;
    // adhaz_config_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    // adhaz_config_int->rawWidth = sharedCom->snsDes.isp_acq_width;
    // adhaz_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdhazHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPreAdhaz* adhaz_pre_int         = (RkAiqAlgoPreAdhaz*)mPreInParam;
    RkAiqAlgoPreResAdhaz* adhaz_pre_res_int  = (RkAiqAlgoPreResAdhaz*)mPreOutParam;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());

    adhaz_pre_int->rawHeight = sharedCom->snsDes.isp_acq_height;
    adhaz_pre_int->rawWidth  = sharedCom->snsDes.isp_acq_width;

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_ADEHAZE("isp stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->adehaze_stats_valid || !sharedCom->init) {
        LOG1("no adehaze stats, ignore!");
        // TODO: keep last result ?
        //
        //
        // return XCAM_RETURN_BYPASS;
    } else {
        // dehaze stats
        if (CHECK_ISP_HW_V20())
            memcpy(&adhaz_pre_int->stats.dehaze_stats_v20,
                   &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v20,
                   sizeof(dehaze_stats_v20_t));
        else if (CHECK_ISP_HW_V21())
            memcpy(&adhaz_pre_int->stats.dehaze_stats_v21,
                   &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v21,
                   sizeof(dehaze_stats_v21_t));
        else if (CHECK_ISP_HW_V30())
            memcpy(&adhaz_pre_int->stats.dehaze_stats_v30,
                   &xIspStats->AdehazeStatsProxy->data()->adehaze_stats.dehaze_stats_v30,
                   sizeof(dehaze_stats_v21_t));

        // other stats
        memcpy(adhaz_pre_int->stats.other_stats.tmo_luma,
               xIspStats->AecStatsProxy->data()->aec_stats.ae_data.extra.rawae_big.channelg_xy,
               sizeof(adhaz_pre_int->stats.other_stats.tmo_luma));

        if (sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
                sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
            memcpy(adhaz_pre_int->stats.other_stats.short_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                   sizeof(adhaz_pre_int->stats.other_stats.short_luma));
            memcpy(
                adhaz_pre_int->stats.other_stats.middle_luma,
                xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy,
                sizeof(adhaz_pre_int->stats.other_stats.middle_luma));
            memcpy(adhaz_pre_int->stats.other_stats.long_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[2].rawae_big.channelg_xy,
                   sizeof(adhaz_pre_int->stats.other_stats.long_luma));
        } else if (sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                   sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
            memcpy(adhaz_pre_int->stats.other_stats.short_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                   sizeof(adhaz_pre_int->stats.other_stats.short_luma));
            memcpy(adhaz_pre_int->stats.other_stats.long_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[1].rawae_big.channelg_xy,
                   sizeof(adhaz_pre_int->stats.other_stats.long_luma));
        } else
            LOGD("Wrong working mode!!!");
    }

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle preProcess failed");
    }

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAdhazHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAdhaz* adhaz_proc_int        = (RkAiqAlgoProcAdhaz*)mProcInParam;
    RkAiqAlgoProcResAdhaz* adhaz_proc_res_int = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    adhaz_proc_int->hdr_mode = sharedCom->working_mode;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle processing failed");
    }

#ifdef RKAIQ_ENABLE_PARSER_V1
    adhaz_proc_int->pCalibDehaze = sharedCom->calib;
#endif

#ifdef RK_SIMULATOR_HW
    // nothing todo
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoPostAdhaz* adhaz_post_int        = (RkAiqAlgoPostAdhaz*)mPostInParam;
    RkAiqAlgoPostResAdhaz* adhaz_post_res_int = (RkAiqAlgoPostResAdhaz*)mPostOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "adhaz handle postProcess failed");
        return ret;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "adhaz algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        rk_aiq_uapi_adehaze_SetAttrib(mAlgoCtx, mCurAtt, false);
        updateAtt = false;
        sendSignal(mCurAtt.sync.sync_mode);
    }

    if (needSync) mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandleInt::setSwAttrib(adehaze_sw_V2_t att) {
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

XCamReturn RkAiqAdhazHandleInt::getSwAttrib(adehaze_sw_V2_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (att->sync.sync_mode == RK_AIQ_UAPI_MODE_SYNC) {
        mCfgMutex.lock();
        rk_aiq_uapi_adehaze_GetAttrib(mAlgoCtx, att);
        att->sync.done = true;
        mCfgMutex.unlock();
    } else {
        if (updateAtt) {
            memcpy(att, &mNewAtt, sizeof(updateAtt));
            att->sync.done = false;
        } else {
            rk_aiq_uapi_adehaze_GetAttrib(mAlgoCtx, att);
            att->sync.sync_mode = mNewAtt.sync.sync_mode;
            att->sync.done      = true;
        }
    }

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAdhazHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom  = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAdhaz* adhaz_com             = (RkAiqAlgoProcResAdhaz*)mProcOutParam;
    rk_aiq_isp_dehaze_params_v20_t* dehaze_param = params->mDehazeParams->data().ptr();

    if (!adhaz_com) {
        LOGD_ANALYZER("no adhaz result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (sharedCom->init) {
        dehaze_param->frame_id = 0;
    } else {
        dehaze_param->frame_id = shared->frameId;
    }
    dehaze_param->result = adhaz_com->AdehzeProcRes;

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAdhaz* adhaz_rk = (RkAiqAlgoProcResAdhaz*)adhaz_com;
    }

    cur_params->mDehazeParams = params->mDehazeParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

};  // namespace RkCam
