
#ifndef _RKAIQ_YNR_V22_H_
#define _RKAIQ_YNR_V22_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "aynrV22/rk_aiq_types_aynr_algo_prvt_v22.h"

XCAM_BEGIN_DECLARE

Aynr_result_V22_t ynr_select_params_by_ISO_V22(RK_YNR_Params_V22_t *pParams, RK_YNR_Params_V22_Select_t *pSelect, Aynr_ExpInfo_V22_t *pExpInfo);

Aynr_result_V22_t ynr_fix_transfer_V22(RK_YNR_Params_V22_Select_t* pSelect, RK_YNR_Fix_V22_t *pFix, rk_aiq_ynr_strength_v22_t* pStrength, Aynr_ExpInfo_V22_t *pExpInfo);

Aynr_result_V22_t ynr_fix_printf_V22(RK_YNR_Fix_V22_t * pFix);

Aynr_result_V22_t ynr_get_setting_by_name_json_V22(CalibDbV2_YnrV22_t *pCalibdbV2, char *name, int* calib_idx, int* tuning_idx);

Aynr_result_V22_t ynr_init_params_json_V22(RK_YNR_Params_V22_t *pYnrParams, CalibDbV2_YnrV22_t *pCalibdbV2, int calib_idx, int tuning_idx);

Aynr_result_V22_t ynr_config_setting_param_json_V22(RK_YNR_Params_V22_t *pParams, CalibDbV2_YnrV22_t *pCalibdbV2, char* param_mode, char * snr_name);





XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

