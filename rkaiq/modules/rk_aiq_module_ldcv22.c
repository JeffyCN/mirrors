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

#include "isp/rk_aiq_isp_ldc22.h"
#include "rk_aiq_isp39_modules.h"
#include "xcore/base/xcam_log.h"

static const uint8_t ldcv_default_bic_table[9][4] = {
    {0x00, 0x80, 0x00, 0x00},  // table0: 0, 0, 128, 0
    {0xfc, 0x7f, 0x05, 0x00},  // table1: 0, 5, 127, -4
    {0xfa, 0x7b, 0x0c, 0xff},  // table2: -1, 12, 123, -6
    {0xf8, 0x76, 0x14, 0xfe},  // table3: -2, 20, 118, -8
    {0xf7, 0x6f, 0x1d, 0xfd},  // table4: -3, 29, 111, -9
    {0xf7, 0x66, 0x27, 0xfc},  // table4: -4, 39, 102, -9
    {0xf7, 0x5d, 0x32, 0xfa},  // table4: -6, 50, 93, -9
    {0xf7, 0x53, 0x3d, 0xf9},  // table4: -7, 61, 83, -9
    {0xf8, 0x48, 0x48, 0xf8},  // table4: -8, 72, 72, -8
};

void rk_aiq_ldcv22_params_cvt(const common_cvt_info_t* cvtinfo, void* attr,
                              isp_params_t* isp_params, isp_params_t* isp_cfg_right,
                              bool is_multi_isp) {
    struct isp39_ldcv_cfg* pHwCfg = &isp_params->isp_cfg->others.ldcv_cfg;
    ldc_ldcv_params_cfg_t* psta   = (ldc_ldcv_params_cfg_t*)attr;

    pHwCfg->hsize  = (((cvtinfo->rawWidth + 15) / 16 + 1) + 1) / 2;
    pHwCfg->vsize  = (cvtinfo->rawHeight + 15) / 16 + 2;
    pHwCfg->buf_fd = psta->lutMapCfg.sw_ldcT_lutMapBuf_fd[0];

    pHwCfg->thumb_mode   = 0;
    pHwCfg->dth_bypass   = 0;
    pHwCfg->map13p3_en   = 0;
    pHwCfg->force_map_en = 0;
    memcpy(pHwCfg->bicubic, ldcv_default_bic_table, sizeof(ldcv_default_bic_table));
    pHwCfg->out_vsize = cvtinfo->rawHeight;

    if (is_multi_isp) {
        struct isp39_ldch_cfg* cfg_right = &(isp_params->isp_cfg + 1)->others.ldch_cfg;
        memcpy(cfg_right, pHwCfg, sizeof(*cfg_right));
        cfg_right->buf_fd = psta->lutMapCfg.sw_ldcT_lutMapBuf_fd[1];
    }

    LOGD_ALDC("Dump LDCV config: ");
    LOGD_ALDC("thumb_mode: %d", pHwCfg->thumb_mode);
    LOGD_ALDC("dth_bypass: %d", pHwCfg->dth_bypass);
    LOGD_ALDC("force_map_en: %d", pHwCfg->force_map_en);
    LOGD_ALDC("map13p3_en: %d", pHwCfg->map13p3_en);
    LOGD_ALDC("out_vsize: %d", pHwCfg->out_vsize);
    LOGD_ALDC("last_offset: %d", pHwCfg->last_offset);
    LOGD_ALDC("hsize: %d", pHwCfg->hsize);
    LOGD_ALDC("vsize: %d", pHwCfg->vsize);
    LOGD_ALDC("mesh buffer fd: %d", pHwCfg->buf_fd);
    LOGD_ALDC("bicubic[0]: %d", pHwCfg->bicubic[0]);
    LOGD_ALDC("bicubic[1]: %d", pHwCfg->bicubic[1]);
    LOGD_ALDC("bicubic[2]: %d", pHwCfg->bicubic[2]);
    LOGD_ALDC("bicubic[3]: %d", pHwCfg->bicubic[3]);
}
