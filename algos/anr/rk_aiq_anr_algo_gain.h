#ifndef _RKAIQ_GAIN_H_
#define _RKAIQ_GAIN_H_

#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "base/xcam_log.h"
#include "rk_aiq_comm.h"
#include "RkAiqCalibDbTypes.h"
#include "anr/rk_aiq_types_anr_algo_prvt.h"

ANRresult_t gain_fix_transfer(RKAnr_Mfnr_Params_Select_t *pMfnrSelect, RKAnr_Gain_Fix_t* pGainFix,  ANRExpInfo_t *pExpInfo, float gain_ratio);




#endif

