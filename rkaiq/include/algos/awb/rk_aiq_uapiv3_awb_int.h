#ifndef _RK_AIQ_UAPIV3_AWB_INT_H_
#define _RK_AIQ_UAPIV3_AWB_INT_H_

#include "xcore/base/xcam_common.h"
#include "algos/rk_aiq_algo_des.h"
#include "algos/awb/rk_aiq_types_awb_algo_int.h"

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAwbHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.
RKAIQ_BEGIN_DECLARE

XCamReturn
rk_aiq_uapiV3_awb_SetAttrib(RkAiqAlgoContext *ctx,
                               awb_api_attrib_t *attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_GetAttrib(RkAiqAlgoContext *ctx,
                               awb_api_attrib_t *attr);
XCamReturn
rk_aiq_uapiV3_awb_SetWbGainCtrlAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCtrl_t *attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_GetWbGainCtrlAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCtrl_t *attr);
XCamReturn
rk_aiq_uapiV3_awb_SetAwbStatsAttrib(RkAiqAlgoContext *ctx,
                               awb_Stats_t *attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_GetAwbStatsAttrib(RkAiqAlgoContext *ctx,
                               awb_Stats_t *attr);
XCamReturn
rk_aiq_uapiV3_awb_SetAwbGnCalcStepAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCalcStep_t *attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_GetAwbGnCalcStepAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCalcStep_t *attr);
XCamReturn
rk_aiq_uapiV3_awb_SetAwbGnCalcOthAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCalcOth_t *attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_GetAwbGnCalcOthAttrib(RkAiqAlgoContext *ctx,
                               awb_gainCalcOth_t *attr);
XCamReturn
rk_aiq_uapiV3_awb_QueryWBInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_wb_querry_info_t *wb_querry_info );
XCamReturn
rk_aiq_uapiV3_awb_Lock(RkAiqAlgoContext *ctx);

XCamReturn
rk_aiq_uapiV3_awb_Unlock(const RkAiqAlgoContext *ctx);

XCamReturn
rk_aiq_uapiV3_awb_GetAlgoStat(const RkAiqAlgoContext *ctx, rk_tool_awb_stat_res_full_t *awb_stat_algo);

XCamReturn
rk_aiq_uapiV3_awb_GetStrategyResult(const RkAiqAlgoContext *ctx, rk_tool_awb_strategy_result_t *awb_strategy_result);
XCamReturn
rk_aiq_uapiV3_awb_WriteInput(RkAiqAlgoContext *ctx,
                             rk_aiq_uapiV2_awb_wrtIn_attr_t attr,
                             bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_SetFstFrWbgain(RkAiqAlgoContext *ctx,
                                 rk_aiq_wb_gain_t wbgain,
                                 bool need_sync);

XCamReturn
rk_aiq_uapiV3_awb_IqMap2Main(RkAiqAlgoContext *ctx, rk_aiq_uapiV2_awb_Slave2Main_Cfg_t slave2Main, bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_SetPreWbgain(RkAiqAlgoContext *ctx,
                               const float  preWbgain[4],
                               bool need_sync);
XCamReturn
rk_aiq_uapiV3_awb_SetNNres(RkAiqAlgoContext *ctx,
                               const awb_ai_res_t *attr,
                               bool need_sync);
RKAIQ_END_DECLARE
#endif

