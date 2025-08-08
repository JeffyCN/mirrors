/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _RK_AIQ_USER_API_CUSTOM_AWB_TYPE_V39_H_
#define _RK_AIQ_USER_API_CUSTOM_AWB_TYPE_V39_H_

RKAIQ_BEGIN_DECLARE

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
typedef struct rk_aiq_camgroup_ctx_s rk_aiq_camgroup_ctx_t;



/* different awb result for each camera */
typedef struct rk_aiq_customeAwb2_single_results_s
{
    rk_aiq_wb_gain_t awb_gain_algo;//for each camera
    struct rk_aiq_customeAwb2_single_results_s *next;
} rk_aiq_customeAwb2_single_results_t;

/* full awb results */
typedef struct rk_aiq_customeAwb2_results_s
{
    bool  IsConverged; //true: converged; false: not converged
    rk_aiq_wb_gain_t awb_gain_algo;
    float awb_smooth_factor;
    awbStats_cfg_t  awbHwConfig;
    rk_aiq_customeAwb2_single_results_t *next;//defalut vaue is nullptr,which means all cameras with the same cfg;
} rk_aiq_customeAwb2_results_t;

typedef struct rk_aiq_customAwb2_stats_s
{
    awbStats_stats_t stats;
    struct rk_aiq_customAwb2_stats_s* next;
} rk_aiq_customAwb2_stats_t;

typedef struct rk_aiq_customeAwb_cbs_s
{
    /* ctx is the rk_aiq_sys_ctx_t which is corresponded to
     * camera, could be mapped to camera id.
     */
    int32_t (*pfn_awb_init)(void* ctx);
    int32_t (*pfn_awb_run)(void* ctx, const void* pstAwbInfo,
                          void* pstAwbResult);
    /* not used now */
    int32_t (*pfn_awb_ctrl)(void* ctx, uint32_t u32Cmd, void *pValue);
    int32_t (*pfn_awb_exit)(void* ctx);
} rk_aiq_customeAwb_cbs_t;


typedef struct _RkAiqAwbAlgoContext2 {
    rk_aiq_customeAwb_cbs_t cbs; // set by register
    union {
        rk_aiq_sys_ctx_t* aiq_ctx;  // set by register
        rk_aiq_camgroup_ctx_t* group_ctx;  // set by register
    };
    rk_aiq_customeAwb2_results_t customRes; // result of pfn_awb_run
    bool cutomAwbInit;
    bool updateCalib;
    int camIdArray[6];
    int camIdArrayLen;
    bool isGroupMode;
    bool isRkCb;
    int camId;
} RkAiqAwbAlgoContext2;



RKAIQ_END_DECLARE

#endif

