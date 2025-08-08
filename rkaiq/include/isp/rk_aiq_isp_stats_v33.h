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

#ifndef _RK_AIQ_PARAM_ISP_STATS_v33_H_
#define _RK_AIQ_PARAM_ISP_STATS_v33_H_

#include "isp/rk_aiq_stats_awb33.h"
#include "isp/rk_aiq_stats_aeStats25.h"

typedef awbStats_stats_priv_t rk_aiq_isp_stats_awb_t;
typedef RKAiqAecStatsV25_t    rk_aiq_isp_stats_ae_t;

typedef struct rk_aiq_isp_statistics_s {
    uint32_t frame_id;
    rk_aiq_isp_stats_ae_t aec_stats;
    bool bValid_aec_stats;
    rk_aiq_isp_stats_awb_t awb_stats;
    bool bValid_awb_stats;
} rk_aiq_isp_statistics_t;

#endif
