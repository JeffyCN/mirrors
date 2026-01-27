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

#include "rk_aiq_isp39_modules.h"

void rk_aiq_ccm22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
#if ISP_HW_V39
    struct isp39_ccm_cfg* cfg = &isp_params->isp_cfg->others.ccm_cfg;
#elif ISP_HW_V33
    struct isp33_ccm_cfg* cfg = &isp_params->isp_cfg->others.ccm_cfg;
#elif ISP_HW_V35
    struct isp33_ccm_cfg* cfg = &isp_params->isp_cfg->others.ccm_cfg;
#endif

    ccm_param_t* ccm_param = (ccm_param_t*)attr;
    ccm_param_dyn_t* pdyn = &ccm_param->dyn;
    ccm_param_static_t* psta = &ccm_param->sta;


    const float* coeff        = pdyn->ccMatrix.hw_ccmC_matrix_coeff;
    const float* offset       = pdyn->ccMatrix.hw_ccmC_matrix_offset;

    cfg->coeff0_r = (coeff[0] - 1) > 0 ? (short)((coeff[0] - 1) * 128 + 0.5)
                    : (short)((coeff[0] - 1) * 128 - 0.5);  // check -128?
    cfg->coeff1_r = coeff[1] > 0 ? (short)(coeff[1] * 128 + 0.5) : (short)(coeff[1] * 128 - 0.5);
    cfg->coeff2_r = coeff[2] > 0 ? (short)(coeff[2] * 128 + 0.5) : (short)(coeff[2] * 128 - 0.5);
    cfg->coeff0_g = coeff[3] > 0 ? (short)(coeff[3] * 128 + 0.5) : (short)(coeff[3] * 128 - 0.5);
    cfg->coeff1_g = (coeff[4] - 1) > 0 ? (short)((coeff[4] - 1) * 128 + 0.5)
                    : (short)((coeff[4] - 1) * 128 - 0.5);
    cfg->coeff2_g = coeff[5] > 0 ? (short)(coeff[5] * 128 + 0.5) : (short)(coeff[5] * 128 - 0.5);
    cfg->coeff0_b = coeff[6] > 0 ? (short)(coeff[6] * 128 + 0.5) : (short)(coeff[6] * 128 - 0.5);
    cfg->coeff1_b = coeff[7] > 0 ? (short)(coeff[7] * 128 + 0.5) : (short)(coeff[7] * 128 - 0.5);
    cfg->coeff2_b = (coeff[8] - 1) > 0 ? (short)((coeff[8] - 1) * 128 + 0.5)
                    : (short)((coeff[8] - 1) * 128 - 0.5);

    cfg->offset_r =
        offset[0] > 0 ? (short)(offset[0] + 0.5) : (short)(offset[0] - 0.5);  // for 12bit
    cfg->offset_g = offset[1] > 0 ? (short)(offset[1] + 0.5) : (int)(offset[1] - 0.5);
    cfg->offset_b = offset[2] > 0 ? (short)(offset[2] + 0.5) : (short)(offset[2] - 0.5);

    
    cfg->bound_bit        = pdyn->ccmAlpha_yFac.hw_ccmT_facMax_minThred;
    cfg->right_bit        = pdyn->ccmAlpha_yFac.hw_ccmT_facMax_maxThred;

    bool same_curve = 0 == memcmp(pdyn->ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0,
            pdyn->ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0, sizeof(pdyn->ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0));
    if (same_curve == cvtinfo->pre_ccm_asym_adj_en || cvtinfo->isFirstFrame) {
        if (same_curve) {
            cfg->asym_adj_en = 0;
            for (int i = 0; i < ISP32_CCM_CURVE_NUM; i++) {
                cfg->alp_y[i] = (u16)(pdyn->ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i]);
            }
            cfg->alp_y[17] = 1024;
        } else {
            cfg->asym_adj_en = 1;
            for (int i = 0; i < ISP32_CCM_CURVE_NUM/2; i++) {
                cfg->alp_y[i] = pdyn->ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i*2];
            }
            for (int i = 0; i < ISP32_CCM_CURVE_NUM/2; i++) {
                cfg->alp_y[ISP32_CCM_CURVE_NUM - 1 - i] = pdyn->ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0[i*2];
            }
        }
        if (cvtinfo->isFirstFrame) {
            memcpy(cvtinfo->ccm_pre_Y2Alpha_fac0, cfg->alp_y, sizeof(cfg->alp_y));
            cvtinfo->pre_ccm_asym_adj_en = same_curve;
        }
    } else {
        memcpy(cfg->alp_y, cvtinfo->ccm_pre_Y2Alpha_fac0, sizeof(cfg->alp_y));
        if (same_curve) {
            cfg->asym_adj_en = 0;
            for (int i = 0; i < ISP32_CCM_CURVE_NUM; i++) {
                cvtinfo->ccm_pre_Y2Alpha_fac0[i] = (u16)(pdyn->ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i]);
            }
            cvtinfo->ccm_pre_Y2Alpha_fac0[17] = 1024;
        } else {
            cfg->asym_adj_en = 1;
            for (int i = 0; i < ISP32_CCM_CURVE_NUM/2; i++) {
                cvtinfo->ccm_pre_Y2Alpha_fac0[i] = pdyn->ccmAlpha_yFac.hw_ccmT_loY2Alpha_fac0[i*2];
            }
            for (int i = 0; i < ISP32_CCM_CURVE_NUM/2; i++) {
                cvtinfo->ccm_pre_Y2Alpha_fac0[ISP32_CCM_CURVE_NUM - 1 - i] = pdyn->ccmAlpha_yFac.hw_ccmT_hiY2Alpha_fac0[i*2];
            }
        }
        cvtinfo->pre_ccm_asym_adj_en = same_curve;
    }

    cfg->coeff0_y         = (u16)psta->hw_ccmCfg_rgb2y_coeff[0];
    cfg->coeff1_y         = (u16)psta->hw_ccmCfg_rgb2y_coeff[1];
    cfg->coeff2_y         = (u16)psta->hw_ccmCfg_rgb2y_coeff[2];

    cfg->hf_up        = pdyn->ccmAlpha_satFac.hw_ccmT_facMax_thred;
    cfg->hf_low       = pdyn->ccmAlpha_satFac.hw_ccmT_satIdx_maxLimit;
    cfg->hf_scale     = (u16)(pdyn->ccmAlpha_satFac.hw_ccmT_satIdx_scale * 256 + 0.5);
    for (int i = 0; i < ISP39_CCM_HF_FACTOR_NUM; i++) {
        cfg->hf_factor[i] = pdyn->ccmAlpha_satFac.hw_ccmT_sat2Alpha_fac1[i];
    }

    cfg->enh_adj_en  = 1;
    cfg->highy_adjust_dis = 0;
    cfg->sat_decay_en = 1;

    if (pdyn->enhance.hw_ccmT_enhance_en) {
        cfg->color_enh_rat_max = (u16)(pdyn->enhance.hw_ccmT_enhanceRat_maxLimit * 1024);
    } else {
        cfg->color_enh_rat_max = 1024;
    }
    cfg->color_coef0_r2y   = (u16)psta->hw_ccmCfg_rgb2y_coeff[0];
    cfg->color_coef1_g2y   = (u16)psta->hw_ccmCfg_rgb2y_coeff[1];
    cfg->color_coef2_b2y   = (u16)psta->hw_ccmCfg_rgb2y_coeff[2];
#if 0
    printf("ccm %f %f %f %f %f %f %f %f %f\n",
           coeff[0], coeff[1], coeff[2], coeff[3], coeff[4], coeff[5], coeff[6], coeff[7], coeff[8]);
    printf("bit=(%d, %d)\n", cfg->bound_bit, cfg->right_bit);
    printf("asym_adj_en=%d\n", cfg->asym_adj_en);
    printf("alp_y = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n",
           cfg->alp_y[0], cfg->alp_y[1], cfg->alp_y[2], cfg->alp_y[3], cfg->alp_y[4], cfg->alp_y[5],
           cfg->alp_y[6], cfg->alp_y[7], cfg->alp_y[8], cfg->alp_y[9], cfg->alp_y[10], cfg->alp_y[11],
           cfg->alp_y[12], cfg->alp_y[13], cfg->alp_y[14], cfg->alp_y[15], cfg->alp_y[16], cfg->alp_y[17]);
    printf("sat_apl1 = [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n", 
            cfg->hf_factor[0], cfg->hf_factor[1], cfg->hf_factor[2], cfg->hf_factor[3], cfg->hf_factor[4],
            cfg->hf_factor[5], cfg->hf_factor[6], cfg->hf_factor[7], cfg->hf_factor[8], cfg->hf_factor[9],
            cfg->hf_factor[10], cfg->hf_factor[11], cfg->hf_factor[12], cfg->hf_factor[13], cfg->hf_factor[14],
            cfg->hf_factor[15], cfg->hf_factor[16]);
    printf("sat adj range = [-%d, %d] * %d\n", cfg->hf_low, cfg->hf_up, cfg->hf_scale);
#endif
    return;
}
