
#ifndef _RKAIQ_BAYERNR_V23_H_
#define _RKAIQ_BAYERNR_V23_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "abayer2dnrV23/rk_aiq_types_abayer2dnr_algo_prvt_v23.h"

XCAM_BEGIN_DECLARE

Abayer2dnr_result_V23_t bayer2dnr_select_params_by_ISO_V23(RK_Bayer2dnr_Params_V23_t *pParams, RK_Bayer2dnrV23_Params_Select_t *pSelect, Abayer2dnr_ExpInfo_V23_t *pExpInfo);

Abayer2dnr_result_V23_t bayer2dnr_fix_transfer_V23(RK_Bayer2dnrV23_Params_Select_t* pSelect, RK_Bayer2dnr_Fix_V23_t *pFix, rk_aiq_bayer2dnr_strength_v23_t* pStrength, Abayer2dnr_ExpInfo_V23_t *pExpInfo);

Abayer2dnr_result_V23_t bayer2dnr_fix_printf_V23(RK_Bayer2dnr_Fix_V23_t * pFix);

Abayer2dnr_result_V23_t bayer2dnr_get_setting_by_name_json_V23(CalibDbV2_Bayer2dnrV23_t* pCalibdb, char *name, int *calib_idx, int *tuning_idx);

Abayer2dnr_result_V23_t bayer2dnr_init_params_json_V23(RK_Bayer2dnr_Params_V23_t *pParams, CalibDbV2_Bayer2dnrV23_t* pCalibdb, int calib_idx, int tuning_idx);

Abayer2dnr_result_V23_t bayer2dnr_config_setting_param_json_V23(RK_Bayer2dnr_Params_V23_t *pParams, CalibDbV2_Bayer2dnrV23_t* pCalibdbV23, char* param_mode, char * snr_name);


XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

