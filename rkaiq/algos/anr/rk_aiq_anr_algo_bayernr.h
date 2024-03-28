
#ifndef _RKAIQ_BAYERNR_H_
#define _RKAIQ_BAYERNR_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "anr/rk_aiq_types_anr_algo_prvt.h"
#include "RkAiqCalibDbTypesV2.h"


RKAIQ_BEGIN_DECLARE

ANRresult_t bayernr_get_mode_cell_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int *mode_idx);

ANRresult_t bayernr_get_setting_idx_by_name(CalibDb_BayerNr_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

ANRresult_t bayernr_config_setting_param(RKAnr_Bayernr_Params_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, char* param_mode, char * snr_name);

ANRresult_t init_bayernr_params(RKAnr_Bayernr_Params_t *pParams, CalibDb_BayerNr_2_t *pCalibdb, int mode_idx, int setting_idx);

ANRresult_t select_bayernr_params_by_ISO(RKAnr_Bayernr_Params_t *stBayerNrParams, RKAnr_Bayernr_Params_Select_t *stBayerNrParamsSelected, ANRExpInfo_t *pExpInfo);

unsigned short bayernr_get_trans(int tmpfix);

ANRresult_t bayernr_fix_tranfer(RKAnr_Bayernr_Params_Select_t* rawnr, RKAnr_Bayernr_Fix_t *pRawnrCfg, float fStrength);

ANRresult_t bayernr_fix_printf(RKAnr_Bayernr_Fix_t * pRawnrCfg);

ANRresult_t bayernr_get_setting_idx_by_name_json(CalibDbV2_BayerNrV1_t *pCalibdb, char *name,  int *calib_idx,  int * tuning_idx);

ANRresult_t init_bayernr_params_json(RKAnr_Bayernr_Params_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, int calib_idx, int tuning_idx);

ANRresult_t bayernr_config_setting_param_json(RKAnr_Bayernr_Params_t *pParams, CalibDbV2_BayerNrV1_t *pCalibdb, char* param_mode, char * snr_name);

ANRresult_t bayernr_calibdbV2_assign(CalibDbV2_BayerNrV1_t *pDst, CalibDbV2_BayerNrV1_t *pSrc);

void bayernr_calibdbV2_free(CalibDbV2_BayerNrV1_t *pCalibdbV2);

ANRresult_t bayernr_algo_param_printf(RKAnr_Bayernr_Params_t *pParams);


RKAIQ_END_DECLARE

#endif  // BAYERNR_READ_PARM_H_

