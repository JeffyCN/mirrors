#ifndef __RKAIQ_CNR_V30_H_
#define __RKAIQ_CNR_V30_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "acnrV30/rk_aiq_types_acnr_algo_prvt_v30.h"

RKAIQ_BEGIN_DECLARE

AcnrV30_result_t cnr_select_params_by_ISO_V30(RK_CNR_Params_V30_t *pParams, RK_CNR_Params_V30_Select_t *pSelect, AcnrV30_ExpInfo_t *pExpInfo);

AcnrV30_result_t cnr_fix_transfer_V30(RK_CNR_Params_V30_Select_t *pSelect, RK_CNR_Fix_V30_t *pFix, AcnrV30_ExpInfo_t *pExpInfo, rk_aiq_cnr_strength_v30_t* pStrength);

AcnrV30_result_t cnr_fix_printf_V30(RK_CNR_Fix_V30_t * pFix);

AcnrV30_result_t cnr_get_setting_by_name_json_V30(CalibDbV2_CNRV30_t* pCalibdbV2, char* name,
        int* tuning_idx);

AcnrV30_result_t cnr_init_params_json_V30(RK_CNR_Params_V30_t *pParams, CalibDbV2_CNRV30_t *pCalibdbV30, int tuning_idx);

AcnrV30_result_t cnr_config_setting_param_json_V30(RK_CNR_Params_V30_t *pParams, CalibDbV2_CNRV30_t *pCalibdbV30, char* param_mode, char * snr_name);



RKAIQ_END_DECLARE





#endif

