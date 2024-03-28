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

#include "rk_aiq_isp39_modules.h"

void rk_aiq_drc40_params_dump(void* attr, struct isp39_isp_params_cfg* isp_cfg) {

    LOG1_ATMO(
        "%s: bypass_en:%d gainx32_en:%d raw_dly_dis:%d sw_drc_position:%d sw_drc_compres_scl:%d "
        "sw_drc_offset_pow2:%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.bypass_en, isp_cfg->others.drc_cfg.gainx32_en,
        isp_cfg->others.drc_cfg.raw_dly_dis, isp_cfg->others.drc_cfg.position,
        isp_cfg->others.drc_cfg.compres_scl, isp_cfg->others.drc_cfg.offset_pow2);
    LOG1_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, isp_cfg->others.drc_cfg.lpdetail_ratio,
              isp_cfg->others.drc_cfg.hpdetail_ratio, isp_cfg->others.drc_cfg.delta_scalein);
    LOG1_ATMO(
        "%s: sw_drc_bilat_wt_off:%d thumb_thd_neg:%d thumb_thd_enable:%d sw_drc_weicur_pix:%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.bilat_wt_off, isp_cfg->others.drc_cfg.thumb_thd_neg,
        isp_cfg->others.drc_cfg.thumb_thd_enable, isp_cfg->others.drc_cfg.weicur_pix);
    LOG1_ATMO(
        "%s: cmps_byp_en:%d cmps_offset_bits_int:%d cmps_fixbit_mode:%d thumb_clip:%d "
        "thumb_scale:%d "
        "\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.cmps_byp_en,
        isp_cfg->others.drc_cfg.cmps_offset_bits_int, isp_cfg->others.drc_cfg.cmps_fixbit_mode,
        isp_cfg->others.drc_cfg.thumb_clip,
        isp_cfg->others.drc_cfg.thumb_scale);
    LOG1_ATMO(
        "%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d weig_bilat:%d weight_8x8thumb:%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.range_sgm_inv0, isp_cfg->others.drc_cfg.range_sgm_inv1,
        isp_cfg->others.drc_cfg.weig_bilat, isp_cfg->others.drc_cfg.weight_8x8thumb);
    LOG1_ATMO(
        "%s: sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d sw_drc_min_ogain:%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.bilat_soft_thd, isp_cfg->others.drc_cfg.enable_soft_thd,
        isp_cfg->others.drc_cfg.min_ogain);
    LOG1_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_cfg->others.drc_cfg.gain_y[0], isp_cfg->others.drc_cfg.gain_y[1],
              isp_cfg->others.drc_cfg.gain_y[2], isp_cfg->others.drc_cfg.gain_y[3],
              isp_cfg->others.drc_cfg.gain_y[4], isp_cfg->others.drc_cfg.gain_y[5],
              isp_cfg->others.drc_cfg.gain_y[6], isp_cfg->others.drc_cfg.gain_y[7],
              isp_cfg->others.drc_cfg.gain_y[8], isp_cfg->others.drc_cfg.gain_y[9],
              isp_cfg->others.drc_cfg.gain_y[10], isp_cfg->others.drc_cfg.gain_y[11],
              isp_cfg->others.drc_cfg.gain_y[12], isp_cfg->others.drc_cfg.gain_y[13],
              isp_cfg->others.drc_cfg.gain_y[14], isp_cfg->others.drc_cfg.gain_y[15],
              isp_cfg->others.drc_cfg.gain_y[16]);
    LOG1_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_cfg->others.drc_cfg.scale_y[0], isp_cfg->others.drc_cfg.scale_y[1],
              isp_cfg->others.drc_cfg.scale_y[2], isp_cfg->others.drc_cfg.scale_y[3],
              isp_cfg->others.drc_cfg.scale_y[4], isp_cfg->others.drc_cfg.scale_y[5],
              isp_cfg->others.drc_cfg.scale_y[6], isp_cfg->others.drc_cfg.scale_y[7],
              isp_cfg->others.drc_cfg.scale_y[8], isp_cfg->others.drc_cfg.scale_y[9],
              isp_cfg->others.drc_cfg.scale_y[10], isp_cfg->others.drc_cfg.scale_y[11],
              isp_cfg->others.drc_cfg.scale_y[12], isp_cfg->others.drc_cfg.scale_y[13],
              isp_cfg->others.drc_cfg.scale_y[14], isp_cfg->others.drc_cfg.scale_y[15],
              isp_cfg->others.drc_cfg.scale_y[16]);
    LOG1_ATMO(
        "%s: sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.compres_y[0], isp_cfg->others.drc_cfg.compres_y[1],
        isp_cfg->others.drc_cfg.compres_y[2], isp_cfg->others.drc_cfg.compres_y[3],
        isp_cfg->others.drc_cfg.compres_y[4], isp_cfg->others.drc_cfg.compres_y[5],
        isp_cfg->others.drc_cfg.compres_y[6], isp_cfg->others.drc_cfg.compres_y[7],
        isp_cfg->others.drc_cfg.compres_y[8], isp_cfg->others.drc_cfg.compres_y[9],
        isp_cfg->others.drc_cfg.compres_y[10], isp_cfg->others.drc_cfg.compres_y[11],
        isp_cfg->others.drc_cfg.compres_y[12], isp_cfg->others.drc_cfg.compres_y[13],
        isp_cfg->others.drc_cfg.compres_y[14], isp_cfg->others.drc_cfg.compres_y[15],
        isp_cfg->others.drc_cfg.compres_y[16]);
    LOG1_ATMO(
        "%s: sfthd_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_cfg->others.drc_cfg.sfthd_y[0], isp_cfg->others.drc_cfg.sfthd_y[1],
        isp_cfg->others.drc_cfg.sfthd_y[2], isp_cfg->others.drc_cfg.sfthd_y[3],
        isp_cfg->others.drc_cfg.sfthd_y[4], isp_cfg->others.drc_cfg.sfthd_y[5],
        isp_cfg->others.drc_cfg.sfthd_y[6], isp_cfg->others.drc_cfg.sfthd_y[7],
        isp_cfg->others.drc_cfg.sfthd_y[8], isp_cfg->others.drc_cfg.sfthd_y[9],
        isp_cfg->others.drc_cfg.sfthd_y[10], isp_cfg->others.drc_cfg.sfthd_y[11],
        isp_cfg->others.drc_cfg.sfthd_y[12], isp_cfg->others.drc_cfg.sfthd_y[13],
        isp_cfg->others.drc_cfg.sfthd_y[14], isp_cfg->others.drc_cfg.sfthd_y[15],
        isp_cfg->others.drc_cfg.sfthd_y[16]);
}

void rk_aiq_drc40_params_cvt(void* attr, struct isp39_isp_params_cfg* isp_cfg, common_cvt_info_t* cvtinfo)
{
    struct isp39_drc_cfg *phwcfg = &isp_cfg->others.drc_cfg;
    drc_param_t *drc_param = &((rk_aiq_isp_drc_v39_t*)attr)->drc_param;
    drc_params_dyn_t *pdyn = &drc_param->dyn;
    trans_api_attrib_t* trans_attr = &((rk_aiq_isp_drc_v39_t*)attr)->trans_attr;
    trans_params_static_t *psta = &trans_attr->stMan.sta;

    uint16_t tmp;
    phwcfg->cmps_byp_en = (trans_attr->en == 1 && trans_attr->bypass == 0);
    phwcfg->offset_pow2 =
        CLIP(psta->hw_transCfg_transOfDrc_offset, 0, (1 << 4) - 1);
    tmp = psta->hw_transCfg_lscOutTrans_offset;
    phwcfg->cmps_offset_bits_int = tmp > 15 ? 15 : tmp;
    phwcfg->cmps_fixbit_mode =
        psta->hw_transCfg_trans_mode == trans_lgi3f9_mode ? 1 : 0;
    LOGD_ATMO("phwcfg->cmps_byp_en %d, phwcfg->offset_pow2 %d, phwcfg->cmps_offset_bits_int %d\n",
        phwcfg->cmps_byp_en, phwcfg->offset_pow2, phwcfg->cmps_offset_bits_int);

    phwcfg->bypass_en   = 0;
    phwcfg->gainx32_en  = 0;
    phwcfg->raw_dly_dis = 0;

    phwcfg->position =
        CLIP(pdyn->preProc.hw_drcT_toneCurveIdx_scale * (1 << 8), 0, (1 << 14) - 1);
    phwcfg->compres_scl = (unsigned short)pdyn->preProc.hw_drcT_drcCurveLuma_scale;

    tmp = pdyn->drcProc.hw_drcT_loDetail_strg;
    phwcfg->lpdetail_ratio = tmp > 4095 ? 4095 : tmp;
    tmp = pdyn->drcProc.hw_drcT_drcStrg_alpha;
    phwcfg->hpdetail_ratio = tmp > 4095 ? 4095 : tmp;
    phwcfg->delta_scalein = pdyn->drcProc.hw_drcT_drcStrgLutLuma_scale;
    phwcfg->min_ogain =
        CLIP((int)(pdyn->drcProc.hw_drcT_drcGain_minLimit * (1 << 15)), 0, (1 << 15) - 1);
    tmp = pdyn->preProc.hw_drcT_lpfSoftThd_thred;
    phwcfg->drc_gas_t = tmp > 1023 ? 1023 : tmp;

    phwcfg->bilat_wt_off = pdyn->bifilt_filter.hw_drcT_rgeWgt_negOff;
    phwcfg->weicur_pix = pdyn->bifilt_filter.hw_drcT_centerPixel_wgt;
    tmp = pdyn->bifilt_filter.hw_drcT_midRgeSgm_val;
    phwcfg->range_sgm_inv0 = tmp > 1023 ? 1023 : tmp;
    tmp = pdyn->bifilt_filter.hw_drcT_loRgeSgm_val;
    phwcfg->range_sgm_inv1 = tmp > 1023 ? 1023 : tmp;
    tmp = pdyn->bifilt_filter.hw_drcT_bifiltOut_alpha;
    phwcfg->weig_bilat = tmp > 15 ? 15 : tmp;
    phwcfg->weight_8x8thumb =
        pdyn->bifilt_filter.hw_drcT_midWgt_alpha;
    tmp = pdyn->bifilt_filter.hw_drcT_softThd_thred;
    phwcfg->bilat_soft_thd = tmp > 2047 ? 2047 : tmp;
    phwcfg->enable_soft_thd = pdyn->bifilt_filter.hw_drcT_softThd_en;

    tmp = pdyn->bifilt_guideDiff.hw_drcT_guideDiff_minLimit;
    phwcfg->thumb_thd_neg = tmp > 511 ? 511 : tmp;
    phwcfg->thumb_thd_enable =
        pdyn->bifilt_guideDiff.hw_drcT_guideDiffLmt_en ? 1 : 0;
    tmp = pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit;
    phwcfg->thumb_clip = tmp > 4095 ? 4095 : tmp;
    phwcfg->thumb_scale = pdyn->bifilt_guideDiff.hw_drcT_maxLutIdx_scale;
    if (pdyn->bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode == drc_cfgCurveCtrlCoeff_mode) {
        float tmp = 0.0f;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            tmp = pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit -
                (pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_maxLimit -
                 pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_minLimit) /
                    (1.0f +
                     pow(2.718f, -pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_slope *
                        (i / 16.0f - pdyn->bifilt_guideDiff.sw_drcT_maxLutCreate_offset)));
            phwcfg->sfthd_y[i] = (unsigned short)(tmp * (255 + 1));
        }
    } else if (pdyn->bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode == drc_cfgCurveDirect_mode) {
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            phwcfg->sfthd_y[i] =
                (unsigned short)(pdyn->bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i]);
        }
    }

    for (int i = 0; i < DRC_CURVE_LEN; ++i) {
        phwcfg->gain_y[i] = pdyn->preProc.hw_drcT_luma2ToneGain_val[i];
        phwcfg->compres_y[i] = pdyn->drcProc.hw_drcT_hdr2Sdr_curve[i];
        phwcfg->scale_y[i] =
            (unsigned short)(pdyn->drcProc.hw_drcT_luma2DrcStrg_val[i]);
    }

    rk_aiq_drc40_params_dump(attr, isp_cfg);
}

