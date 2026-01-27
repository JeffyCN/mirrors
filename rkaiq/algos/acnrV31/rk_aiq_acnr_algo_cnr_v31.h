#ifndef __RKAIQ_CNR_V31_H_
#define __RKAIQ_CNR_V31_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "acnrV31/rk_aiq_types_acnr_algo_prvt_v31.h"

RKAIQ_BEGIN_DECLARE

AcnrV31_result_t cnr_select_params_by_ISO_V31(RK_CNR_Params_V31_t *pParams, RK_CNR_Params_V31_Select_t *pSelect, AcnrV31_ExpInfo_t *pExpInfo);

AcnrV31_result_t cnr_fix_transfer_V31(RK_CNR_Params_V31_Select_t *pSelect, RK_CNR_Fix_V31_t *pFix, AcnrV31_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v31_t* pStrength);

AcnrV31_result_t cnr_fix_printf_V31(RK_CNR_Fix_V31_t * pFix);

AcnrV31_result_t cnr_get_setting_by_name_json_V31(CalibDbV2_CNRV31_t* pCalibdbV2, char* name,
        int* tuning_idx);

AcnrV31_result_t cnr_init_params_json_V31(RK_CNR_Params_V31_t *pParams, CalibDbV2_CNRV31_t *pCalibdbV31, int tuning_idx);

AcnrV31_result_t cnr_config_setting_param_json_V31(RK_CNR_Params_V31_t *pParams, CalibDbV2_CNRV31_t *pCalibdbV31, char* param_mode, char * snr_name);



RKAIQ_END_DECLARE





#endif

