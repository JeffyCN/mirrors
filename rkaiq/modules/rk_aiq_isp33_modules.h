#ifndef _RK_AIQ_ISP33_MODULES_H_
#define _RK_AIQ_ISP33_MODULES_H_

#include "rk_aiq_isp32_modules.h"
#include "rk_aiq_module_btnr_common.h"
#include "rk_aiq_module_common.h"
#include "common/rk-isp33-config.h"

RKAIQ_BEGIN_DECLARE

void rk_aiq_dm24_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_histeq30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_enh30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
void rk_aiq_gic30_params_cvt(void* attr, struct isp33_gic_cfg* gic_cfg, btnr_cvt_info_t* pBtnrInfo);
void rk_aiq_cac30_params_cvt(void* attr, struct isp33_cac_cfg* isp_cfg,
                             struct isp33_cac_cfg* isp_cfg_right, bool is_multi_isp, common_cvt_info_t* cvtinfo);
void rk_aiq_btnr41_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo, mergeLuma2Wgt_t* pMergeLuma2Wgt);
void rk_aiq_sharp40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, btnr_cvt_info_t* pBtnrInfo);
void rk_aiq_ynr40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_hsv10_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
RKAIQ_END_DECLARE

#endif

