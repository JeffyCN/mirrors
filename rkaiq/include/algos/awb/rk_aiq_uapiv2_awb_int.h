#ifndef _RK_AIQ_UAPIV2_AWB_INT_H_
#define _RK_AIQ_UAPIV2_AWB_INT_H_

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
rk_aiq_uapiV2_awbV20_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV20_attrib_t attr,
                               bool need_sync);

XCamReturn
rk_aiq_uapiV2_awbV20_GetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV20_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awbV21_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV21_attrib_t attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV21_GetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV21_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awb_SetMwbMode(RkAiqAlgoContext *ctx,
                             rk_aiq_wb_op_mode_t mode,
                             bool need_sync);

XCamReturn
rk_aiq_uapiV2_awb_GetMwbMode(RkAiqAlgoContext *ctx,
                             rk_aiq_wb_op_mode_t *mode);

XCamReturn
rk_aiq_uapiV2_awb_SetMwbAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_wb_mwb_attrib_t attr,
                               bool need_sync);

XCamReturn
rk_aiq_uapiV2_awb_GetMwbAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_wb_mwb_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awbV20_SetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV20_awb_attrib_t attr,
                                  bool need_sync);

XCamReturn
rk_aiq_uapiV2_awbV20_GetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV20_awb_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awbV21_SetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV21_awb_attrib_t attr,
                                  bool need_sync);

XCamReturn
rk_aiq_uapiV2_awbV21_GetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV21_awb_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awb_SetAwbGainAdjust(RkAiqAlgoContext *ctx,
                                   rk_aiq_uapiV2_wb_awb_wbGainAdjust_t wbGainAdjust,
                                   bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_GetAwbGainAdjust(RkAiqAlgoContext *ctx,
                                   rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *wbGainAdjust);

XCamReturn
rk_aiq_uapiV2_awb_SetAwbGainOffset(RkAiqAlgoContext *ctx,
                                   CalibDbV2_Awb_gain_offset_cfg_t wbGainOffset,
                                   bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_GetAwbGainOffset(RkAiqAlgoContext *ctx,
                                   CalibDbV2_Awb_gain_offset_cfg_t *wbGainOffset);
XCamReturn
rk_aiq_uapiV2_awb_GetCCT(const RkAiqAlgoContext *ctx,
                         rk_aiq_wb_cct_t *cct );

XCamReturn
rk_aiq_uapiV2_awb_SetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                    CalibDbV2_Awb_Mul_Win_t  multiWindow,
                                    bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_GetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                    CalibDbV2_Awb_Mul_Win_t  *multiWindow);

XCamReturn
rk_aiq_uapiV2_awb_QueryWBInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_wb_querry_info_t *wb_querry_info );
XCamReturn
rk_aiq_uapiV2_awb_Lock(RkAiqAlgoContext *ctx);

XCamReturn
rk_aiq_uapiV2_awb_Unlock(const RkAiqAlgoContext *ctx);

XCamReturn
rk_aiq_uapiV2_awb_GetAlgoStat(const RkAiqAlgoContext *ctx, rk_tool_awb_stat_res_full_t *awb_stat_algo);

XCamReturn
rk_aiq_uapiV2_awb_GetStrategyResult(const RkAiqAlgoContext *ctx, rk_tool_awb_strategy_result_t *awb_strategy_result);
XCamReturn
rk_aiq_uapiV2_awbV32_SetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV32_awb_mulWindow_t  multiWindow,
                                       bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV32_GetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV32_awb_mulWindow_t  *multiWindow);
XCamReturn
rk_aiq_uapiV2_awbV32_SetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV32_awb_attrib_t attr,
                                  bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV32_GetAwbAttrib(RkAiqAlgoContext *ctx,
                                  rk_aiq_uapiV2_wbV32_awb_attrib_t *attr);

XCamReturn
rk_aiq_uapiV2_awbV32_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV32_attrib_t attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV32_GetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV32_attrib_t *attr);
XCamReturn
rk_aiq_uapiV2_awbV32_GetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV32_awb_mulWindow_t  *multiWindow);
XCamReturn
rk_aiq_uapiV2_awbV32_SetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV32_awb_mulWindow_t  multiWindow,
                                       bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_WriteInput(RkAiqAlgoContext *ctx,
                             rk_aiq_uapiV2_awb_wrtIn_attr_t attr,
                             bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_SetFstFrWbgain(RkAiqAlgoContext *ctx,
                                 rk_aiq_wb_gain_t wbgain,
                                 bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_GetIQAutoExtPara(RkAiqAlgoContext *ctx,
                                   rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t *autoExtPara);
XCamReturn
rk_aiq_uapiV2_awb_SetIQAutoExtPara(RkAiqAlgoContext *ctx,
                                   const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t *autoExtPara,
                                   bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_GetIQAutoPara(RkAiqAlgoContext *ctx,
                                rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t *autoPara);
XCamReturn
rk_aiq_uapiV2_awb_SetIQAutoPara(RkAiqAlgoContext *ctx,
                                const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t *autoPara,
                                bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_IqMap2Main(RkAiqAlgoContext *ctx, rk_aiq_uapiV2_awb_Slave2Main_Cfg_t slave2Main, bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_SetPreWbgain(RkAiqAlgoContext *ctx,
                               const float  preWbgain[4],
                               bool need_sync);
XCamReturn
rk_aiq_uapiV2_awb_freeConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg);
XCamReturn
rk_aiq_uapiV2_awb_loadConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg, char *fullfile);
XCamReturn
rk_aiq_uapiV2_awb_wbgainConvert2(rk_aiq_wb_gain_t  wbGainIn, rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg, rk_aiq_wb_gain_t  *wbGainOut);
XCamReturn
rk_aiq_uapiV2_awbV39_SetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV39_awb_nonROI_t  attr,
                                       bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV39_GetAwbMultiwindow(RkAiqAlgoContext *ctx,
                                       rk_aiq_uapiV2_wbV39_awb_nonROI_t  *attr);
XCamReturn
rk_aiq_uapiV2_awbV39_SetAwbAttrib1(RkAiqAlgoContext *ctx,
                                   rk_aiq_uapiV2_wbV39_awb_attrib_t attr,
                                   bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV39_GetAwbAttrib1(RkAiqAlgoContext *ctx,
                                   rk_aiq_uapiV2_wbV39_awb_attrib_t *attr);
XCamReturn
rk_aiq_uapiV2_awbV39_SetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV39_attrib_t attr,
                               bool need_sync);
XCamReturn
rk_aiq_uapiV2_awbV39_GetAttrib(RkAiqAlgoContext *ctx,
                               rk_aiq_uapiV2_wbV39_attrib_t *attr);
RKAIQ_END_DECLARE
#endif

