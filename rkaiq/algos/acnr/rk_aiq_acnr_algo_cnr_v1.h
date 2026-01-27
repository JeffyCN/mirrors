#ifndef __RKAIQ_CNR_V1_H_
#define __RKAIQ_CNR_V1_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "acnr/rk_aiq_types_acnr_algo_prvt_v1.h"

RKAIQ_BEGIN_DECLARE

Acnr_result_t cnr_get_mode_by_name_V1(struct list_head* pCalibdbList, char *name, Calibdb_Cnr_V1_t** ppProfile);

Acnr_result_t cnr_get_setting_by_name_V1(struct list_head *pSettingList, char *name, Calibdb_Cnr_params_V1_t** ppSetting);

Acnr_result_t cnr_config_setting_param_V1(RK_CNR_Params_V1_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name);

Acnr_result_t cnr_init_params_V1(RK_CNR_Params_V1_t *pParams, Calibdb_Cnr_params_V1_t* pCalibParms);

Acnr_result_t cnr_select_params_by_ISO_V1(RK_CNR_Params_V1_t *pParams, RK_CNR_Params_V1_Select_t *pSelect, Acnr_ExpInfo_t *pExpInfo);

Acnr_result_t cnr_fix_transfer_V1(RK_CNR_Params_V1_Select_t *pSelect, RK_CNR_Fix_V1_t *pFix, Acnr_ExpInfo_t *pExpInfo, float fStrength);

Acnr_result_t cnr_fix_printf_V1(RK_CNR_Fix_V1_t  * pFix);

Acnr_result_t cnr_get_setting_by_name_json_V1(CalibDbV2_CNR_t *pCalibdbV2, char *name, int *tuning_idx);

Acnr_result_t cnr_init_params_json_V1(RK_CNR_Params_V1_t *pParams, CalibDbV2_CNR_t *pCalibdbV2, int tuning_idx);

Acnr_result_t cnr_config_setting_param_json_V1(RK_CNR_Params_V1_t *pParams, CalibDbV2_CNR_t *pCalibdbV2, char* param_mode, char * snr_name);



RKAIQ_END_DECLARE





#endif

