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

#include "rk_aiq_isp32_modules.h"

void rk_aiq_drc32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    struct isp32_drc_cfg *phwcfg = &isp_cfg->others.drc_cfg;
    drc_param_t *drc_attrib = (drc_param_t*) attr;
    drc_params_static_t* psta = &drc_attrib->sta;
    drc_params_dyn_t* pdyn = &drc_attrib->dyn;
    uint16_t tmp;

    phwcfg->bypass_en   = 0;
    phwcfg->position = (unsigned short)(pdyn->DrcGain.Clip * (1 << 8) + 0.5);
    phwcfg->compres_scl = (unsigned short)psta->compres_scl;
    phwcfg->offset_pow2 = psta->offset_pow2;
    phwcfg->delta_scalein = psta->delta_scalein;
    tmp = (unsigned short)pdyn->LocalData.GlobalContrast * 4095;
    phwcfg->lpdetail_ratio = tmp > 4095 ? 4095 : tmp;
    tmp = (unsigned short)pdyn->LocalData.LoLitContrast * 4095;
    phwcfg->hpdetail_ratio = tmp > 4095 ? 4095 : tmp;
    tmp = (unsigned short)pdyn->MotionData.MotionStr * 255;
    phwcfg->bilat_wt_off = tmp > 255 ? 255 : tmp;

    tmp = (unsigned short)psta->LocalSetting.curPixWeit * 255;
    phwcfg->weicur_pix = tmp > 255 ? 255 : tmp;
    tmp = (unsigned short)psta->LocalSetting.preFrameWeit * 255;
    phwcfg->weipre_frame = tmp > 255 ? 255 : tmp;
    phwcfg->force_sgm_inv0 =
        (unsigned short)(psta->LocalSetting.Range_force_sgm * 8191 + 0.5f);
    phwcfg->motion_scl = 0;
    tmp = (unsigned short)psta->Edge_Weit * 255;
    phwcfg->edge_scl = tmp > 255 ? 255 : tmp;
    phwcfg->space_sgm_inv1  = (unsigned short)psta->LocalSetting.Space_sgm_cur;
    phwcfg->space_sgm_inv0  = (unsigned short)psta->LocalSetting.Space_sgm_pre;
    phwcfg->range_sgm_inv1 =
        (unsigned short)(psta->LocalSetting.Range_sgm_cur * 8191 + 0.5f);
    phwcfg->range_sgm_inv0 =
        (unsigned short)(psta->LocalSetting.Range_sgm_pre * 8191 + 0.5f);
    phwcfg->weig_maxl = (unsigned short)(pdyn->HiLightData.Strength * 16 + 0.5f);
    tmp = (unsigned short)pdyn->LocalData.LocalWeit * 16;
    phwcfg->weig_bilat = tmp > 16 ? 16 : tmp;
    phwcfg->enable_soft_thd = pdyn->LocalData.LocalAutoEnable;
    tmp = (unsigned short)pdyn->LocalData.LocalAutoWeit * 16383;
    phwcfg->bilat_soft_thd = tmp > 16383 ? 16383 : tmp;
    phwcfg->iir_weight =
        (unsigned char)((psta->IIR_frame - 1.0f) / psta->IIR_frame  + 0.5f);
    phwcfg->min_ogain = (unsigned short)(psta->sw_drc_min_ogain * (1 << 15) + 0.5f);

    for (int i = 0; i < ISP32_DRC_Y_NUM; i++) {
        phwcfg->gain_y[i]    = psta->gain_y[i];
        phwcfg->compres_y[i] = psta->compres_y[i];
        phwcfg->scale_y[i]   = (unsigned short)psta->Scale_y[i];
    }

    // drc v12 add
    tmp = pdyn->HiLightData.gas_t * 2048;
    phwcfg->gas_t = (unsigned short)(tmp * (1 << 8) + 0.5);
    phwcfg->gas_l0          = (unsigned char)psta->HiLight.gas_l0;
    phwcfg->gas_l1          = (unsigned char)psta->HiLight.gas_l1;
    phwcfg->gas_l2          = (unsigned char)psta->HiLight.gas_l2;
    phwcfg->gas_l3          = (unsigned char)psta->HiLight.gas_l3;
    
}

