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

#include "hwi_api_lsc21.h"
#include "newStruct/lsc/include/lsc_algo_api.h"

RKAIQ_BEGIN_DECLARE

int hwi_api_lsc21_params_check(lsc_param_t* attr)
{
    // TODO
    return 0;
}

void hwi_api_lsc21_params_dump(lsc_param_t* attr, struct isp3x_lsc_cfg *phwcfg)
{
    // TODO
}

void hwi_api_lsc21_params_cvt(lsc_param_t* attr, struct isp32_isp_params_cfg* isp_cfg)
{
    struct isp3x_lsc_cfg *phwcfg = &isp_cfg->others.lsc_cfg;
    lsc_params_static_t* psta = &attr->sta;
    phwcfg->sector_16x16 = true;
    memcpy(phwcfg->x_size_tbl, psta->lscHwConf.x_size_tbl, sizeof(phwcfg->x_size_tbl));
    memcpy(phwcfg->y_size_tbl, psta->lscHwConf.y_size_tbl, sizeof(phwcfg->y_size_tbl));
    memcpy(phwcfg->x_grad_tbl, psta->lscHwConf.x_grad_tbl, sizeof(phwcfg->x_grad_tbl));
    memcpy(phwcfg->y_grad_tbl, psta->lscHwConf.y_grad_tbl, sizeof(phwcfg->y_grad_tbl));

    memcpy(phwcfg->r_data_tbl, psta->lscHwConf.r_data_tbl, sizeof(psta->lscHwConf.r_data_tbl));
    memcpy(phwcfg->gr_data_tbl, psta->lscHwConf.gr_data_tbl, sizeof(psta->lscHwConf.gr_data_tbl));
    memcpy(phwcfg->gb_data_tbl, psta->lscHwConf.gb_data_tbl, sizeof(psta->lscHwConf.gb_data_tbl));
    memcpy(phwcfg->b_data_tbl, psta->lscHwConf.b_data_tbl, sizeof(psta->lscHwConf.b_data_tbl));
#ifndef MAX_LSC_VALUE
#define MAX_LSC_VALUE 8191
#endif
if(psta->lscHwConf.lsc_en) {
    for(int i = 0; i < ISP3X_LSC_DATA_TBL_SIZE; i++) {
        phwcfg->b_data_tbl[i] = MIN(phwcfg->b_data_tbl[i], MAX_LSC_VALUE);
        phwcfg->gb_data_tbl[i] = MIN(phwcfg->gb_data_tbl[i], MAX_LSC_VALUE);
        phwcfg->r_data_tbl[i] = MIN(phwcfg->r_data_tbl[i], MAX_LSC_VALUE);
        phwcfg->gr_data_tbl[i] = MIN(phwcfg->gr_data_tbl[i], MAX_LSC_VALUE);
    }
}
}

RKAIQ_END_DECLARE