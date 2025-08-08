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

void rk_aiq_csm21_params_cvt(void* attr, isp_params_t* isp_params)
{
    struct isp21_csm_cfg *phwcfg = &isp_params->isp_cfg->others.csm_cfg;
    csm_params_static_t *psta = &((csm_param_t*)attr)->sta;

    phwcfg->csm_full_range = psta->hw_csmT_full_range ? 1 : 0;
    phwcfg->csm_y_offset   = psta->hw_csmT_y_offset;
    phwcfg->csm_c_offset   = psta->hw_csmT_c_offset;
    for (int i = 0; i < RK_AIQ_CSM_COEFF_NUM; i++) {
        phwcfg->csm_coeff[i] = psta->sw_csmT_coeff[i] > 0
                                ? (short)(psta->sw_csmT_coeff[i] * 128 + 0.5)
                                : (short)(psta->sw_csmT_coeff[i] * 128 - 0.5);
    }

    return;
}
