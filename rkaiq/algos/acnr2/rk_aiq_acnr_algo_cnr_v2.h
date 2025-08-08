#ifndef __RKAIQ_CNR_V2_H_
#define __RKAIQ_CNR_V2_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "acnr2/rk_aiq_types_acnr_algo_prvt_v2.h"

RKAIQ_BEGIN_DECLARE

AcnrV2_result_t cnr_select_params_by_ISO_V2(RK_CNR_Params_V2_t *pParams, RK_CNR_Params_V2_Select_t *pSelect, AcnrV2_ExpInfo_t *pExpInfo);

AcnrV2_result_t cnr_fix_transfer_V2(RK_CNR_Params_V2_Select_t *pSelect, RK_CNR_Fix_V2_t *pFix, AcnrV2_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v2_t *pStrength);

AcnrV2_result_t cnr_fix_printf_V2(RK_CNR_Fix_V2_t  * pFix);

AcnrV2_result_t cnr_get_setting_by_name_json_V2(CalibDbV2_CNRV2_t *pCalibdbV2, char *name, int *tuning_idx);

AcnrV2_result_t cnr_init_params_json_V2(RK_CNR_Params_V2_t *pParams, CalibDbV2_CNRV2_t *pCalibdbV2, int tuning_idx);

AcnrV2_result_t cnr_config_setting_param_json_V2(RK_CNR_Params_V2_t *pParams, CalibDbV2_CNRV2_t *pCalibdbV2, char* param_mode, char * snr_name);



RKAIQ_END_DECLARE





#endif

