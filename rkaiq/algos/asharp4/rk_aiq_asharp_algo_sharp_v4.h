#ifndef __RKAIQ_ASHARP_V4_H__
#define __RKAIQ_ASHARP_V4_H__

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "rk_aiq_types_asharp_algo_prvt_v4.h"

RKAIQ_BEGIN_DECLARE

Asharp4_result_t sharp_select_params_by_ISO_V4(RK_SHARP_Params_V4_t *pParams, RK_SHARP_Params_V4_Select_t *pSelect, Asharp4_ExpInfo_t *pExpInfo);

Asharp4_result_t sharp_fix_transfer_V4(RK_SHARP_Params_V4_Select_t *pSelect, RK_SHARP_Fix_V4_t* pFix, rk_aiq_sharp_strength_v4_t *pStrength);

Asharp4_result_t sharp_fix_printf_V4(RK_SHARP_Fix_V4_t  * pFix);

Asharp4_result_t sharp_get_setting_by_name_json_V4(CalibDbV2_SharpV4_t *pCalibdbV2, char *name, int *tuning_idx);

Asharp4_result_t sharp_init_params_json_V4(RK_SHARP_Params_V4_t *pSharpParams, CalibDbV2_SharpV4_t *pCalibdbV2, int tuning_idx);

Asharp4_result_t sharp_config_setting_param_json_V4(RK_SHARP_Params_V4_t *pParams, CalibDbV2_SharpV4_t *pCalibdbV2, char* param_mode, char * snr_name);


RKAIQ_END_DECLARE



#endif
