
#ifndef _RKAIQ_YNR_V3_H_
#define _RKAIQ_YNR_V3_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "aynr3/rk_aiq_types_aynr_algo_prvt_v3.h"

XCAM_BEGIN_DECLARE

Aynr_result_V3_t ynr_select_params_by_ISO_V3(RK_YNR_Params_V3_t *pParams, RK_YNR_Params_V3_Select_t *pSelect, Aynr_ExpInfo_V3_t *pExpInfo);

Aynr_result_V3_t ynr_fix_transfer_V3(RK_YNR_Params_V3_Select_t* pSelect, RK_YNR_Fix_V3_t *pFix, rk_aiq_ynr_strength_v3_t *pStrength, Aynr_ExpInfo_V3_t *pExpInfo);

Aynr_result_V3_t ynr_fix_printf_V3(RK_YNR_Fix_V3_t * pFix);

Aynr_result_V3_t ynr_get_setting_by_name_json_V3(CalibDbV2_YnrV3_t *pCalibdbV2, char *name, int* calib_idx, int* tuning_idx);

Aynr_result_V3_t ynr_init_params_json_V3(RK_YNR_Params_V3_t *pYnrParams, CalibDbV2_YnrV3_t *pCalibdbV2, int calib_idx, int tuning_idx);

Aynr_result_V3_t ynr_config_setting_param_json_V3(RK_YNR_Params_V3_t *pParams, CalibDbV2_YnrV3_t *pCalibdbV2, char* param_mode, char * snr_name);





XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

