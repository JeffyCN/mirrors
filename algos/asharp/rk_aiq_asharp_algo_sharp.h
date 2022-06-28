#ifndef __RKAIQ_ASHARP_SHARP_H__
#define __RKAIQ_ASHARP_SHARP_H__

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "rk_aiq_types_asharp_algo_prvt.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"



RKAIQ_BEGIN_DECLARE

AsharpResult_t sharp_get_mode_cell_idx_by_name_v1(CalibDb_Sharp_2_t *pCalibdb, char *name, int *mode_idx);

AsharpResult_t sharp_get_setting_idx_by_name_v1(CalibDb_Sharp_2_t *pCalibdb, char *name, int mode_idx, int *setting_idx);

AsharpResult_t sharp_config_setting_param_v1(RKAsharp_Sharp_HW_Params_t *pParams, CalibDb_Sharp_2_t *pCalibdb, char *param_mode, char* snr_name);

AsharpResult_t init_sharp_params_v1(RKAsharp_Sharp_HW_Params_t *pParams, CalibDb_Sharp_2_t *pCalibdb, int mode_idx, int setting_idx);

AsharpResult_t select_rk_sharpen_hw_params_by_ISO(RKAsharp_Sharp_HW_Params_t *strksharpenParams, RKAsharp_Sharp_HW_Params_Select_t *strksharpenParamsSelected, AsharpExpInfo_t *pExpInfo);

AsharpResult_t select_rk_sharpen_hw_v2_params_by_ISO(RKAsharp_Sharp_HW_V2_Params_t *strksharpenParams, RKAsharp_Sharp_HW_V2_Params_Select_t *strksharpenParamsSelected, AsharpExpInfo_t *pExpInfo);

AsharpResult_t select_rk_sharpen_hw_v3_params_by_ISO (RKAsharp_Sharp_HW_V3_Params_t *strksharpenParams, RKAsharp_Sharp_HW_V3_Params_Select_t *strksharpenParamsSelected, AsharpExpInfo_t *pExpInfo);

void select_sharpen_params_by_ISO(RKAsharp_Sharp_Params_t *strksharpenParams, RKAsharp_Sharp_Params_Select_t *strksharpenParamsSelected, AsharpExpInfo_t *pExpInfo);

AsharpResult_t rk_Sharp_V1_fix_transfer(RKAsharp_Sharp_HW_Params_Select_t *pSharpV1, RKAsharp_Sharp_HW_Fix_t* pSharpCfg, float fPercent);

AsharpResult_t rk_Sharp_fix_transfer(RKAsharp_Sharp_Params_Select_t* sharp, RKAsharp_Sharp_Fix_t* pSharpCfg, float fPercent);

AsharpResult_t sharp_config_setting_param_v1_json(RKAsharp_Sharp_HW_Params_t *pParams, CalibDbV2_SharpV1_t *pCalibdb, char *param_mode, char* snr_name);

AsharpResult_t init_sharp_params_v1_json(RKAsharp_Sharp_HW_Params_t *pParams, CalibDbV2_SharpV1_t *pCalibdb,  int setting_idx);

AsharpResult_t sharp_get_setting_idx_by_name_v1_json(CalibDbV2_SharpV1_t *pCalibdb, char *name,  int *setting_idx);

AsharpResult_t sharp_calibdbV2_assign(CalibDbV2_SharpV1_t *pDst, CalibDbV2_SharpV1_t *pSrc);

void sharp_calibdbV2_free(CalibDbV2_SharpV1_t *pCalibdbV2);

AsharpResult_t sharp_algo_param_printf(RKAsharp_Sharp_HW_Params_t *pParams);



RKAIQ_END_DECLARE

#endif

