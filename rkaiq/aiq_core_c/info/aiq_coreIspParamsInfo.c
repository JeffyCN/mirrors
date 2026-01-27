/*
 *  Copyright (c) 2024 Rockchip Corporation
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
 *
 */

#include "aiq_coreIspParamsInfo.h"

#include "aiq_algo_handler.h"
#include "rk_info_utils.h"

void core_isp_params_dump_mod_param(AiqCore_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s", "phy_chn");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d", self->mAlogsComSharedParams.mCamPhyId);

    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_isp_params_dump_ldc_params(AiqCore_t* self, st_string* result) {
    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ALDC];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "ldc mod params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s%-8s", "mod", "is_update", "en", "buf_fd");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    aiq_params_base_t* base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_LDC_PARAM];
    ldc_param_t* ldc_param  = (ldc_param_t*)(base->_data);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s%-8d", "ldch", base->is_update ? "Y" : "N",
             ldc_param->sta.ldchCfg.en ? "Y" : "N",
             ldc_param->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
#if defined(ISP_HW_V39)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s%-8d", "ldcv", base->is_update ? "Y" : "N",
             ldc_param->sta.ldcvCfg.en ? "Y" : "N",
             ldc_param->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMapBuf_fd[0]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
}

void core_isp_params_dump_awbgain_params(AiqCore_t* self, st_string* result) {
    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "awbgain mod params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s", "mod", "is_update", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    aiq_params_base_t* base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_AWBGAIN_PARAM];
    rk_aiq_isp_awb_gain_params_t* awbgain_param  = (rk_aiq_isp_awb_gain_params_t*)(base->_data);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s", "awbgain", base->is_update ? "Y" : "N",
             "Y" );
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "awbgain_r", "awbgain_gr",
             "awbgain_gb", "awbgain_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    memset(buffer, 0, MAX_LINE_LENGTH);

    snprintf(buffer, MAX_LINE_LENGTH, "%-14f%-14f%-14f%-14f", awbgain_param->rgain,
             awbgain_param->grgain, awbgain_param->gbgain, awbgain_param->bgain);
    aiq_string_printf(result, buffer);

    aiq_string_printf(result, "\n");
}

void core_isp_params_dump_rawawb_params(AiqCore_t* self, st_string* result) {
    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};
    aiq_params_base_t* base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_AWB_PARAM];

    const rk_aiq_isp_awb_params_t* awb_meas_priv =  (rk_aiq_isp_awb_params_t*)base->_data;
    const awbStats_cfg_t* awb_meas = &awb_meas_priv->com;

    aiq_info_dump_title(result, "rawawb mod params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s%-9s", "mode", "is_update", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-11s%-9s", awb_meas_priv->mode, base->is_update ? "Y" : "N",
             awb_meas_priv->com.hw_awbCfg_stats_en ? "Y" : "N" );
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "uvDct_en", "xyDct_en",
             "statsSrc_mode", "light_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d",
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_uvDct_en,
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_xyDct_en,
             awb_meas->hw_awbCfg_statsSrc_mode, awb_meas->wpEngine.hw_awbCfg_lightSrcNum_val);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#else
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s%-14s", "uvDct_en", "xyDct_en",
             "yuvDct_en", "statsSrc_mode", "light_num");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d%-14d",
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_uvDct_en,
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_xyDct_en,
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_rotYuvDct_en,
             awb_meas->hw_awbCfg_statsSrc_mode, awb_meas->wpEngine.hw_awbCfg_lightSrcNum_val);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s", "zoneStat_en", "zoneStat_mod");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d",
             awb_meas->pixEngine.hw_awbCfg_stats_en,
             awb_meas->pixEngine.hw_awbCfg_zoneStatsSrc_mode);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s", "is12bit", "ds_mode",
             "overexpo_th");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14d%-14s", "unkonwn",
             awb_meas->hw_awbCfg_ds_mode, "unkonwn");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s%-14s", "mainWin_x", "mainWin_y",
             "mainWin_width", "mainWin_hegt", "nonROI_en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-14d%-14d%-14d%-14d",
             awb_meas->mainWin.hw_awbCfg_win_x,
             awb_meas->mainWin.hw_awbCfg_win_y,
             awb_meas->mainWin.hw_awbCfg_win_width,
             awb_meas->mainWin.hw_awbCfg_win_height,
             awb_meas->mainWin.hw_awbCfg_nonROI_en);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s%-14s", "limit_maxR", "limit_maxG",
             "limit_maxB", "limit_minY");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14.4f%-14.4f%-14.4f%-14.4f",
             awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxR_thred,
             awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxG_thred,
             awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxB_thred,
             awb_meas->wpEngine.wpDct_rgbySpace.hw_awbT_wpMaxY_thred);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s", "luma2WpWgt_en", "w0",
             "w1", "w2", "w3", "w4", "w5", "w6", "w7", "w8");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14d%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f",
             awb_meas->wpEngine.norWpStatsCfg.hw_awbCfg_lumaWgt_en,
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[0],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[1],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[2],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[3],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[4],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[5],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[6],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[7],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_curve.val[8]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
#if defined(ISP_HW_V33) || defined(ISP_HW_V35)
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s", "ccm0r", "ccm1r",
             "ccm2r", "ccm0g", "ccm1g", "ccm1g", "ccm0b", "ccm1b", "ccm1b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-8.3f%-3.2f",
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[0],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[1],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[2],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[3],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[4],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[5],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[6],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[7],
             awb_meas->wpEngine.hw_awbT_luma2WpWgt_ccm[8]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
#endif
    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-14s%-14s%-14s", "pre_wbgain_r", "pre_wbgain_g",
             "pre_wbgain_b");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");
    snprintf(buffer, MAX_LINE_LENGTH, "%-14f%-14f%-14f", awb_meas_priv->preWbgainSw[0],
             awb_meas_priv->preWbgainSw[1], awb_meas_priv->preWbgainSw[3]);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

void core_isp_params_dump_cac_params(AiqCore_t* self, st_string* result) {
    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_ACAC];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "cac mod params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s", "mod", "is_update");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);

    aiq_params_base_t* base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_CAC_PARAM];
    rk_aiq_isp_cac_params_t* cac_param  = (rk_aiq_isp_cac_params_t*)(base->_data);
    cac_params_dyn_t* pdyn = &cac_param->dyn;
    cac_params_static_t* psta = &cac_param->sta;
    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-11s", "cac", base->is_update ? "Y" : "N");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-11s%-13s%-13s%-11s", "en", "bypass", "center_en",
             "clip_g_mode", "edge_detect", "neg_clip0");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", "en", "bypass", "edge_detect_en",
             "neg_clip0_en", "wgt_color_en");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", "en", "bypass", "edge_detect_en",
             "neg_clip0_en", "wgt_color_en");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    bool en = base->en;

    memset(buffer, 0, MAX_LINE_LENGTH);
#if defined(ISP_HW_V39)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-11s%-13d%-13s%-11s", en ? "Y" : "N",
             base->bypass ? "Y" : "N", psta->strgCenter.hw_cacT_strgCenter_en ? "Y" : "N",
             (int)pdyn->chromaAberrCorr.hw_cacT_clipG_mode,
             pdyn->strgInterp.hw_cacT_edgeDetect_en ? "Y" : "N",
             pdyn->chromaAberrCorr.hw_cacT_negClip0_en ? "Y" : "N");
#elif defined(ISP_HW_V33)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", en ? "Y" : "N",
             base->bypass ? "Y" : "N", pdyn->edgeDetect.hw_cacT_edgeDetect_en ? "Y" : "N",
             pdyn->hfCalc.hw_cacT_negClip0_en ? "Y" : "N",
             pdyn->chromaAberrCorr.hw_cacT_wgtColor_en ? "Y" : "N");
#elif defined(ISP_HW_V35)
    snprintf(buffer, MAX_LINE_LENGTH, "%-5s%-8s%-16s%-14s%-14s", en ? "Y" : "N",
             base->bypass ? "Y" : "N", pdyn->edgeDetect.hw_cacT_edgeDetect_en ? "Y" : "N",
             pdyn->hfCalc.hw_cacT_negClip0_en ? "Y" : "N",
             pdyn->chromaAberrCorr.hw_cacT_wgtColor_en ? "Y" : "N");
#endif
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");
}

#if defined(USE_NEWSTRUCT)
void core_isp_params_dump_aec_params(AiqCore_t* self, st_string* result) {
    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_params_base_t* base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_AESTATS_PARAM];

    const rk_aiq_isp_ae_stats_cfg_t* ae_meas_cfg =  (rk_aiq_isp_ae_stats_cfg_t*)base->_data;

    aiq_info_dump_title(result, "aec hwi params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-11s%-9s", "aeswap", "aesel", "is_update", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    u16 aeswap = 0, aesel = 0;
    if(ae_meas_cfg->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl0DpcOut_mode) {
        aeswap = 0;

    } else if(ae_meas_cfg->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_entity0_chl1DpcOut_mode) {
        aeswap = 1;

    } else if(ae_meas_cfg->entityGroup.entities.entity0.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutLow_mode) {
        aeswap = 0x20;
    }

    if(ae_meas_cfg->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl0DpcOut_mode) {
        aesel = 0;

    } else if(ae_meas_cfg->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_entity3_chl1DpcOut_mode) {
        aesel = 1;

    } else if(ae_meas_cfg->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_dmIn_mode) {
        aesel = 3;

    } else if(ae_meas_cfg->entityGroup.entities.entity3.hw_aeCfg_statsSrc_mode ==  aeStats_btnrOutHigh_mode) {
        aesel = 0x10;
    }

    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-8d%-11s%-9s", aeswap, aesel,
             base->is_update ? "Y" : "N", ae_meas_cfg->sw_aeCfg_stats_en ? "Y" : "N" );
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13s%-13s%-13s%-13s", "entity", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawae0",
             ae_meas_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_x,
             ae_meas_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_y,
             ae_meas_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_width,
             ae_meas_cfg->entityGroup.entities.entity0.mainWin.hw_aeCfg_win_height);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawae3",
             ae_meas_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_x,
             ae_meas_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_y,
             ae_meas_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_width,
             ae_meas_cfg->entityGroup.entities.entity3.mainWin.hw_aeCfg_win_height);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawhist0",
             ae_meas_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_x,
             ae_meas_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_y,
             ae_meas_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_width,
             ae_meas_cfg->entityGroup.entities.entity0.hist.hw_aeCfg_win_height);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawhist3",
             ae_meas_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_x,
             ae_meas_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_y,
             ae_meas_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_width,
             ae_meas_cfg->entityGroup.entities.entity3.hist.hw_aeCfg_win_height);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

}

#else
void core_isp_params_dump_aec_params(AiqCore_t* self, st_string* result) {

    AiqAlgoHandler_t* handler = self->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
    if (!handler || !AiqAlgoHandler_getEnable(handler)) return;

    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_params_base_t* ae_base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_AEC_PARAM];
    const rk_aiq_isp_aec_params_t* ae_meas_cfg =    (rk_aiq_isp_aec_params_t*)ae_base->_data;

    aiq_params_base_t* hist_base = self->mAiqCurParams->pParamsArray[RESULT_TYPE_HIST_PARAM];
    const rk_aiq_isp_hist_params_t* hist_meas_cfg = (rk_aiq_isp_hist_params_t*)hist_base->_data;


    aiq_info_dump_title(result, "aec hwi params");

    snprintf(buffer, MAX_LINE_LENGTH, "%-8s%-8s%-11s%-9s", "aeswap", "aesel", "is_update", "en");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-8d%-8d%-11s%-9s", ae_meas_cfg->rawae0.rawae_sel, ae_meas_cfg->rawae3.rawae_sel,, ae_base->is_update ? "Y" : "N",
             ae_meas_cfg->ae_meas_en ? "Y" : "N" );
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13s%-13s%-13s%-13s", "entity", "win_off_h", "win_off_v", "win_size_h", "win_size_v");
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawae0",
             ae_meas_cfg->rawae0.win.h_offs, ae_meas_cfg->rawae0.win.v_offs,
             ae_meas_cfg->rawae0.win.h_size, ae_meas_cfg->rawae0.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawae3",
             ae_meas_cfg->rawae3.win.h_offs, ae_meas_cfg->rawae3.win.v_offs,
             ae_meas_cfg->rawae3.win.h_size, ae_meas_cfg->rawae3.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawhist0",
             hist_meas_cfg->rawhist0.win.h_offs, hist_meas_cfg->rawhist0.win.v_offs,
             hist_meas_cfg->rawhist0.win.h_size, hist_meas_cfg->rawhist0.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-15s%-13d%-13d%-13d%-13d", "rawhist3",
             hist_meas_cfg->rawhist3.win.h_offs, hist_meas_cfg->rawhist3.win.v_offs,
             hist_meas_cfg->rawhist3.win.h_size, hist_meas_cfg->rawhist3.win.v_size);
    aiq_string_printf(result, buffer);
    aiq_string_printf(result, "\n\n");

}
#endif

#define DUMP_INFO(_type, _func) \
    [_type] = {                 \
        .type = _type,          \
        .dump = _func,          \
    }

struct params_dump_info {
    int32_t type;
    const char* name;
    void (*dump)(AiqCore_t* self, st_string* result);
};

static const struct params_dump_info params_dump[] = {
    DUMP_INFO(RESULT_TYPE_DEBAYER_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_AESTATS_PARAM, core_isp_params_dump_aec_params),
    DUMP_INFO(RESULT_TYPE_AWB_PARAM, core_isp_params_dump_rawawb_params),
    DUMP_INFO(RESULT_TYPE_AWBGAIN_PARAM, core_isp_params_dump_awbgain_params),
    DUMP_INFO(RESULT_TYPE_CCM_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_AF_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_TNR_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_YNR_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_UVNR_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_MERGE_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_HISTEQ_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_SHARPEN_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_BLC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_CSM_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_MOTION_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_DPCC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_AGAMMA_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_LSC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_DRC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_GIC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_CGC_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_IE_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_GAIN_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_CAC_PARAM, core_isp_params_dump_cac_params),
#if defined(ISP_HW_V39)
    DUMP_INFO(RESULT_TYPE_DEHAZE_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_RGBIR_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_CP_PARAM, NULL),
    DUMP_INFO(RESULT_TYPE_LUT3D_PARAM, NULL),
#endif
    DUMP_INFO(RESULT_TYPE_LDC_PARAM, core_isp_params_dump_ldc_params),
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define MODS_SIZE ARRAY_SIZE(params_dump)

int core_isp_params_dump(void* self, st_string* result, int argc, void* argv[]) {
    core_isp_params_dump_mod_param((AiqCore_t*)self, result);
    core_isp_params_dump_ldc_params((AiqCore_t*)self, result);
    core_isp_params_dump_awbgain_params((AiqCore_t*)self, result);
    core_isp_params_dump_rawawb_params((AiqCore_t*)self, result);
    core_isp_params_dump_cac_params((AiqCore_t*)self, result);
    core_isp_params_dump_aec_params((AiqCore_t*)self, result);
    return 0;
}

void core_isp_params_dump_by_type(void* self, int type, st_string* result) {
    for (size_t pos = 0; pos < MODS_SIZE; pos++) {
        if ((params_dump[pos].type == type) && params_dump[pos].dump) {
            const char* name = aiq_notifier_module_name(&((AiqCore_t*)self)->notifier,
                               Cam3aResult2NotifierType[type]);
            aiq_info_dump_mod_name(result, name);
            aiq_info_dump_submod_name(result, "ALGO -> isp_result_params");
            core_isp_params_dump_mod_param((AiqCore_t*)self, result);

            params_dump[pos].dump((AiqCore_t*)self, result);
        }
    }
}
