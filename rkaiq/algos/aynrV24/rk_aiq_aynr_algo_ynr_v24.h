
#ifndef _RKAIQ_YNR_V24_H_
#define _RKAIQ_YNR_V24_H_

#include "RkAiqCalibApi.h"
#include "RkAiqCalibDbTypes.h"
#include "aynrV24/rk_aiq_types_aynr_algo_prvt_v24.h"
#include "base/xcam_log.h"
#include "math.h"
#include "rk_aiq_comm.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

XCAM_BEGIN_DECLARE

Aynr_result_V24_t ynr_select_params_by_ISO_V24(RK_YNR_Params_V24_t* pParams,
                                               RK_YNR_Params_V24_Select_t* pSelect,
                                               Aynr_ExpInfo_V24_t* pExpInfo);

Aynr_result_V24_t ynr_fix_transfer_V24(RK_YNR_Params_V24_Select_t* pSelect, RK_YNR_Fix_V24_t* pFix,
                                       rk_aiq_ynr_strength_v24_t* pStrength,
                                       Aynr_ExpInfo_V24_t* pExpInfo);

Aynr_result_V24_t ynr_fix_printf_V24(RK_YNR_Fix_V24_t* pFix, RK_YNR_Params_V24_Select_t* pSelect);

Aynr_result_V24_t ynr_get_setting_by_name_json_V24(CalibDbV2_YnrV24_t* pCalibdbV2, char* name,
                                                   int* calib_idx, int* tuning_idx);

Aynr_result_V24_t ynr_init_params_json_V24(RK_YNR_Params_V24_t* pYnrParams,
                                           CalibDbV2_YnrV24_t* pCalibdbV2, int calib_idx,
                                           int tuning_idx);

Aynr_result_V24_t ynr_config_setting_param_json_V24(RK_YNR_Params_V24_t* pParams,
                                                    CalibDbV2_YnrV24_t* pCalibdbV2,
                                                    char* param_mode, char* snr_name);

XCAM_END_DECLARE

#endif  // BAYERNR_READ_PARM_H_
