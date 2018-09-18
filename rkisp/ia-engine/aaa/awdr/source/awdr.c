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

  if (pAwdrCtx->WdrEnable) {
    switch (pConfig->mode) {
      case AWDR_MODE_CONTROL_BY_GAIN:
        // caluclate init strength
        result = AwdrCalculateWdrMaxGainLevel(&pAwdrCtx->pWdrGlobal->wdr_MaxGain_Level_curve, pConfig->fSensorGain, &pAwdrCtx->Wdr_MaxGain_level_RegValue);
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

      if (Wdr_MaxGain_level_RegValue != pAwdrCtx->Wdr_MaxGain_level_RegValue) {
        pAwdrCtx->Wdr_MaxGain_level_RegValue = Wdr_MaxGain_level_RegValue;
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
    result->Wdr_MaxGain_level_RegValue = pAwdrCtx->Wdr_MaxGain_level_RegValue;
    result->actives |= AWDR_WDR_MAXGAIN_LEVEL_MASK;
    pAwdrCtx->actives &= ~AWDR_WDR_MAXGAIN_LEVEL_MASK;
    ALOGV( "%s: sensor gain(%f) Wdr maxgain(%d)\n", __func__, pAwdrCtx->gain, pAwdrCtx->Wdr_MaxGain_level_RegValue);
  }

  return RET_SUCCESS;
}



