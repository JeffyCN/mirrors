#ifndef _RK_ISP20_YNR_H_
#define _RK_ISP20_YNR_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "anr/rk_aiq_types_anr_algo_prvt.h"
#include "RkAiqCalibDbTypesV2.h"



RKAIQ_BEGIN_DECLARE

ANRresult_t ynr_get_mode_cell_idx_by_name(CalibDb_YNR_2_t *pCalibdb, char *name, int *mode_idx);

ANRresult_t ynr_get_setting_idx_by_name(CalibDb_YNR_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

ANRresult_t ynr_config_setting_param(RKAnr_Ynr_Params_s *pParams, CalibDb_YNR_2_t *pCalibdb, char* param_mode, char* snr_name);

ANRresult_t init_ynr_params(RKAnr_Ynr_Params_s *pYnrParams, CalibDb_YNR_2_t* pYnrCalib, int mode_idx, int setting_idx);

ANRresult_t select_ynr_params_by_ISO(RKAnr_Ynr_Params_t *stYnrParam, RKAnr_Ynr_Params_Select_t *stYnrParamSelected, ANRExpInfo_t *pExpInfo, short bitValue);

ANRresult_t ynr_fix_transfer(RKAnr_Ynr_Params_Select_t* ynr, RKAnr_Ynr_Fix_t *pNrCfg, float gain_ratio, float fStrength);

ANRresult_t ynr_fix_printf(RKAnr_Ynr_Fix_t * pNrCfg);

ANRresult_t ynr_get_setting_idx_by_name_json(CalibDbV2_YnrV1_t *pCalibdb, char *name, int *calib_idx, int *tuning_idx);

ANRresult_t init_ynr_params_json(RKAnr_Ynr_Params_s *pYnrParams, CalibDbV2_YnrV1_t* pYnrCalib,  int calib_idx, int tuning_idx);

ANRresult_t ynr_config_setting_param_json(RKAnr_Ynr_Params_s *pParams, CalibDbV2_YnrV1_t*pCalibdb, char* param_mode, char* snr_name);

ANRresult_t ynr_calibdbV2_assign(CalibDbV2_YnrV1_t *pDst, CalibDbV2_YnrV1_t *pSrc);

void ynr_calibdbV2_free(CalibDbV2_YnrV1_t *pCalibdbV2);

ANRresult_t ynr_algo_param_printf(RKAnr_Ynr_Params_s *pYnrParams);

RKAIQ_END_DECLARE


#endif  // INITAL_ALG_PARAMS_YNR_H_
