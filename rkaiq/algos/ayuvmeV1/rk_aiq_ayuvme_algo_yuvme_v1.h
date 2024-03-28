
#ifndef _RKAIQ_YUVME_V1_H_
#define _RKAIQ_YUVME_V1_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibApi.h"
#include "ayuvmeV1/rk_aiq_types_ayuvme_algo_prvt_v1.h"

XCAM_BEGIN_DECLARE

Ayuvme_result_V1_t yuvme_select_params_by_ISO_V1(RK_YUVME_Params_V1_t *pParams, RK_YUVME_Params_V1_Select_t *pSelect, Ayuvme_ExpInfo_V1_t *pExpInfo);

Ayuvme_result_V1_t yuvme_fix_transfer_V1(RK_YUVME_Params_V1_Select_t* pSelect, RK_YUVME_Fix_V1_t *pFix, rk_aiq_yuvme_strength_v1_t* pStrength, Ayuvme_ExpInfo_V1_t *pExpInfo);

Ayuvme_result_V1_t yuvme_fix_printf_V1(RK_YUVME_Fix_V1_t * pFix);

Ayuvme_result_V1_t yuvme_get_setting_by_name_json_V1(CalibDbV2_YuvmeV1_t *pCalibdbV2, char *name, int* tuning_idx);

Ayuvme_result_V1_t yuvme_init_params_json_V1(RK_YUVME_Params_V1_t *pYuvmeParams, CalibDbV2_YuvmeV1_t *pCalibdbV2, int tuning_idx);

Ayuvme_result_V1_t yuvme_config_setting_param_json_V1(RK_YUVME_Params_V1_t *pParams, CalibDbV2_YuvmeV1_t *pCalibdbV2, char* param_mode, char * snr_name);





XCAM_END_DECLARE


#endif  // BAYERNR_READ_PARM_H_

