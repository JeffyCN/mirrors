
#ifndef _RKAIQ_BAYERTNR_V30_H_
#define _RKAIQ_BAYERTNR_V30_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "abayertnrV30/rk_aiq_types_abayertnr_algo_prvt_v30.h"

XCAM_BEGIN_DECLARE

Abayertnr_result_V30_t bayertnr_select_params_by_ISO_V30(void* pParams_v, void* pSelect_v,
        Abayertnr_ExpInfo_V30_t* pExpInfo);

Abayertnr_result_V30_t bayertnr_fix_transfer_V30(void* pSelect_v, RK_Bayertnr_Fix_V30_t* pFix,
        rk_aiq_bayertnr_strength_v30_t* pStrength,
        Abayertnr_ExpInfo_V30_t* pExpInfo,
        Abayertnr_trans_params_v30_t *pTransPrarms,
        Abayertnr_Stats_V30_t *pStats);

Abayertnr_result_V30_t bayertnr_fix_printf_V30(RK_Bayertnr_Fix_V30_t * pFix);

Abayertnr_result_V30_t bayertnr_get_setting_by_name_json_V30(void* pCalibdb_v, char* name,
        int* calib_idx, int* tuning_idx);

Abayertnr_result_V30_t bayertnr_init_params_json_V30(void* pParams_v, void* pCalibdb_v,
        int calib_idx, int tuning_idx);

Abayertnr_result_V30_t bayertnr_config_setting_param_json_V30(void* pParams_v, void* pCalibdbV30_v,
        char* param_mode, char* snr_name);

XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

