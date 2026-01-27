
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
#include "abayer2dnr2/rk_aiq_types_abayer2dnr_algo_prvt_v2.h"

XCAM_BEGIN_DECLARE

Abayer2dnr_result_V2_t bayer2dnr_get_mode_by_name_V2(struct list_head *pCalibdbList, char *name, CalibDb_Bayernr_V2_t** ppProfile);

Abayer2dnr_result_V2_t bayer2dnr_get_setting_by_name_V2(struct list_head *pSettingList, char *name, Calibdb_Bayernr_2Dparams_V2_t** ppSetting);

Abayer2dnr_result_V2_t bayer2dnr_config_setting_param_V2(RK_Bayer2dnr_Params_V2_t *pParams, struct list_head *pCalibdbList, char* param_mode, char * snr_name);

Abayer2dnr_result_V2_t bayer2dnr_init_params_V2(RK_Bayer2dnr_Params_V2_t *pParams, Calibdb_Bayernr_2Dparams_V2_t* pCalibdbParams);

Abayer2dnr_result_V2_t bayer2dnr_select_params_by_ISO_V2(RK_Bayer2dnr_Params_V2_t *pParams, RK_Bayer2dnr_Params_V2_Select_t *pSelect, Abayer2dnr_ExpInfo_V2_t *pExpInfo);

Abayer2dnr_result_V2_t bayer2dnr_fix_transfer_V2(RK_Bayer2dnr_Params_V2_Select_t* pSelect, RK_Bayer2dnr_Fix_V2_t *pFix, rk_aiq_bayer2dnr_strength_v2_t *pStrength, Abayer2dnr_ExpInfo_V2_t *pExpInfo);

Abayer2dnr_result_V2_t bayer2dnr_fix_printf_V2(RK_Bayer2dnr_Fix_V2_t * pFix);

Abayer2dnr_result_V2_t bayer2dnr_get_setting_by_name_json_V2(CalibDbV2_Bayer2dnrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Abayer2dnr_result_V2_t bayer2dnr_init_params_json_V2(RK_Bayer2dnr_Params_V2_t *pParams, CalibDbV2_Bayer2dnrV2_t* pCalibdb, int calib_idx, int tuning_idx);

Abayer2dnr_result_V2_t bayer2dnr_config_setting_param_json_V2(RK_Bayer2dnr_Params_V2_t *pParams, CalibDbV2_Bayer2dnrV2_t* pCalibdbV2, char* param_mode, char * snr_name);


XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

