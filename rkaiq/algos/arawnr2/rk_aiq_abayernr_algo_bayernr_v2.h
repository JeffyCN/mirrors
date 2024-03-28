
#ifndef _RKAIQ_BAYERNR_V2_H_
#define _RKAIQ_BAYERNR_V2_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "arawnr2/rk_aiq_types_abayernr_algo_prvt_v2.h"

XCAM_BEGIN_DECLARE

Abayernr_result_t bayernr_get_mode_by_name_V2(struct list_head *pCalibdbList, char *name, CalibDb_Bayernr_V2_t** ppProfile);

Abayernr_result_t bayernr2D_get_setting_by_name_V2(struct list_head *pSettingList, char *name, Calibdb_Bayernr_2Dparams_V2_t** ppSetting);

Abayernr_result_t bayernr3D_get_setting_by_name_V2(struct list_head *pSettingList, char *name, CalibDb_Bayernr_3DParams_V2_t** ppSetting);

Abayernr_result_t bayernr2D_config_setting_param_V2(RK_Bayernr_2D_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name);

Abayernr_result_t bayernr3D_config_setting_param_V2(RK_Bayernr_3D_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name);

Abayernr_result_t bayernr2D_init_params_V2(RK_Bayernr_2D_Params_V2_t *pParams, Calibdb_Bayernr_2Dparams_V2_t* pCalibdbParams);

Abayernr_result_t bayernr3D_init_params_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDb_Bayernr_3DParams_V2_t* pCalibdbParams);

Abayernr_result_t bayernr2D_select_params_by_ISO_V2(RK_Bayernr_2D_Params_V2_t *pParams, RK_Bayernr_2D_Params_V2_Select_t *pSelect, Abayernr_ExpInfo_t *pExpInfo);

Abayernr_result_t bayernr3D_select_params_by_ISO_V2(RK_Bayernr_3D_Params_V2_t *pParams, RK_Bayernr_3D_Params_V2_Select_t *pSelect, Abayernr_ExpInfo_t *pExpInfo);

Abayernr_result_t bayernr2D_fix_transfer_V2(RK_Bayernr_2D_Params_V2_Select_t* pSelect, RK_Bayernr_2D_Fix_V2_t *pFix, float fStrength, Abayernr_ExpInfo_t *pExpInfo);

Abayernr_result_t bayernr3D_fix_transfer_V2(RK_Bayernr_3D_Params_V2_Select_t* pSelect, RK_Bayernr_3D_Fix_V2_t *pFix, float fStrength, Abayernr_ExpInfo_t *pExpInfo);

Abayernr_result_t bayernr2D_fix_printf_V2(RK_Bayernr_2D_Fix_V2_t * pFix);

Abayernr_result_t bayernr3D_fix_printf_V2(RK_Bayernr_3D_Fix_V2_t * pFix);

Abayernr_result_t bayernr2D_get_setting_by_name_json_V2(CalibDbV2_BayerNrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Abayernr_result_t bayernr2D_init_params_json_V2(RK_Bayernr_2D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdb, int calib_idx, int tuning_idx);

Abayernr_result_t bayernr2D_config_setting_param_json_V2(RK_Bayernr_2D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdbV2, char* param_mode, char * snr_name);

Abayernr_result_t bayernr3D_get_setting_by_name_json_V2(CalibDbV2_BayerNrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Abayernr_result_t bayernr3D_init_params_json_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdb, int calib_idx, int tuning_idx);

Abayernr_result_t bayernr3D_config_setting_param_json_V2(RK_Bayernr_3D_Params_V2_t *pParams, CalibDbV2_BayerNrV2_t* pCalibdbV2, char* param_mode, char * snr_name);


XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

