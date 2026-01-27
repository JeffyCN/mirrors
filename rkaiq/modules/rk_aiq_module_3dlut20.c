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

void rk_aiq_lut3d20_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp2x_3dlut_cfg *phwcfg = &isp_params->isp_cfg->others.isp3dlut_cfg;
    lut3d_param_t* lut3d = (lut3d_param_t*)attr;

    // phwcfg->bypass_en   = false;
    phwcfg->actual_size = RK_AIQ_3DLUT20_LUT_WSIZE;

    memcpy(phwcfg->lut_r, lut3d->dyn.meshGain.hw_lut3dC_lutR_val, sizeof(phwcfg->lut_r));
    memcpy(phwcfg->lut_g, lut3d->dyn.meshGain.hw_lut3dC_lutG_val, sizeof(phwcfg->lut_g));
    memcpy(phwcfg->lut_b, lut3d->dyn.meshGain.hw_lut3dC_lutB_val, sizeof(phwcfg->lut_b));

    return;
}

RKAIQ_END_DECLARE
