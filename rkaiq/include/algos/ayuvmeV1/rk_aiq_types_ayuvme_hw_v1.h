/*
 *rk_aiq_types_alsc_hw.h
 *
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

#ifndef _RK_AIQ_TYPE_AYUVME_HW_V1_H_
#define _RK_AIQ_TYPE_AYUVME_HW_V1_H_
#include "rk_aiq_comm.h"



typedef struct RK_YUVME_Fix_V1_s {
    /* CTRL */
    uint8_t bypass;
    uint8_t tnr_wgt0_en;
    /* YUVME_PARA0 */
    uint8_t global_nr_strg;
    uint8_t wgt_fact3;
    uint8_t search_range_mode;
    uint8_t static_detect_thred;
    /* YUVME_PARA1 */
    uint8_t time_relevance_offset;
    uint8_t space_relevance_offset;
    uint8_t nr_diff_scale;
    uint16_t nr_fusion_limit;
    /* YUVME_PARA2 */
    uint8_t nr_static_scale;
    uint16_t nr_motion_scale;
    uint8_t nr_fusion_mode;
    uint16_t cur_weight_limit;
    /* YUVME_SIGMA */
    uint16_t nr_luma2sigma_val[16];

} RK_YUVME_Fix_V1_t;


#endif



