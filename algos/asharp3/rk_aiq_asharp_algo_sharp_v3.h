#ifndef __RKAIQ_ASHARP_V3_H__
#define __RKAIQ_ASHARP_V3_H__

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "rk_aiq_types_asharp_algo_prvt_v3.h"

RKAIQ_BEGIN_DECLARE

Asharp3_result_t sharp_get_mode_by_name_V3(struct list_head* pCalibdbList, char *name, Calibdb_Sharp_V3_t** ppProfile);

Asharp3_result_t sharp_get_setting_by_name_V3(struct list_head *pSettingList, char *name, Calibdb_Sharp_params_V3_t** ppSetting);

Asharp3_result_t sharp_config_setting_param_V3(RK_SHARP_Params_V3_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name);

Asharp3_result_t sharp_init_params_V3(RK_SHARP_Params_V3_t *pSharpParams, Calibdb_Sharp_params_V3_t* pCalibParms);

Asharp3_result_t sharp_select_params_by_ISO_V3(RK_SHARP_Params_V3_t *pParams, RK_SHARP_Params_V3_Select_t *pSelect, Asharp3_ExpInfo_t *pExpInfo);

Asharp3_result_t sharp_fix_transfer_V3(RK_SHARP_Params_V3_Select_t *pSelect, RK_SHARP_Fix_V3_t* pFix, float fPercent);

Asharp3_result_t sharp_fix_printf_V3(RK_SHARP_Fix_V3_t  * pFix);

Asharp3_result_t sharp_get_setting_by_name_json_V3(CalibDbV2_SharpV3_t *pCalibdbV2, char *name, int *tuning_idx);

Asharp3_result_t sharp_init_params_json_V3(RK_SHARP_Params_V3_t *pSharpParams, CalibDbV2_SharpV3_t *pCalibdbV2, int tuning_idx);

Asharp3_result_t sharp_config_setting_param_json_V3(RK_SHARP_Params_V3_t *pParams, CalibDbV2_SharpV3_t *pCalibdbV2, char* param_mode, char * snr_name);


RKAIQ_END_DECLARE



#endif
