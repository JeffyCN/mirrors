/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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

#include "rk_aiq_isp33_modules.h"

RKAIQ_BEGIN_DECLARE

#define LIMIT_VALUE_UNSIGNED(value, max_value) (value > max_value ? max_value : value)
#define DETAIL2STRG_VAL_DEFAULT (1024)
#define LUMA2STRG_VAL_DEFAULT   (1024)
#define DETAIL2STRG_POWER_MAX   (10)
    
    void rk_aiq_enh30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo)
{
#if defined(ISP_HW_V35)
    struct isp35_enh_cfg *pFix = &isp_params->isp_cfg->others.enh_cfg;
#else
    struct isp33_enh_cfg *pFix = &isp_params->isp_cfg->others.enh_cfg;
#endif
    enh_param_t* enh_attrib = (enh_param_t*) attr;
    enh_params_dyn_t *pdyn = &enh_attrib->dyn;
    int tmp;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;

    /* CTRL */
    pFix->blf3_bypass = !pdyn->loBifilt.hw_enhT_loBlf_en;
    /* IIR_FLT */
    tmp = (int)(256.0f / MIN(pow(2.0f, pdyn->guideImg_iir.spatial.hw_enhT_sigma_val), 256.0f));
    pFix->iir_inv_sigma = tmp > 0x100 ? 0x100 : tmp;
    tmp = ClipFloatValue(pdyn->guideImg_iir.spatial.hw_enhT_softThd_val, 5, 0, false);
    pFix->iir_soft_thed = tmp;
    tmp = ClipFloatValue(pdyn->guideImg_iir.spatial.hw_enhT_centerPix_wgt, 0, 5, true);
    pFix->iir_cur_wgt = tmp;
    /* BILAT_FLT3X3 */
    tmp = (int)(256.0f / MIN(pow(2.0f, pdyn->loBifilt.hw_enhT_sigma_val), 256.0f));
    pFix->blf3_inv_sigma     = tmp > 0x100 ? 0x100 : tmp;
    tmp                      = ClipFloatValue(pdyn->loBifilt.hw_enhT_centerPix_wgt, 0, 8, true);
    pFix->blf3_cur_wgt = tmp;
    tmp = ClipFloatValue(pdyn->guideImg_iir.temporal.hw_enhT_iirFrm_maxLimit, 4, 0, false);
    pFix->blf3_thumb_cur_wgt = tmp;
    /* BILAT_FLT5X5 */
    tmp                = ClipFloatValue(pdyn->midBifilt.hw_enhT_centerPix_wgt, 0, 5, true);
    pFix->blf5_cur_wgt = tmp;
    tmp                = (int)(256.0f / MIN(pow(2.0f, pdyn->midBifilt.hw_enhT_sigma_val), 256.0f));
    pFix->blf5_inv_sigma = tmp > 0x100 ? 0x100 : tmp;
    /* GLOBAL_STRG */
    tmp               = ClipFloatValue(pdyn->strg.hw_enhT_global_strg, 4, 10, false);
    pFix->global_strg = tmp;
    /* LUMA_LUT */
    if (pdyn->strg.hw_enhT_luma2Strg_en) {
        for (int i = 0; i < ISP33_ENH_LUMA_NUM; i++) {
            tmp               = ClipFloatValue(pdyn->strg.hw_enhT_luma2Strg_val[i], 0, 10, true);
            pFix->lum2strg[i] = tmp;
        }
    } else {
        for (int i = 0; i < ISP33_ENH_LUMA_NUM; i++) {
            pFix->lum2strg[i] = LUMA2STRG_VAL_DEFAULT;
        }
    }

    /* DETAIL_IDX */
    for (int i = 0; i < ISP33_ENH_DETAIL_NUM - 1; i++) {
        tmp = ClipFloatValue(pdyn->strg.hw_enhT_detail2Strg_curve.idx[i], 10, 0, false);
        pFix->detail2strg_idx[i] = tmp;
    }
    tmp = ClipFloatValue(pdyn->strg.hw_enhT_detail2Strg_curve.idx[ISP33_ENH_DETAIL_NUM - 1], 10, 0,
                         true);
    pFix->detail2strg_idx[7] = tmp;
    /* DETAIL_POWER */
    float tmp_float;
    uint8_t detail2strg_power[7];
    for (int i = 0; i < 7; ++i) {
        tmp_float = (float)(pFix->detail2strg_idx[i + 1] - pFix->detail2strg_idx[i]);
        detail2strg_power[i] = LIMIT_VALUE_UNSIGNED(
            (unsigned char)(log(tmp_float) / log(2.0f)), DETAIL2STRG_POWER_MAX);
    }
    pFix->detail2strg_power0 = detail2strg_power[0];
    pFix->detail2strg_power1 = detail2strg_power[1];
    pFix->detail2strg_power2 = detail2strg_power[2];
    pFix->detail2strg_power3 = detail2strg_power[3];
    pFix->detail2strg_power4 = detail2strg_power[4];
    pFix->detail2strg_power5 = detail2strg_power[5];
    pFix->detail2strg_power6 = detail2strg_power[6];
    /* DETAIL_VALUE */
    if (pdyn->strg.hw_enhT_detail2Strg_en) {
        for (int i = 0; i < ISP33_ENH_DETAIL_NUM; i++) {
            tmp = ClipFloatValue(pdyn->strg.hw_enhT_detail2Strg_curve.val[i], 0, 10, true);
            pFix->detail2strg_val[i] = tmp;
        }
    } else {
        for (int i = 0; i < ISP33_ENH_DETAIL_NUM; i++) {
            pFix->detail2strg_val[i] = DETAIL2STRG_VAL_DEFAULT;
        }
    }
    /* PRE_FRAME */
    // pFix->pre_wet_frame_cnt0 = pdyn->pre_wet_frame_cnt0;
    // pFix->pre_wet_frame_cnt1 = pdyn->pre_wet_frame_cnt1;
    pFix->iir_wr = 0;
}

RKAIQ_END_DECLARE
