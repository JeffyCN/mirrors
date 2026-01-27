/*
 *  Copyright (c) 2019 Rockchip Corporation
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

#ifndef _RK_AIQ_USER_API2_AWB_V3_H_
#define _RK_AIQ_USER_API2_AWB_V3_H_

#include "algos/rk_aiq_api_types_awb.h"
#include "algos/awb/rk_aiq_types_awb_algo_int.h"
#ifndef RK_AIQ_SYS_CTX_T
#define RK_AIQ_SYS_CTX_T
typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;
#endif

RKAIQ_BEGIN_DECLARE
XCamReturn
rk_aiq_user_api2_awb_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_api_attrib_t* attr);
XCamReturn
rk_aiq_user_api2_awb_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_api_attrib_t* attr);
XCamReturn
rk_aiq_user_api2_awb_SetWbGainCtrlAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCtrl_t* attr);
XCamReturn
rk_aiq_user_api2_awb_GetWbGainCtrlAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCtrl_t* attr);
XCamReturn
rk_aiq_user_api2_awb_SetAwbStatsAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_Stats_t* attr);
XCamReturn
rk_aiq_user_api2_awb_GetAwbStatsAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_Stats_t* attr);
XCamReturn
rk_aiq_user_api2_awb_SetAwbGnCalcStepAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcStep_t* attr);
XCamReturn
rk_aiq_user_api2_awb_GetAwbGnCalcStepAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcStep_t* attr);
XCamReturn
rk_aiq_user_api2_awb_SetAwbGnCalcOthAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcOth_t* attr);
XCamReturn
rk_aiq_user_api2_awb_GetAwbGnCalcOthAttrib(const rk_aiq_sys_ctx_t* sys_ctx, awb_gainCalcOth_t* attr);
XCamReturn
rk_aiq_user_api2_awb_setAwbPreWbgain(const rk_aiq_sys_ctx_t* sys_ctx,  const float att[4]);
XCamReturn
rk_aiq_user_api2_awb_IqMap2Main(const rk_aiq_sys_ctx_t* sys_ctx,  rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att);

XCamReturn
rk_aiq_user_api2_awb_QueryWBInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_querry_info_t *wb_querry_info);
XCamReturn
rk_aiq_user_api2_awb_Lock(const rk_aiq_sys_ctx_t* sys_ctx);
XCamReturn
rk_aiq_user_api2_awb_Unlock(const rk_aiq_sys_ctx_t* sys_ctx);
XCamReturn
rk_aiq_user_api2_awb_getStrategyResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_strategy_result_t *attr);
XCamReturn
rk_aiq_user_api2_awb_getAlgoSta(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res_full_t *attr);
XCamReturn
rk_aiq_user_api2_awb_WriteAwbIn(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_wrtIn_attr_t attr);
XCamReturn
rk_aiq_user_api2_awb_SetFFWbgainAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_ffwbgain_attr_t attr);
XCamReturn
rk_aiq_user_api2_awb_freeConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *gain_lut_cfg);
XCamReturn
rk_aiq_user_api2_awb_loadConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *gain_lut_cfg, char *fullfile);
XCamReturn
rk_aiq_user_api2_awb_wbgainConvert2(rk_aiq_wb_gain_t  wbGainIn, rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *gain_lut_cfg, rk_aiq_wb_gain_t  *wbGainOut);
XCamReturn
rk_aiq_user_api2_awb_SetNNres(const rk_aiq_sys_ctx_t* sys_ctx, awb_ai_res_t *attr);

RKAIQ_END_DECLARE

#endif
