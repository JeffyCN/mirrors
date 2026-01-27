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

#ifndef _AIQ_ALGO_CAMGROUP_AWB_HANDLE_H_
#define _AIQ_ALGO_CAMGROUP_AWB_HANDLE_H_

#include "aiq_algo_camgroup_handler.h"

RKAIQ_BEGIN_DECLARE

typedef AiqAlgoCamGroupHandler_t AiqAlgoCamGroupAwbHandler_t;

XCamReturn AiqAlgoCamGroupAwbHandler_setAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_api_attrib_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_api_attrib_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbGainCtrlAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCtrl_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbGainCtrlAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCtrl_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setAwbStatsAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_Stats_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAwbStatsAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_Stats_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setAwbGnCalcStepAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcStep_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAwbGnCalcStepAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcStep_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setAwbGnCalcOthAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcOth_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAwbGnCalcOthAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, awb_gainCalcOth_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_writeAwbIn(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_awb_wrtIn_attr_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_awbIqMap2Main(AiqAlgoCamGroupAwbHandler_t* pAwbHdl,  rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_setAwbPreWbgain(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const float att[4]);
XCamReturn AiqAlgoCamGroupAwbHandler_queryWBInfo(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_querry_info_t* wb_querry_info);
XCamReturn AiqAlgoCamGroupAwbHandler_lock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl);
XCamReturn AiqAlgoCamGroupAwbHandler_unlock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl);
XCamReturn AiqAlgoCamGroupAwbHandler_setFFWbgainAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_awb_ffwbgain_attr_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAlgoStat(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo);
XCamReturn AiqAlgoCamGroupAwbHandler_getStrategyResult(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result);


RKAIQ_END_DECLARE

#endif
