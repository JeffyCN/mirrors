
#ifndef _RKAIQ_BAYERTNR_V2_H_
#define _RKAIQ_BAYERTNR_V2_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "abayertnr2/rk_aiq_types_abayertnr_algo_prvt_v2.h"

XCAM_BEGIN_DECLARE


Abayertnr_result_V2_t bayertnr_select_params_by_ISO_V2(RK_Bayertnr_Params_V2_t *pParams, RK_Bayertnr_Params_V2_Select_t *pSelect, Abayertnr_ExpInfo_V2_t *pExpInfo);

Abayertnr_result_V2_t bayertnr_fix_transfer_V2(RK_Bayertnr_Params_V2_Select_t* pSelect, RK_Bayertnr_Fix_V2_t *pFix, rk_aiq_bayertnr_strength_v2_t *pStrength, Abayertnr_ExpInfo_V2_t *pExpInfo);

Abayertnr_result_V2_t bayertnr_fix_printf_V2(RK_Bayertnr_Fix_V2_t * pFix);

Abayertnr_result_V2_t bayertnr_get_setting_by_name_json_V2(CalibDbV2_BayerTnrV2_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Abayertnr_result_V2_t bayertnr_init_params_json_V2(RK_Bayertnr_Params_V2_t *pParams, CalibDbV2_BayerTnrV2_t* pCalibdb, int calib_idx, int tuning_idx);

Abayertnr_result_V2_t bayertnr_config_setting_param_json_V2(RK_Bayertnr_Params_V2_t *pParams, CalibDbV2_BayerTnrV2_t* pCalibdbV2, char* param_mode, char * snr_name);


XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

