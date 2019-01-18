/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file awdr.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <base/log.h>

#include <common/return_codes.h>
#include <common/misc.h>

#include "awdr.h"
#include "awdr_ctrl.h"

/*used by gloabl mode*/
static uint32_t cam_ia_wdr_def_global_y[AWDR_WDR_SECTION_MAX ] = {
  0x0000, 0x00a2, 0x00a2, 0x016a,
  0x01e6, 0x029c, 0x02e6, 0x0368,
  0x03d9, 0x049b, 0x049b, 0x058d,
  0x0619, 0x069a, 0x0712, 0x0783,
  0x07ed, 0x0852, 0x090e, 0x0967,
  0x0a0f, 0x0a5f, 0x0af9, 0x0af9,
  0x0af9, 0x0c9a, 0x0d1e, 0x0d69,
  0x0e21, 0x0e9f, 0x0ef4, 0x0f74,
  0x1000,

};

/*used by block mode*/
static uint16_t cam_ia_wdr_def_block_y[AWDR_WDR_SECTION_MAX ] = {
  0x0000, 0x011c, 0x011c, 0x02d8,
  0x0375, 0x0478, 0x054f, 0x0609,
  0x06b0, 0x07d6, 0x07d6, 0x09b9,
  0x0b49, 0x0ca5, 0x0ddc, 0x0ef8,
  0x0fff, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000,
};

//each value means 2^(val+3)
static uint8_t  cam_ia_wdr_def_segment[AWDR_WDR_SECTION_MAX - 1] = {
  0x0, 0x1, 0x1, 0x2, 0x3, 0x2, 0x3, 0x3, //0x33232110
  0x4, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, //0x44444434
  0x4, 0x5, 0x4, 0x5, 0x4, 0x5, 0x5, 0x5, //0x55545454
  0x5, 0x5, 0x4, 0x5, 0x4, 0x3, 0x3, 0x2, //0x23345455
};


/******************************************************************************
 * local macro definitions
 *****************************************************************************/

static RESULT AwdrCalculateWdrMaxGainLevel
(
    CamCalibWdrMaxGainLevelCurve_t* pWdrMaxGainLevelCurve,
    const float             fSensorGain,
    uint8_t*       MaxGainLevelRegValue
) {
  ALOGV( "%s: (enter)\n", __func__);
  if (pWdrMaxGainLevelCurve == NULL) {
    ALOGE("%s: (exit) pWdrMaxGainLevelCurve == NULL \n", __func__);
    return (RET_NULL_POINTER);
  }

  if (fSensorGain < 1.0f) {
    ALOGE("%s: fSensorGain  < 1.0f  \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (pWdrMaxGainLevelCurve->nSize < 1) {
    ALOGE("%s: (exit) nSize == 0 \n", __func__);
    return (RET_INVALID_PARM);
  }

  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;
  float MaxGainResult = 1.0;
  nMax = (pWdrMaxGainLevelCurve->nSize - 1U);

  /* lower range check */
  if (Dgain < pWdrMaxGainLevelCurve->pfSensorGain_level[0]) {
    Dgain = pWdrMaxGainLevelCurve->pfSensorGain_level[0];
  }

  /* upper range check */
  if (Dgain > pWdrMaxGainLevelCurve->pfSensorGain_level[nMax]) {
    Dgain = pWdrMaxGainLevelCurve->pfSensorGain_level[nMax];
  }

  /* find x area */
  n = 0;
  while ((Dgain >=  pWdrMaxGainLevelCurve->pfSensorGain_level[n]) && (n <= nMax)) {
    ++n;
  }
  --n;

  /**
   * If n was larger than nMax, which means fSensorGain lies exactly on the
  * last interval border, we count fSensorGain to the last interval and
   * have to decrease n one more time */
  if (n == nMax) {
    --n;
  }

#if 0
  //interpolate
  MaxGainResult
    = ((pWdrMaxGainLevelCurve->pfMaxGain_level[n + 1] - pWdrMaxGainLevelCurve->pfMaxGain_level[n]) / (pWdrMaxGainLevelCurve->pfSensorGain_level[n + 1] - pWdrMaxGainLevelCurve->pfSensorGain_level[n]))
      * (Dgain - pWdrMaxGainLevelCurve->pfSensorGain_level[n])
      + (pWdrMaxGainLevelCurve->pfMaxGain_level[n]);
#else
  MaxGainResult = pWdrMaxGainLevelCurve->pfMaxGain_level[n + 1];

  if (Dgain == pWdrMaxGainLevelCurve->pfSensorGain_level[0])
    MaxGainResult = pWdrMaxGainLevelCurve->pfMaxGain_level[0];

#endif

  if (MaxGainResult < 1.0)
    MaxGainResult = 1.0;

  if (MaxGainResult > 15.0)
    MaxGainResult = 15.0;

  *MaxGainLevelRegValue = (((uint8_t)MaxGainResult) << 4);

  ALOGV( "%s: SensorGain(%f) MaxGainLimit(0x%02x) \n", __func__, Dgain, *MaxGainLevelRegValue);

  ALOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * AdpfApplyConfiguration()
 *****************************************************************************/
static RESULT AwdrApplyConfiguration
(
    AwdrContext_t*   pAwdrCtx,
    AwdrConfig_t*    pConfig
) {
  RESULT result = RET_SUCCESS;

  //CamerIcWdrConfig_t  WdrConfig;

  ALOGV( "%s: (enter)\n", __func__);

  if (pConfig->hCamCalibDb == NULL) {
    ALOGE("%s: hCamCalibDb NULL\n", __func__);
    return (RET_INVALID_PARM);
  }

  result = CamCalibDbGetWdrGlobal(pConfig->hCamCalibDb, &pAwdrCtx->pWdrGlobal);
  if (result == RET_SUCCESS) {
    pAwdrCtx->hCamCalibDb = pConfig->hCamCalibDb;
    if (pAwdrCtx->pWdrGlobal != NULL && pAwdrCtx->pWdrGlobal->Enabled) {
      pAwdrCtx->WdrEnable = 1;
      if (pAwdrCtx->pWdrGlobal->wdr_MaxGain_Level_curve.filter_enable) {
        pAwdrCtx->WdrMaxGainEnable = 1;
      } else {
        pAwdrCtx->WdrMaxGainEnable = 0;
      }
    }

  } else {
    ALOGE("%s: (enter) CamCalibDbGetWdrGlobal fail (%d)\n", __func__, result);
    pAwdrCtx->WdrEnable = 0;
    pAwdrCtx->WdrMaxGainEnable = 0;
  }

  unsigned int index = 0;
  AwdrResult_t* wdr_result = &pAwdrCtx->awdr_result;
  CamCalibWdrGlobal_t* pWdrGlobal = pAwdrCtx->pWdrGlobal; 
  
  wdr_result->wdr_enable = BOOL_TRUE;
  wdr_result->mode = 1;
  if (pWdrGlobal != NULL) {
    wdr_result->wdr_enable = pWdrGlobal->Enabled ? BOOL_TRUE : BOOL_FALSE;
    wdr_result->mode = pWdrGlobal->Mode;
  }
  
  for (index = 0; index < (AWDR_WDR_SECTION_MAX - 1); index++)
    wdr_result->wdr_dx[index] = cam_ia_wdr_def_segment[index];
  
  if (pWdrGlobal != NULL && pWdrGlobal->LocalCurve[AWDR_WDR_SECTION_MAX / 2] > 0) {
    for (index = 0; index < (AWDR_WDR_SECTION_MAX); index++)
      wdr_result->wdr_block_dy[index] = pWdrGlobal->LocalCurve[index];
  } else {
    for (index = 0; index < (AWDR_WDR_SECTION_MAX); index++)
      wdr_result->wdr_block_dy[index] = cam_ia_wdr_def_block_y[index];
  }
  
  if (pWdrGlobal != NULL && pWdrGlobal->GlobalCurve[AWDR_WDR_SECTION_MAX / 2] > 0) {
    for (index = 0; index < (AWDR_WDR_SECTION_MAX); index++)
      wdr_result->wdr_global_dy[index] = pWdrGlobal->GlobalCurve[index];
  } else {
    for (index = 0; index < (AWDR_WDR_SECTION_MAX); index++)
      wdr_result->wdr_global_dy[index] = cam_ia_wdr_def_global_y[index];
  }
  
  //now value means as ISP register
  wdr_result->wdr_pym_cc = 0x3;
  wdr_result->wdr_epsilon = 0xc;
  wdr_result->wdr_lvl_en = 0xf;
  wdr_result->wdr_gain_max_clip_enable = 0x1;
  wdr_result->wdr_gain_max_value = 0x40;
  wdr_result->wdr_bavg_clip = 0x3;
  wdr_result->wdr_nonl_segm = 0x0;
  wdr_result->wdr_nonl_open = 0x1;
  wdr_result->wdr_nonl_mode1 = 0x0;
  wdr_result->wdr_flt_sel = 0x1;
  wdr_result->wdr_gain_off1 = 0x000000cd;
  wdr_result->wdr_bestlight = 0x0ccc;
  wdr_result->wdr_noiseratio = 0x00ee;
  wdr_result->wdr_coe0 = 0x00000036;
  wdr_result->wdr_coe1 = 0x000000b7;
  wdr_result->wdr_coe2 = 0x00000012;
  wdr_result->wdr_coe_off = 0x0;
  
  if (pWdrGlobal != NULL) {
    wdr_result->wdr_pym_cc = pWdrGlobal->wdr_pym_cc;
    wdr_result->wdr_epsilon = pWdrGlobal->wdr_epsilon;
    wdr_result->wdr_lvl_en = pWdrGlobal->wdr_lvl_en;
    wdr_result->wdr_gain_max_clip_enable = pWdrGlobal->wdr_gain_max_clip_enable;
    wdr_result->wdr_gain_max_value = pWdrGlobal->wdr_gain_max_value;
    wdr_result->wdr_bavg_clip = pWdrGlobal->wdr_bavg_clip;
    wdr_result->wdr_nonl_segm = pWdrGlobal->wdr_nonl_segm;
    wdr_result->wdr_nonl_open = pWdrGlobal->wdr_nonl_open;
    wdr_result->wdr_nonl_mode1 = pWdrGlobal->wdr_nonl_mode1;
  
    wdr_result->wdr_flt_sel = pWdrGlobal->wdr_flt_sel;
    wdr_result->wdr_gain_off1 = pWdrGlobal->wdr_gain_off1;
    wdr_result->wdr_bestlight = pWdrGlobal->wdr_bestlight;
    wdr_result->wdr_noiseratio = pWdrGlobal->wdr_noiseratio;
    wdr_result->wdr_coe0 = pWdrGlobal->wdr_coe0;
    wdr_result->wdr_coe1 = pWdrGlobal->wdr_coe1;
    wdr_result->wdr_coe2 = pWdrGlobal->wdr_coe2;
    wdr_result->wdr_coe_off = pWdrGlobal->wdr_coe_off;
  }
  
  if (pAwdrCtx->WdrEnable) {
    switch (pConfig->mode) {
      case AWDR_MODE_CONTROL_BY_GAIN:
        // caluclate init strength
        result = AwdrCalculateWdrMaxGainLevel(&pAwdrCtx->pWdrGlobal->wdr_MaxGain_Level_curve, pConfig->fSensorGain, &pAwdrCtx->awdr_result.wdr_gain_max_value);
        if (result != RET_SUCCESS) {
          ALOGV( "%s : AwdrCalculateWdrMaxGainLevel failed", __func__);
          return (result);
        }
        break;
      default:
        ALOGV( "%s: pConfig->mode: %d isn't support",
              __func__, pConfig->mode);
        break;
    }

    pAwdrCtx->actives |= (AWDR_WDR_MAXGAIN_LEVEL_MASK);
  }
  /* save configuration into context */
  pAwdrCtx->Config = *pConfig;

  ALOGV( "%s: (exit)\n", __func__);

  return (result);
}

/******************************************************************************
 * AdpfInit()
 *****************************************************************************/
RESULT AwdrInit
(
    AwdrHandle_t* handlePtr,
    AwdrConfig_t* pConfig
) {
  RESULT result = RET_SUCCESS;

  AwdrContext_t* pAwdrCtx;

  ALOGV( "%s: (enter)\n", __func__);

  /* allocate auto exposure control context */
  pAwdrCtx = (AwdrContext_t*)malloc(sizeof(AwdrContext_t));
  if (NULL == pAwdrCtx) {
    ALOGV(  "%s: Can't allocate AWDR context\n",  __func__);
    return (RET_OUTOFMEM);
  }

  /* pre-initialize context */
  MEMSET(pAwdrCtx, 0, sizeof(*pAwdrCtx));
  pAwdrCtx->state         = AWDR_STATE_INITIALIZED;

  /* return handle */
  *handlePtr = (AwdrHandle_t)pAwdrCtx;

  /* config awdr during initialize */
  AwdrConfigure(*handlePtr, pConfig);

  ALOGV( "%s: (exit)\n", __func__);

  return (result);
}

/******************************************************************************
 * AdpfRelease()
 *****************************************************************************/
RESULT AwdrRelease
(
    AwdrHandle_t handle
) {
  AwdrContext_t* pAwdrCtx = (AwdrContext_t*)handle;

  ALOGV( "%s: (enter)\n", __func__);

  /* initial checks */
  if (NULL == pAwdrCtx) {
    return (RET_WRONG_HANDLE);
  }

#if 0
  /* check state */
  if ((ADPF_STATE_RUNNING == pAdpfCtx->state) ||
      (ADPF_STATE_LOCKED == pAdpfCtx->state)) {
    return (RET_BUSY);
  }
#endif
  MEMSET(pAwdrCtx, 0, sizeof(AwdrContext_t));
  free(pAwdrCtx);

  ALOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * AdpfConfigure()
 *****************************************************************************/
RESULT AwdrConfigure
(
    AwdrHandle_t handle,
    AwdrConfig_t* pConfig
) {
  RESULT result = RET_SUCCESS;

  AwdrContext_t* pAwdrCtx = (AwdrContext_t*) handle;

  ALOGV( "%s: (enter)\n", __func__);

  /* initial checks */
  if (NULL == pAwdrCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pConfig) {
    return (RET_INVALID_PARM);
  }

  if (pConfig->mode == AWDR_MODE_INVALID)
    pConfig->mode = AWDR_MODE_CONTROL_BY_GAIN;

  if (pConfig->fSensorGain == 0.0)
    pConfig->fSensorGain = 1.0f;

  if (memcmp(pConfig, &pAwdrCtx->Config, sizeof(AwdrConfig_t)))  {
    /* apply new configuration */
    result = AwdrApplyConfiguration(pAwdrCtx, pConfig);
    if (result != RET_SUCCESS) {
      ALOGE( "%s: Can't configure CamerIc WDR (%d)\n",  __func__, result);
      return (result);
    }
  }

  ALOGV( "%s: (exit)\n", __func__);
  return (result);
}

/******************************************************************************
 * AdpfProcessFrame()
 *****************************************************************************/
RESULT AwdrRun
(
    AwdrHandle_t    handle,
    const float     gain
) {
  AwdrContext_t* pAwdrCtx = (AwdrContext_t*)handle;

  RESULT result = RET_SUCCESS;

  float dgain = 0.0f; /* gain difference */

  ALOGV( "%s: (enter)\n", __func__);

  /* initial checks */
  if (pAwdrCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  //manual mode ,stop awdr
  if (pAwdrCtx->Config.mode == AWDR_MODE_CONTROL_BY_MANUAL)
	  return (result);

  //wdr dynamic with aec
  if (pAwdrCtx->WdrEnable && pAwdrCtx->WdrMaxGainEnable) {
    dgain = (gain > pAwdrCtx->gain) ? (gain - pAwdrCtx->gain) : (pAwdrCtx->gain - gain);
    if (dgain > 0.15f) {
      uint8_t Wdr_MaxGain_level_RegValue;
      result = AwdrCalculateWdrMaxGainLevel(&pAwdrCtx->pWdrGlobal->wdr_MaxGain_Level_curve, gain, &Wdr_MaxGain_level_RegValue);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);

      if (Wdr_MaxGain_level_RegValue != pAwdrCtx->awdr_result.wdr_gain_max_value) {
        pAwdrCtx->awdr_result.wdr_gain_max_value = Wdr_MaxGain_level_RegValue;
        pAwdrCtx->actives |= AWDR_WDR_MAXGAIN_LEVEL_MASK;
      }
      pAwdrCtx->gain = gain;
    }
  }

  ALOGV( "%s: (exit)\n", __func__);

  return (result);
}

RESULT AwdrGetResult
(
    AwdrHandle_t    handle,
    AwdrResult_t*  result
) {
  AwdrContext_t* pAwdrCtx = (AwdrContext_t*)handle;

  if (pAwdrCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  if (pAwdrCtx->actives & AWDR_WDR_MAXGAIN_LEVEL_MASK) {
    *result = pAwdrCtx->awdr_result;
    result->actives |= AWDR_WDR_MAXGAIN_LEVEL_MASK;
    pAwdrCtx->actives &= ~AWDR_WDR_MAXGAIN_LEVEL_MASK;
    ALOGV( "%s: sensor gain(%f) Wdr maxgain(%d)\n", __func__, pAwdrCtx->gain, pAwdrCtx->awdr_result.wdr_gain_max_value);
  }

  return RET_SUCCESS;
}



