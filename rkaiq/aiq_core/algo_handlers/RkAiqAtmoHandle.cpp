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
#include "RkAiqAtmoHandle.h"

#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqAtmoHandleInt);

void RkAiqAtmoHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigAtmo());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoProcAtmo());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResAtmo());

    EXIT_ANALYZER_FUNCTION();
}
XCamReturn RkAiqAtmoHandleInt::updateConfig(bool needSync) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef DISABLE_HANDLE_ATTRIB
    if (needSync) mCfgMutex.lock();
    // if something changed
    if (updateAtt) {
        mCurAtt   = mNewAtt;
        updateAtt = false;
        rk_aiq_uapi_atmo_SetAttrib(mAlgoCtx, mCurAtt, true);
        sendSignal();
    }
    if (needSync) mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandleInt::setAttrib(atmo_attrib_t att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
#ifdef DISABLE_HANDLE_ATTRIB
    ret = rk_aiq_uapi_atmo_SetAttrib(mAlgoCtx, att, false);
#else
    // TODO
    // check if there is different between att & mCurAtt
    // if something changed, set att to mNewAtt, and
    // the new params will be effective later when updateConfig
    // called by RkAiqCore

    // if something changed
    if (0 != memcmp(&mCurAtt, &att, sizeof(atmo_attrib_t))) {
        mNewAtt   = att;
        updateAtt = true;
        waitSignal();
    }
    mCfgMutex.unlock();
#endif

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
XCamReturn RkAiqAtmoHandleInt::getAttrib(atmo_attrib_t* att) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rk_aiq_uapi_atmo_GetAttrib(mAlgoCtx, att);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "atmo handle prepare failed");

    RkAiqAlgoConfigAtmo* atmo_config_int = (RkAiqAlgoConfigAtmo*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    // TODO
    atmo_config_int->rawHeight    = sharedCom->snsDes.isp_acq_height;
    atmo_config_int->rawWidth     = sharedCom->snsDes.isp_acq_width;
    atmo_config_int->working_mode = sharedCom->working_mode;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "atmo algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAtmoHandleInt::preProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::preProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "atmo handle preProcess failed");
    }

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_AEC("isp stats is null");
    } else {
        LOGW_ADEHAZE("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->atmo_stats_valid || !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->pre_process(mPreInParam, mPreOutParam);
    RKAIQCORE_CHECK_RET(ret, "ahdr algo pre_process failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqAtmoHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcAtmo* atmo_proc_int        = (RkAiqAlgoProcAtmo*)mProcInParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "atmo handle processing failed");
    }

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_ATMO("isp stats is null");
    } else {
        LOGW_ATMO("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->atmo_stats_valid || !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //
        return XCAM_RETURN_BYPASS;
    } else {
        memcpy(&atmo_proc_int->ispAtmoStats.tmo_stats,
               &xIspStats->AtmoStatsProxy->data()->atmo_stats.tmo_stats, sizeof(hdrtmo_stats_t));
        memcpy(atmo_proc_int->ispAtmoStats.other_stats.tmo_luma,
               xIspStats->AecStatsProxy->data()->aec_stats.ae_data.extra.rawae_big.channelg_xy,
               sizeof(atmo_proc_int->ispAtmoStats.other_stats.tmo_luma));

        if (sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
            sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.short_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                   sizeof(atmo_proc_int->ispAtmoStats.other_stats.short_luma));
            memcpy(
                atmo_proc_int->ispAtmoStats.other_stats.middle_luma,
                xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[1].rawae_lite.channelg_xy,
                sizeof(atmo_proc_int->ispAtmoStats.other_stats.middle_luma));
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.long_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[2].rawae_big.channelg_xy,
                   sizeof(atmo_proc_int->ispAtmoStats.other_stats.long_luma));
        } else if (sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
                   sharedCom->working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.short_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[0].rawae_big.channelg_xy,
                   sizeof(atmo_proc_int->ispAtmoStats.other_stats.short_luma));
            memcpy(atmo_proc_int->ispAtmoStats.other_stats.long_luma,
                   xIspStats->AecStatsProxy->data()->aec_stats.ae_data.chn[1].rawae_big.channelg_xy,
                   sizeof(atmo_proc_int->ispAtmoStats.other_stats.long_luma));
        } else
            LOGD("Wrong working mode!!!");
    }

#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.lock();
#endif
    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->processing(mProcInParam, mProcOutParam);
#ifdef DISABLE_HANDLE_ATTRIB
    mCfgMutex.unlock();
#endif
    RKAIQCORE_CHECK_RET(ret, "atmo algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandleInt::postProcess() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::postProcess();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ahdr handle postProcess failed");
        return ret;
    }

    RkAiqIspStats* xIspStats = nullptr;
    if (shared->ispStats) {
        xIspStats = (RkAiqIspStats*)shared->ispStats->map(shared->ispStats);
        if (!xIspStats) LOGE_ATMO("isp stats is null");
    } else {
        LOGW_ATMO("the xcamvideobuffer of isp stats is null");
    }

    if (!xIspStats || !xIspStats->atmo_stats_valid || !sharedCom->init) {
        LOGD("no atmo stats, ignore!");
        // TODO: keep last result ?
        //
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->post_process(mPostInParam, mPostOutParam);
    RKAIQCORE_CHECK_RET(ret, "atmo algo post_process failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqAtmoHandleInt::genIspResult(RkAiqFullParams* params, RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResAtmo* atmo_com              = (RkAiqAlgoProcResAtmo*)mProcOutParam;

    rk_aiq_isp_tmo_params_t* tmo_param = params->mTmoParams->data().ptr();

    if (!atmo_com) {
        LOGD_ANALYZER("no atmo result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        RkAiqAlgoProcResAtmo* atmo_rk = (RkAiqAlgoProcResAtmo*)atmo_com;

        if (sharedCom->init) {
            tmo_param->frame_id = 0;
        } else {
            tmo_param->frame_id = shared->frameId;
        }

        tmo_param->result.Res.sw_hdrtmo_lgmax      = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgmax;
        tmo_param->result.Res.sw_hdrtmo_lgscl      = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl;
        tmo_param->result.Res.sw_hdrtmo_lgscl_inv  = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl_inv;
        tmo_param->result.Res.sw_hdrtmo_clipratio0 = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipratio0;
        tmo_param->result.Res.sw_hdrtmo_clipratio1 = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipratio1;
        tmo_param->result.Res.sw_hdrtmo_clipgap0   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipgap0;
        tmo_param->result.Res.sw_hdrtmo_clipgap1   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_clipgap1;
        tmo_param->result.Res.sw_hdrtmo_ratiol     = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_ratiol;
        tmo_param->result.Res.sw_hdrtmo_hist_min   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_min;
        tmo_param->result.Res.sw_hdrtmo_hist_low   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_low;
        tmo_param->result.Res.sw_hdrtmo_hist_high  = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_high;
        tmo_param->result.Res.sw_hdrtmo_hist_0p3   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_0p3;
        tmo_param->result.Res.sw_hdrtmo_hist_shift = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_hist_shift;
        tmo_param->result.Res.sw_hdrtmo_palpha_0p18 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_0p18;
        tmo_param->result.Res.sw_hdrtmo_palpha_lw0p5 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_lw0p5;
        tmo_param->result.Res.sw_hdrtmo_palpha_lwscl =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_palpha_lwscl;
        tmo_param->result.Res.sw_hdrtmo_maxpalpha = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_maxpalpha;
        tmo_param->result.Res.sw_hdrtmo_maxgain   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_maxgain;
        tmo_param->result.Res.sw_hdrtmo_cfg_alpha = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cfg_alpha;
        tmo_param->result.Res.sw_hdrtmo_set_gainoff =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_gainoff;
        tmo_param->result.Res.sw_hdrtmo_set_lgmin  = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmin;
        tmo_param->result.Res.sw_hdrtmo_set_lgmax  = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmax;
        tmo_param->result.Res.sw_hdrtmo_set_lgmean = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgmean;
        tmo_param->result.Res.sw_hdrtmo_set_weightkey =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_weightkey;
        tmo_param->result.Res.sw_hdrtmo_set_lgrange0 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgrange0;
        tmo_param->result.Res.sw_hdrtmo_set_lgrange1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgrange1;
        tmo_param->result.Res.sw_hdrtmo_set_lgavgmax =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_lgavgmax;
        tmo_param->result.Res.sw_hdrtmo_set_palpha = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_set_palpha;
        tmo_param->result.Res.sw_hdrtmo_gain_ld_off1 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_gain_ld_off1;
        tmo_param->result.Res.sw_hdrtmo_gain_ld_off2 =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_gain_ld_off2;
        tmo_param->result.Res.sw_hdrtmo_cnt_vsize  = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cnt_vsize;
        tmo_param->result.Res.sw_hdrtmo_big_en     = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_big_en;
        tmo_param->result.Res.sw_hdrtmo_nobig_en   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_nobig_en;
        tmo_param->result.Res.sw_hdrtmo_newhist_en = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_newhist_en;
        tmo_param->result.Res.sw_hdrtmo_cnt_mode   = atmo_rk->AtmoProcRes.Res.sw_hdrtmo_cnt_mode;
        tmo_param->result.Res.sw_hdrtmo_expl_lgratio =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_expl_lgratio;
        tmo_param->result.Res.sw_hdrtmo_lgscl_ratio =
            atmo_rk->AtmoProcRes.Res.sw_hdrtmo_lgscl_ratio;

        tmo_param->result.LongFrameMode = atmo_rk->AtmoProcRes.LongFrameMode;

        tmo_param->result.isHdrGlobalTmo = atmo_rk->AtmoProcRes.isHdrGlobalTmo;

        tmo_param->result.bTmoEn = atmo_rk->AtmoProcRes.bTmoEn;

        tmo_param->result.isLinearTmo = atmo_rk->AtmoProcRes.isLinearTmo;

        tmo_param->result.TmoFlicker.GlobalTmoStrengthDown =
            atmo_rk->AtmoProcRes.TmoFlicker.GlobalTmoStrengthDown;
        tmo_param->result.TmoFlicker.GlobalTmoStrength =
            atmo_rk->AtmoProcRes.TmoFlicker.GlobalTmoStrength;
        tmo_param->result.TmoFlicker.iir    = atmo_rk->AtmoProcRes.TmoFlicker.iir;
        tmo_param->result.TmoFlicker.iirmax = atmo_rk->AtmoProcRes.TmoFlicker.iirmax;
        tmo_param->result.TmoFlicker.height = atmo_rk->AtmoProcRes.TmoFlicker.height;
        tmo_param->result.TmoFlicker.width  = atmo_rk->AtmoProcRes.TmoFlicker.width;

        tmo_param->result.TmoFlicker.PredictK.correction_factor =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.correction_factor;
        tmo_param->result.TmoFlicker.PredictK.correction_offset =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.correction_offset;
        tmo_param->result.TmoFlicker.PredictK.Hdr3xLongPercent =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.Hdr3xLongPercent;
        tmo_param->result.TmoFlicker.PredictK.UseLongUpTh =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.UseLongUpTh;
        tmo_param->result.TmoFlicker.PredictK.UseLongLowTh =
            atmo_rk->AtmoProcRes.TmoFlicker.PredictK.UseLongLowTh;
        for (int i = 0; i < 3; i++)
            tmo_param->result.TmoFlicker.LumaDeviation[i] =
                atmo_rk->AtmoProcRes.TmoFlicker.LumaDeviation[i];
        tmo_param->result.TmoFlicker.StableThr = atmo_rk->AtmoProcRes.TmoFlicker.StableThr;
    }

    cur_params->mTmoParams = params->mTmoParams;

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
