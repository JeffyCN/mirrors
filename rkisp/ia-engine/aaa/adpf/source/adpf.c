/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file adpf.c
 *
 * @brief
 *   ADD_DESCRIPTION_HERE
 *
 *****************************************************************************/
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <ebase/utl_fixfloat.h>
#include <ebase/types.h>
#include <ebase/builtins.h>
#include <ebase/utl_fixfloat.h>


#include <common/return_codes.h>
#include <common/misc.h>

#include "adpf.h"
#include "adpf_ctrl.h"
#include "base/log.h"


#define RK_IESHARP_GIAN (4.0)


/******************************************************************************
 * local macro definitions
 *****************************************************************************/
#define DSP_3DNR_MIN_LEVEL      (1)
#define DSP_3DNR_MAX_LEVEL      (32)


#define UTL_FIX_MASK_U0800       0x0ff
#define UTL_FIX_MAX_U0800        255.499f //exactly this would be < 256 - 0.5
#define UTL_FIX_MIN_U0800          0.0f

typedef struct InterpolateCtx_s {
  float*    pX;         /**< x-array, lookup table */
  float*    pY;         /**< y-array         */
  uint16_t  size;     /**< size of above arrays  */
  float     x_i;        /**< value to find matching interval for in x-array */
  float   y_i;        /**< result calculated from x_i, pfx and pfy */
} InterpolateCtx_t;

static RESULT Interpolate
(
    InterpolateCtx_t* pInterpolationCtx
) {
  uint16_t n    = 0U;
  uint16_t nMax = 0U;

  // LOGV("%s: (enter)\n", __FUNCTION__);

  if (pInterpolationCtx == NULL) {
    return (RET_NULL_POINTER);
  }

  nMax = (pInterpolationCtx->size - 1U);

  /* lower range check */
  if (pInterpolationCtx->x_i < pInterpolationCtx->pX[0]) {
    pInterpolationCtx->y_i = pInterpolationCtx->pY[0];
	LOGV("%s: x_i(%f) < limit(%f), use limit instead! \n", __func__, pInterpolationCtx->x_i, pInterpolationCtx->pX[0]);
    return (RET_SUCCESS);
  }

  /* upper range check */
  if (pInterpolationCtx->x_i > pInterpolationCtx->pX[nMax]) {
    pInterpolationCtx->y_i = pInterpolationCtx->pY[nMax];
	LOGV("%s: x_i(%f) > limit(%f), use limit instead! \n", __func__, pInterpolationCtx->x_i, pInterpolationCtx->pX[nMax]);
    return (RET_SUCCESS);
  }

  /* find x area */
  n = 0;
  while ((pInterpolationCtx->x_i >= pInterpolationCtx->pX[n]) && (n <= nMax)) {
    ++n;
  }
  --n;

  /**
   * If n was larger than nMax, which means f_xi lies exactly on the
   * last interval border, we count f_xi to the last interval and
   * have to decrease n one more time */
  if (n == nMax) {
    --n;
  }

  pInterpolationCtx->y_i
    = ((pInterpolationCtx->pY[n + 1] - pInterpolationCtx->pY[n]) / (pInterpolationCtx->pX[n + 1] - pInterpolationCtx->pX[n]))
      * (pInterpolationCtx->x_i - pInterpolationCtx->pX[n])
      + (pInterpolationCtx->pY[n]);

  // LOGV("%s: (exit)\n", __FUNCTION__);

  return (RET_SUCCESS);
}


/*****************************************************************************/
/**
 * @brief   This array defines the green square radius for the spatial
 *          weight calculation. The square radius values are calculated
 *          by the following formula:  r^2 = (x^2 + y^2)
 */
/*****************************************************************************/
const float fSpatialRadiusSqrG[CAMERIC_DPF_MAX_SPATIAL_COEFFS]  = {
  2.0f,      /**< r^2 = (1^2 + 1^2) */
  4.0f,      /**< r^2 = (2^2 + 0^2) */
  9.0f,      /**< r^2 = ((2^2 + 2^2) + (3^2 + 1^2)) / 2 */
  16.0f,      /**< r^2 = (4^2 + 0^2) */
  19.0f,      /**< r^2 = ((3^2 + 3^2) + (4^2 + 2^2)) / 2 */
  32.0f       /**< r^2 = (4^2 + 4^2) */
};


/*****************************************************************************/
/**
 * @brief   This array defines the red/blue square radius for the spatial
 *          weight calculation. The square radius values are calculated
 *          by the following formula:  r^2 = (x^2 + y^2)
 */
/*****************************************************************************/
const float fSpatialRadiusSqrRB[CAMERIC_DPF_MAX_SPATIAL_COEFFS] = {
  4.0f,      /**< r^2 = (2^2 + 0^2) */
  8.0f,      /**< r^2 = (2^2 + 2^2) */
  16.0f,      /**< r^2 = (4^2 + 0^2) */
  20.0f,      /**< r^2 = (4^2 + 2^2) */
  38.0f,      /**< r^2 = ((6^2 + 0^2) + (6^2 + 2^2)) / 2 */
  52.0f       /**< r^2 = (6^2 + 4^2) */
};



/*****************************************************************************/
/**
 * @brief   This is an example/default NLL coefficient configuration for a
 *          static setup of DPF module. The NLL coefficients are selected by
 *          the gain, means is gain in the range of fMinGain to fMaxGain the
 *          coressponding NLL coefficients are programmed.
 */
/*****************************************************************************/
typedef struct CamerIcDpfNoiseLevelLookUpConfig_s {
  CamerIcDpfNoiseLevelLookUp_t    NLL;
  float                           fMinGain;
  float                           fMaxGain;
} CamerIcDpfNoiseLevelLookUpConfig_t;


#define CAMERIC_DPF_MAX_NLL     8
const CamerIcDpfNoiseLevelLookUpConfig_t CamerIcDpfNllDefault[CAMERIC_DPF_MAX_NLL] = {
  {
    .fMinGain    = 1.0,
    .fMaxGain    = 1.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x3FF, 0x1FF, 0x16B, 0x129, 0x102, 0x0D3,
        0x0B7, 0x0A3, 0x095, 0x081, 0x074, 0x06A,
        0x05B, 0x052, 0x04B, 0x045, 0x041
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 1.5,
    .fMaxGain    = 2.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x2D4, 0x2D4, 0x2D4, 0x2D3, 0x292, 0x238,
        0x1FB, 0x1CE, 0x1AB, 0x178, 0x154, 0x139,
        0x111, 0x0F6, 0x0E1, 0x0D1, 0x0C4
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 2.5,
    .fMaxGain    = 3.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x24F, 0x24F, 0x24F, 0x24E, 0x219, 0x1D0,
        0x19E, 0x179, 0x15D, 0x133, 0x116, 0x0FF,
        0x0DF, 0x0C9, 0x0B8, 0x0AB, 0x0A0
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 3.5,
    .fMaxGain    = 4.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x200, 0x200, 0x200, 0x1FF, 0x1D1, 0x191,
        0x166, 0x147, 0x12E, 0x10A, 0x0F1, 0x0DD,
        0x0C1, 0x0AE, 0x09F, 0x094, 0x08B
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 4.5,
    .fMaxGain    = 5.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x1CA, 0x1CA, 0x1CA, 0x1C9, 0x1A0, 0x167,
        0x141, 0x124, 0x10E, 0x0EE, 0x0D7, 0x0C6,
        0x0AD, 0x09B, 0x08E, 0x084, 0x07C
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 5.5,
    .fMaxGain    = 6.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x1A2, 0x1A2, 0x1A2, 0x1A1, 0x17C, 0x148,
        0x125, 0x10B, 0x0F7, 0x0D9, 0x0C4, 0x0B5,
        0x09E, 0x08E, 0x082, 0x079, 0x071
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 6.5,
    .fMaxGain    = 7.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x183, 0x183, 0x183, 0x182, 0x160, 0x12F,
        0x10F, 0x0F7, 0x0E4, 0x0C9, 0x0B6, 0x0A7,
        0x092, 0x083, 0x078, 0x070, 0x069
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  },
  {
    .fMinGain    = 7.5,
    .fMaxGain    = 8.5,
    .NLL         =
    {
      .NllCoeff   = {
        0x16A, 0x16A, 0x16A, 0x169, 0x149, 0x11C,
        0x0FD, 0x0E7, 0x0D6, 0x0BC, 0x0AA, 0x09C,
        0x089, 0x07B, 0x071, 0x069, 0x062
      },
      .xScale     = CAMERIC_NLL_SCALE_LOGARITHMIC
    }
  }
};


/******************************************************************************
 * local type definitions
 *****************************************************************************/


/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/


/******************************************************************************
 * local functions
 *****************************************************************************/

/*****************************************************************************/
/**
 * @brief   This local function prepares a resolution access identifier for
 *          calibration database access.
 *
 * @param   pAdpfCtx        adpf context
 * @param   hCamCalibDb     handle to calibration database to use
 * @param   width           image resolution ( width )
 * @param   height          image resolution ( height )
 * @param   framerate       framerate per second

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfPrepareCalibDbAccess
(
    AdpfContext_t*               pAdpfCtx,
    const CamCalibDbHandle_t    hCamCalibDb,
    const uint16_t              width,
    const uint16_t              height,
    const uint16_t              framerate
) {
  RESULT result = RET_SUCCESS;

  LOGV( "%s: (enter)\n", __func__);

  result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, width, height, &pAdpfCtx->ResName);
  if (RET_SUCCESS != result) {
    LOGV("%s: resolution (%dx%d@%d) not found in database\n", __func__, width, height, framerate);
    return (result);
  }
  LOGV( "%s: resolution = %s\n", __func__, pAdpfCtx->ResName);

  pAdpfCtx->hCamCalibDb = hCamCalibDb;

  LOGV( "%s: (exit)\n", __func__);

  return (result);
}


/*****************************************************************************/
/**
 * @brief   This local function prepares a resolution access identifier for
 *          calibration database access.
 *
 * @param   pAdpfCtx        adpf context
 * @param   pNll            resulted NLL coefficients

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfCalculateNllCoefficients
(
    AdpfContext_t*                   pAdpfCtx,

    const float                     fSensorGain,
    CamerIcDpfNoiseLevelLookUp_t*    pNll
) {
  (void) pAdpfCtx;

  RESULT result = RET_SUCCESS;

  int32_t i;

  LOGV( "%s: (enter)\n", __func__);

  // initial check
  if (fSensorGain < 1.0f) {
    return (RET_INVALID_PARM);
  }

  // loop over the default array
  for (i = 0; i < CAMERIC_DPF_MAX_NLL; i++) {
    if ((CamerIcDpfNllDefault[i].fMinGain <= fSensorGain)
        && (fSensorGain < CamerIcDpfNllDefault[i].fMaxGain)) {
      int32_t size = sizeof(CamerIcDpfNllDefault[i].NLL.NllCoeff);

      pNll->xScale = CamerIcDpfNllDefault[i].NLL.xScale;
      memcpy(pNll->NllCoeff, CamerIcDpfNllDefault[i].NLL.NllCoeff, size);

      return (RET_SUCCESS);
    }
  }

  LOGV( "%s: (exit)\n", __func__);

  return (result);
}



/*****************************************************************************/
/**
 * @brief   This local function calculates the spatial weights for Green
 *          and Red/Blue Pixel of the filter cluster.
 *
 *          s_weight = exp( -1 * (x^2 + y^2) / ( 2 * sigma^2) )
 *
 * @param   pAdpfCtx        adpf context
 * @param   SigmaGreen      sigma value for green pixel
 * @param   SigmaRedBlue    sigma value for red/blue pixel
 * @param   pSpatialG       resulted spatial weights for green
 * @param   pSpatialRB      resulted spatial weights for blue

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfCalculateSpatialWeights
(
    AdpfContext_t*          pAdpfCtx,
    const uint32_t         SigmaGreen,
    const uint32_t         SigmaRedBlue,

    CamerIcDpfSpatial_t*    pSpatialG,
    CamerIcDpfSpatial_t*    pSpatialRB
) {
  (void) pAdpfCtx;

  double dSigmaSqr = 0.0f;
  float fExpResult = 0.0f;
  uint32_t i       = 0UL;

  uint32_t sqr1    = 0UL;
  uint32_t sqr2    = 0UL;

  LOGV( "%s: (enter)\n", __func__);

  sqr1 = (SigmaGreen * SigmaGreen);
  sqr2 = (SigmaRedBlue * SigmaRedBlue);
  if (!sqr1 || !sqr2) {
    return (RET_DIVISION_BY_ZERO);
  }

  // spatial weights for green channel
  dSigmaSqr = (double)(sqr1);
  for (i = 0UL; i < CAMERIC_DPF_MAX_SPATIAL_COEFFS; i++) {
    fExpResult = (float)(16.0f * exp(-1.0f * (double)fSpatialRadiusSqrG[i] / (2.0f * dSigmaSqr)));
    if (fExpResult > 16.0f) {
      // clip to max value
      fExpResult = 16.0f;
    }
    pSpatialG->WeightCoeff[i] = (uint8_t)UtlFloatToFix_U0800(fExpResult);
  }

  // spatial weights for red/blue channel
  dSigmaSqr = (double)(sqr2);
  for (i = 0UL; i < CAMERIC_DPF_MAX_SPATIAL_COEFFS; i++) {
    fExpResult = (float)(16.0f * exp(-1.0f * (double)fSpatialRadiusSqrRB[i] / (2.0f * dSigmaSqr)));
    if (fExpResult > 16.0f) {
      // clip to max value
      fExpResult = 16.0f;
    }
    pSpatialRB->WeightCoeff[i] = (uint8_t)UtlFloatToFix_U0800(fExpResult);
  }

  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}



/*****************************************************************************/
/**
 * @brief   This local function calculates the strength value.
 *
 * @param   pAdpfCtx        adpf context
 * @param   fSensorGain     current sensor gain
 * @param   fGradient       gradient / slope
 * @param   fOffset         additve offset

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfCalculateStrength
(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    const float             fGradient,
    const float             fOffset,
    const float             fMin,
    const float             fDiv,

    CamerIcDpfInvStrength_t* pDynInvStrength
) {
  (void) pAdpfCtx;

  float fStrength = 0.0f ;

  LOGV( "%s: (enter)\n", __func__);

  // initial check
  if (fSensorGain < 1.0f) {
    return (RET_INVALID_PARM);
  }

  fStrength = sqrtf(fGradient * fSensorGain) + fOffset;
  fStrength = MIN(fMin, fStrength);

  /* The following values show examples:          */
  /* weight=0.251 -> 255, weight=0.5 -> 128,      */
  /* weight=1     ->  64, weight=1.0 *default*    */
  /* weight=1.25  ->  51, weight=1.5 -> 42,       */
  /* weight=1.75  ->  37, weight=2   -> 32        */

  if (fStrength <= 0.251f) {
    /* set to min. strength */
    pDynInvStrength->WeightR = 0x7FU;
    pDynInvStrength->WeightG = 0xFFU;
    pDynInvStrength->WeightB = 0x7FU;
  } else if (fStrength >= 128.0f) {
    /* set to max. strength - low bandpass */
    pDynInvStrength->WeightR = 0x00U;
    pDynInvStrength->WeightG = 0x00U;
    pDynInvStrength->WeightB = 0x00U;
  } else {
    /* fStrength is never 0.0f else division by null                */
    /* half the InvStrength of B/R, because no lost of sharpening   */
    pDynInvStrength->WeightR = (uint8_t)UtlFloatToFix_U0800((fDiv / fStrength));
    pDynInvStrength->WeightG = (uint8_t)UtlFloatToFix_U0800((fDiv / fStrength));
    pDynInvStrength->WeightB = (uint8_t)UtlFloatToFix_U0800((fDiv / fStrength));

  }

  LOGV("%s: (gain=%f fStrength=%f, R:%u, G:%u, B:%u)\n",
        __func__, fSensorGain, fStrength,
        pDynInvStrength->WeightR, pDynInvStrength->WeightG, pDynInvStrength->WeightB);

  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief   This local function calculates the strength value.
 *
 * @param   pAdpfCtx        adpf context
 * @param   fSensorGain     current sensor gain
 * @param   fGradient       gradient / slope
 * @param   fOffset         additve offset

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfCalculateDenoiseLevel
(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    CamDenoiseLevelCurve_t*   pDenoiseLevelCurve,
    CamerIcIspFltDeNoiseLevel_t* deNoiseLevel
) {
  (void) pAdpfCtx;
  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;//Denoise level gain
  // initial check
  if (pDenoiseLevelCurve == NULL) {
    LOGV("%s: pDenoiseLevelCurve == NULL \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (fSensorGain < 1.0f) {

    LOGV("%s: 222(enter)\n", __func__);
    return (RET_INVALID_PARM);
  }
  
  LOGV( "%s:(enter) fSensorGain(%f) size(%d)\n", __func__, fSensorGain, pDenoiseLevelCurve->ArraySize);

  nMax = (pDenoiseLevelCurve->ArraySize - 1U);
  /* lower range check */
  if (Dgain < pDenoiseLevelCurve->pSensorGain[0]) {
    Dgain = pDenoiseLevelCurve->pSensorGain[0];
  }

  /* upper range check */
  if (Dgain > pDenoiseLevelCurve->pSensorGain[nMax]) {
    Dgain = pDenoiseLevelCurve->pSensorGain[nMax];
  }

  /* find x area */
  n = 0;
  while ((n <= nMax) && (Dgain >=  pDenoiseLevelCurve->pSensorGain[n])) {
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
  float sub1 = ABS(pDenoiseLevelCurve->pSensorGain[n] - Dgain);
  float sub2 = ABS(pDenoiseLevelCurve->pSensorGain[n + 1] - Dgain);
  n = sub1 < sub2 ? n : n + 1;

  *deNoiseLevel = pDenoiseLevelCurve->pDlevel[n];
  if (*deNoiseLevel >  CAMERIC_ISP_FLT_DENOISE_LEVEL_MAX)
    *deNoiseLevel = CAMERIC_ISP_FLT_DENOISE_LEVEL_MAX - 1;

  if (*deNoiseLevel <  CAMERIC_ISP_FLT_DENOISE_LEVEL_INVALID)
    *deNoiseLevel = CAMERIC_ISP_FLT_DENOISE_LEVEL_INVALID + 1;

  *deNoiseLevel = *deNoiseLevel - 1;
  LOGV( "%s: gain=%f,dLelvel=%d\n", __func__, Dgain, *deNoiseLevel);
  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief   This local function calculates the strength value.
 *
 * @param   pAdpfCtx        adpf context
 * @param   fSensorGain     current sensor gain
 * @param   fGradient       gradient / slope
 * @param   fOffset         additve offset

 * @return                  Return the result of the function call.
 * @retval  RET_SUCCESS     function succeed
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
static RESULT AdpfCalculateSharpeningLevel
(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    CamSharpeningLevelCurve_t*   pSharpeningLevelCurve,
    CamerIcIspFltSharpeningLevel_t* sharpeningLevel
) {
  (void) pAdpfCtx;

  LOGV( "%s: (enter)\n", __func__);

  // initial check
  if (pSharpeningLevelCurve == NULL) {
    LOGV("%s: pSharpeningLevelCurve == NULL \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (fSensorGain < 1.0f) {
    LOGV("%s: fSensorGain  < 1.0f  \n", __func__);
    return (RET_INVALID_PARM);
  }
  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;//sharping level gain
  nMax = (pSharpeningLevelCurve->ArraySize - 1U);
  /* lower range check */
  if (Dgain < pSharpeningLevelCurve->pSensorGain[0]) {
    Dgain = pSharpeningLevelCurve->pSensorGain[0];
  }

  /* upper range check */
  if (Dgain > pSharpeningLevelCurve->pSensorGain[nMax]) {
    Dgain = pSharpeningLevelCurve->pSensorGain[nMax];
  }

  /* find x area */
  n = 0;
  while ((n <= nMax) && (Dgain >=  pSharpeningLevelCurve->pSensorGain[n])) {
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
  float sub1 = ABS(pSharpeningLevelCurve->pSensorGain[n] - Dgain);
  float sub2 = ABS(pSharpeningLevelCurve->pSensorGain[n + 1] - Dgain);
  n = sub1 < sub2 ? n : n + 1;

  *sharpeningLevel  = pSharpeningLevelCurve->pSlevel[n];
  if (*sharpeningLevel >  CAMERIC_ISP_FLT_SHARPENING_LEVEL_MAX)
    *sharpeningLevel = CAMERIC_ISP_FLT_SHARPENING_LEVEL_MAX - 1;

  if (*sharpeningLevel <  CAMERIC_ISP_FLT_SHARPENING_LEVEL_INVALID)
    *sharpeningLevel = CAMERIC_ISP_FLT_SHARPENING_LEVEL_INVALID + 1;

  *sharpeningLevel = *sharpeningLevel - 1;
  LOGV( "%s: gain=%f,sLelvel=%d\n", __func__, Dgain, *sharpeningLevel);
  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}

static RESULT AdpfCalculateDemosaicThLevel
(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    CamDemosaicThCurve_t*   pDemosaicThCurve,
    uint8_t* demosaic_th_level
) {
  (void) pAdpfCtx;

  LOGV( "%s: (enter)\n", __func__);

  // initial check
  if (pDemosaicThCurve == NULL) {
    LOGV("%s: pSharpeningLevelCurve == NULL \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (fSensorGain < 1.0f) {
    LOGV("%s: fSensorGain  < 1.0f  \n", __func__);
    return (RET_INVALID_PARM);
  }
  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;//sharping level gain
  nMax = (pDemosaicThCurve->ArraySize - 1U);
  /* lower range check */
  if (Dgain < pDemosaicThCurve->pSensorGain[0]) {
    Dgain = pDemosaicThCurve->pSensorGain[0];
  }

  /* upper range check */
  if (Dgain > pDemosaicThCurve->pSensorGain[nMax]) {
    Dgain = pDemosaicThCurve->pSensorGain[nMax];
  }

  /* find x area */
  n = 0;
  while ((n <= nMax) && (Dgain >=  pDemosaicThCurve->pSensorGain[n])) {
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
  float sub1 = ABS(pDemosaicThCurve->pSensorGain[n] - Dgain);
  float sub2 = ABS(pDemosaicThCurve->pSensorGain[n + 1] - Dgain);
  n = sub1 < sub2 ? n : n + 1;

  *demosaic_th_level  = pDemosaicThCurve->pThlevel[n];

  LOGV( "%s: gain=%f,demosaic_th=%d\n", __func__, Dgain, *demosaic_th_level);
  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}


static RESULT AdpfCalculate3DNRResult(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    CamDsp3DNRSettingProfile_t* pCamDsp3DNRSettingProfile,
    Dsp3DnrResult_t*     pDsp3DNRResult
) {
  (void) pAdpfCtx;
  CamDsp3DNRLumaSetting_t *pLumaSetting = &pCamDsp3DNRSettingProfile->sLumaSetting;
  CamDsp3DNRChrmSetting_t *pChrmSetting = &pCamDsp3DNRSettingProfile->sChrmSetting;
  CamDsp3DNRShpSetting_t *pSharpSetting = &pCamDsp3DNRSettingProfile->sSharpSetting;
  
  LOGV( "%s: (enter) \n", __func__);

  // initial check
  if (pCamDsp3DNRSettingProfile == NULL
      || pCamDsp3DNRSettingProfile->Enable== 0
     ) {
    LOGV("%s: NULL pointer \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (fSensorGain < 1.0f || pCamDsp3DNRSettingProfile->ArraySize < 1) {
    LOGV("%s: INVALID_PARM fSensorGain(%f)  ArraySize(%d) \n", __func__,
         fSensorGain, pCamDsp3DNRSettingProfile->ArraySize);
    return (RET_INVALID_PARM);
  }

  for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
	if(pLumaSetting->pluma_weight[i]==NULL 
		|| pChrmSetting->pchrm_weight[i]==NULL 
		 || pSharpSetting->psrc_shp_weight[i]==NULL )
	{
		LOGV("%s:%d: 3dnr para NULL pointer \n", __func__, __LINE__);
    	return (RET_INVALID_PARM);
	}
  }
  
  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;
  nMax = (pCamDsp3DNRSettingProfile->ArraySize - 1U);
  /* lower range check */
  if (Dgain < pCamDsp3DNRSettingProfile->pgain_Level[0]) {
    Dgain = pCamDsp3DNRSettingProfile->pgain_Level[0];
  }

  /* upper range check */
  if (Dgain > pCamDsp3DNRSettingProfile->pgain_Level[nMax]) {
    Dgain = pCamDsp3DNRSettingProfile->pgain_Level[nMax];
  }

  /* find x area */
  n = 0;
  while ((n <= nMax) && (Dgain >=  pCamDsp3DNRSettingProfile->pgain_Level[n])) {
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

  float sub1 = ABS(pCamDsp3DNRSettingProfile->pgain_Level[n] - Dgain);
  float sub2 = ABS(pCamDsp3DNRSettingProfile->pgain_Level[n + 1] - Dgain);
  n = sub1 < sub2 ? n : n + 1;

  pDsp3DNRResult->noise_coef_num = pCamDsp3DNRSettingProfile->pnoise_coef_numerator[n];
  pDsp3DNRResult->noise_coef_den= pCamDsp3DNRSettingProfile->pnoise_coef_denominator[n];
  pDsp3DNRResult->luma_sp_nr_en = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.luma_sp_nr_en;
  pDsp3DNRResult->luma_te_nr_en = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.luma_te_nr_en;
  pDsp3DNRResult->chrm_sp_nr_en = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.chrm_sp_nr_en;
  pDsp3DNRResult->chrm_te_nr_en = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.chrm_te_nr_en;
  pDsp3DNRResult->shp_en = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.shp_en;  
  pDsp3DNRResult->luma_sp_nr_level= pCamDsp3DNRSettingProfile->sDefaultLevelSetting.pluma_sp_nr_level[n];
  pDsp3DNRResult->luma_te_nr_level= pCamDsp3DNRSettingProfile->sDefaultLevelSetting.pluma_te_nr_level[n];
  pDsp3DNRResult->chrm_sp_nr_level= pCamDsp3DNRSettingProfile->sDefaultLevelSetting.pchrm_sp_nr_level[n];
  pDsp3DNRResult->chrm_te_nr_level= pCamDsp3DNRSettingProfile->sDefaultLevelSetting.pchrm_te_nr_level[n];
  pDsp3DNRResult->shp_level = pCamDsp3DNRSettingProfile->sDefaultLevelSetting.pshp_level[n];

  pDsp3DNRResult->luma_default = pCamDsp3DNRSettingProfile->sLumaSetting.luma_default;
  pDsp3DNRResult->luma_sp_rad = pCamDsp3DNRSettingProfile->sLumaSetting.pluma_sp_rad[n];
  pDsp3DNRResult->luma_te_max_bi_num = pCamDsp3DNRSettingProfile->sLumaSetting.pluma_te_max_bi_num[n];

  
  pDsp3DNRResult->luma_w0 = ((pLumaSetting->pluma_weight[0][n]&0x3f)<<24) | ((pLumaSetting->pluma_weight[1][n]&0x3f)<<18)
  							| ((pLumaSetting->pluma_weight[2][n]&0x3f)<<12) | ((pLumaSetting->pluma_weight[3][n]&0x3f)<<6)
  							| ((pLumaSetting->pluma_weight[4][n]&0x3f));
  pDsp3DNRResult->luma_w1 = ((pLumaSetting->pluma_weight[5][n]&0x3f)<<24) | ((pLumaSetting->pluma_weight[6][n]&0x3f)<<18)
  							| ((pLumaSetting->pluma_weight[7][n]&0x3f)<<12) | ((pLumaSetting->pluma_weight[8][n]&0x3f)<<6)
  							| ((pLumaSetting->pluma_weight[9][n]&0x3f));
  pDsp3DNRResult->luma_w2 = ((pLumaSetting->pluma_weight[10][n]&0x3f)<<26) | ((pLumaSetting->pluma_weight[11][n]&0x3f)<<20)
  							| ((pLumaSetting->pluma_weight[12][n]&0xff)<<12) | ((pLumaSetting->pluma_weight[13][n]&0x3f)<<6)
  							| ((pLumaSetting->pluma_weight[14][n]&0x3f));
  pDsp3DNRResult->luma_w3 = ((pLumaSetting->pluma_weight[15][n]&0x3f)<<24) | ((pLumaSetting->pluma_weight[16][n]&0x3f)<<18)
  							| ((pLumaSetting->pluma_weight[17][n]&0x3f)<<12) | ((pLumaSetting->pluma_weight[18][n]&0x3f)<<6)
  							| ((pLumaSetting->pluma_weight[19][n]&0x3f));
  pDsp3DNRResult->luma_w4 = ((pLumaSetting->pluma_weight[20][n]&0x3f)<<24) | ((pLumaSetting->pluma_weight[21][n]&0x3f)<<18)
  							| ((pLumaSetting->pluma_weight[22][n]&0x3f)<<12) | ((pLumaSetting->pluma_weight[23][n]&0x3f)<<6)
  							| ((pLumaSetting->pluma_weight[24][n]&0x3f));

  pDsp3DNRResult->chrm_default = pCamDsp3DNRSettingProfile->sChrmSetting.chrm_default;
  pDsp3DNRResult->chrm_sp_rad = pCamDsp3DNRSettingProfile->sChrmSetting.pchrm_sp_rad[n];
  pDsp3DNRResult->chrm_te_max_bi_num = pCamDsp3DNRSettingProfile->sChrmSetting.pchrm_te_max_bi_num[n];
  pDsp3DNRResult->chrm_w0 = ((pChrmSetting->pchrm_weight[0][n]&0x3f)<<24) | ((pChrmSetting->pchrm_weight[1][n]&0x3f)<<18)
  						  | ((pChrmSetting->pchrm_weight[2][n]&0x3f)<<12) | ((pChrmSetting->pchrm_weight[3][n]&0x3f)<<6)
  						  | ((pChrmSetting->pchrm_weight[4][n]&0x3f));
  pDsp3DNRResult->chrm_w1 = ((pChrmSetting->pchrm_weight[5][n]&0x3f)<<24) | ((pChrmSetting->pchrm_weight[6][n]&0x3f)<<18)
  						  | ((pChrmSetting->pchrm_weight[7][n]&0x3f)<<12) | ((pChrmSetting->pchrm_weight[8][n]&0x3f)<<6)
  						  | ((pChrmSetting->pchrm_weight[9][n]&0x3f));
  pDsp3DNRResult->chrm_w2 = ((pChrmSetting->pchrm_weight[10][n]&0x3f)<<26) | ((pChrmSetting->pchrm_weight[11][n]&0x3f)<<20)
  						  | ((pChrmSetting->pchrm_weight[12][n]&0xff)<<12) | ((pChrmSetting->pchrm_weight[13][n]&0x3f)<<6)
  						  | ((pChrmSetting->pchrm_weight[14][n]&0x3f));
  pDsp3DNRResult->chrm_w3 = ((pChrmSetting->pchrm_weight[15][n]&0x3f)<<24) | ((pChrmSetting->pchrm_weight[16][n]&0x3f)<<18)
  						  | ((pChrmSetting->pchrm_weight[17][n]&0x3f)<<12) | ((pChrmSetting->pchrm_weight[18][n]&0x3f)<<6)
  						  | ((pChrmSetting->pchrm_weight[19][n]&0x3f));
  pDsp3DNRResult->chrm_w4 = ((pChrmSetting->pchrm_weight[20][n]&0x3f)<<24) | ((pChrmSetting->pchrm_weight[21][n]&0x3f)<<18)
  						  | ((pChrmSetting->pchrm_weight[22][n]&0x3f)<<12) | ((pChrmSetting->pchrm_weight[23][n]&0x3f)<<6)
  						  | ((pChrmSetting->pchrm_weight[24][n]&0x3f));

  pDsp3DNRResult->shp_default = pCamDsp3DNRSettingProfile->sSharpSetting.shp_default;
  pDsp3DNRResult->src_shp_c = pCamDsp3DNRSettingProfile->sSharpSetting.psrc_shp_c[n];
  pDsp3DNRResult->src_shp_l = pCamDsp3DNRSettingProfile->sSharpSetting.psrc_shp_l[n];
  pDsp3DNRResult->src_shp_div = pCamDsp3DNRSettingProfile->sSharpSetting.psrc_shp_div[n];
  pDsp3DNRResult->src_shp_thr = pCamDsp3DNRSettingProfile->sSharpSetting.psrc_shp_thr[n];
  pDsp3DNRResult->src_shp_w0 = ((pSharpSetting->psrc_shp_weight[0][n]&0x3f)<<24) | ((pSharpSetting->psrc_shp_weight[1][n]&0x3f)<<18)
  						  | ((pSharpSetting->psrc_shp_weight[2][n]&0x3f)<<12) | ((pSharpSetting->psrc_shp_weight[3][n]&0x3f)<<6)
  						  | ((pSharpSetting->psrc_shp_weight[4][n]&0x3f));
  pDsp3DNRResult->src_shp_w1 = ((pSharpSetting->psrc_shp_weight[5][n]&0x3f)<<24) | ((pSharpSetting->psrc_shp_weight[6][n]&0x3f)<<18)
  						  | ((pSharpSetting->psrc_shp_weight[7][n]&0x3f)<<12) | ((pSharpSetting->psrc_shp_weight[8][n]&0x3f)<<6)
  						  | ((pSharpSetting->psrc_shp_weight[9][n]&0x3f));
  pDsp3DNRResult->src_shp_w2 = ((pSharpSetting->psrc_shp_weight[10][n]&0x3f)<<26) | ((pSharpSetting->psrc_shp_weight[11][n]&0x3f)<<20)
  						  | ((pSharpSetting->psrc_shp_weight[12][n]&0xff)<<12) | ((pSharpSetting->psrc_shp_weight[13][n]&0x3f)<<6)
  						  | ((pSharpSetting->psrc_shp_weight[14][n]&0x3f));
  pDsp3DNRResult->src_shp_w3 = ((pSharpSetting->psrc_shp_weight[15][n]&0x3f)<<24) | ((pSharpSetting->psrc_shp_weight[16][n]&0x3f)<<18)
  						  | ((pSharpSetting->psrc_shp_weight[17][n]&0x3f)<<12) | ((pSharpSetting->psrc_shp_weight[18][n]&0x3f)<<6)
  						  | ((pSharpSetting->psrc_shp_weight[19][n]&0x3f));
  pDsp3DNRResult->src_shp_w4 = ((pSharpSetting->psrc_shp_weight[20][n]&0x3f)<<24) | ((pSharpSetting->psrc_shp_weight[21][n]&0x3f)<<18)
  						  | ((pSharpSetting->psrc_shp_weight[22][n]&0x3f)<<12) | ((pSharpSetting->psrc_shp_weight[23][n]&0x3f)<<6)
  						  | ((pSharpSetting->psrc_shp_weight[24][n]&0x3f));

  LOGV( "%s: oyyf gain=%f, n=%d, luma_sp:%d luma_te:%d chrm_sp:%d chrm_te:%d shp:%d noise:num(%d) den(%d)\n", 
  		__func__, Dgain, n,
        pDsp3DNRResult->luma_sp_nr_level, pDsp3DNRResult->luma_te_nr_level, 
        pDsp3DNRResult->chrm_sp_nr_level, pDsp3DNRResult->chrm_te_nr_level, pDsp3DNRResult->shp_level,
        pDsp3DNRResult->noise_coef_num, pDsp3DNRResult->noise_coef_den);
	 
  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}


static RESULT AdpfCalculateNew3DNRResult(
    AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
    CamNewDsp3DNRProfile_t* pCamNewDsp3DNRProfile,
    NewDsp3DnrResult_t*     pNewDsp3DnrResult
) {
  (void) pAdpfCtx;
  
  LOGV( "%s: (enter) \n", __func__);

  // initial check
  if (pCamNewDsp3DNRProfile == NULL
      || pCamNewDsp3DNRProfile->enable_3dnr == 0
     ) {
    LOGV( "%s: NULL pointer \n", __func__);
    return (RET_INVALID_PARM);
  }

  if (fSensorGain < 1.0f || pCamNewDsp3DNRProfile->ArraySize < 1) {
    LOGV( "%s: INVALID_PARM fSensorGain(%d)  ArraySize(%d) \n", __func__);
    return (RET_INVALID_PARM);
  }
  
  uint16_t n    = 0U;
  uint16_t nMax = 0U;
  float Dgain = fSensorGain;
  nMax = (pCamNewDsp3DNRProfile->ArraySize - 1U);
  /* lower range check */
  if (Dgain < pCamNewDsp3DNRProfile->pgain_Level[0]) {
    Dgain = pCamNewDsp3DNRProfile->pgain_Level[0];
  }

  /* upper range check */
  if (Dgain > pCamNewDsp3DNRProfile->pgain_Level[nMax]) {
    Dgain = pCamNewDsp3DNRProfile->pgain_Level[nMax];
  }

  /* find x area */
  n = 0;
  while ((Dgain >=  pCamNewDsp3DNRProfile->pgain_Level[n]) && (n <= nMax)) {
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

  float sub1 = ABS(pCamNewDsp3DNRProfile->pgain_Level[n] - Dgain);
  float sub2 = ABS(pCamNewDsp3DNRProfile->pgain_Level[n + 1] - Dgain);
  n = sub1 < sub2 ? n : n + 1;

  pNewDsp3DnrResult->ynr_time_weight = pCamNewDsp3DNRProfile->ynr.pynr_time_weight_level[n];
  pNewDsp3DnrResult->ynr_spat_weight = pCamNewDsp3DNRProfile->ynr.pynr_spat_weight_level[n];
  pNewDsp3DnrResult->uvnr_weight = pCamNewDsp3DNRProfile->uvnr.puvnr_weight_level[n];
  pNewDsp3DnrResult->sharp_weight = pCamNewDsp3DNRProfile->sharp.psharp_weight_level[n];

  LOGV( "%s: gain=%f, n=%d, ynr_time_weight:%d ynr_spat_weight:%d uvnr_weight:%d sharp_weight:%d\n", 
  		__func__, Dgain, n,
		pNewDsp3DnrResult->ynr_time_weight,
		pNewDsp3DnrResult->ynr_spat_weight,
		pNewDsp3DnrResult->uvnr_weight,
		pNewDsp3DnrResult->sharp_weight);
	 
  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}

uint16_t AdpfRKLpInterpolate
(
	float *src_divided,
	float *gainArry,
	uint8_t counts,
	const float AECgain
)
{
	uint16_t retvalue=0;
	InterpolateCtx_t InterpolateCtx;

	if(src_divided == NULL){
		LOGV("%s: src_divided == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}

	if(gainArry == NULL){
		LOGV("%s: gainArry == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}
	
	InterpolateCtx.size = counts;
	InterpolateCtx.x_i = AECgain;
	InterpolateCtx.pX = gainArry;
	InterpolateCtx.pY = src_divided;

	if(counts!=0)
	{
		if(RET_OUTOFRANGE == Interpolate(&InterpolateCtx))
		{
			LOGV("%s: Interpolate :out of range!\n",__func__);
		}

		retvalue = (uint16_t)InterpolateCtx.y_i;
	}

	return(retvalue);

}


/*****************************************************************************
*AdpfRKLpCalMatrix
********************************************************************************/
static RESULT AdpfRKLpCalMatrix
(
	AdpfContext_t*          pAdpfCtx,
    const float             fSensorGain,
	CamDemosaicLpProfile_t *pRKDLpConf,
	RKDemosiacLpResult_t   *prkDLpResult
)
{
    (void) pAdpfCtx;
	uint8_t counts = 6;
	RESULT result = RET_SUCCESS;

	
	if (pRKDLpConf == NULL) {
		LOGV("%s: pRKDLpConf == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}

	if(fSensorGain < 1.0f)
	{
		LOGV( "%s: AECgain(%f) is invalid\n", __func__,fSensorGain );
		 return ( RET_INVALID_PARM );
	}

	if (prkDLpResult == NULL) {
		LOGV("%s: prkDLpResult == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}
	
	prkDLpResult->thgrad_divided[0] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thH_divided0), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_divided[1] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thH_divided1), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_divided[2] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thH_divided2), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_divided[3] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thH_divided3), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_divided[4] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thH_divided4), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thcsc_divided[0] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thCSC_divided0), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thcsc_divided[1] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thCSC_divided1), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thcsc_divided[2] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thCSC_divided2), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thcsc_divided[3] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thCSC_divided3), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thcsc_divided[4] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thCSC_divided4), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_divided[0] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->diff_divided0), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_divided[1] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->diff_divided1), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_divided[2] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->diff_divided2), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_divided[3] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->diff_divided3), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_divided[4] = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->diff_divided4), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_divided[0] = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->varTh_divided0), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_divided[1] = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->varTh_divided1), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_divided[2] = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->varTh_divided2), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_divided[3] = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->varTh_divided3), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_divided[4] = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->varTh_divided4), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->th_grad = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->th_grad), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->th_diff = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->th_diff), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->th_csc = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->th_csc), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->th_var = (uint16_t)AdpfRKLpInterpolate((pRKDLpConf->th_var), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_r_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thgrad_r_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_r_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thdiff_r_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_r_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thvar_r_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thgrad_b_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thgrad_b_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thdiff_b_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thdiff_b_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->thvar_b_fct = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->thvar_b_fct), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->similarity_th = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->similarity_th), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->flat_level_sel= (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->flat_level_sel), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->pattern_level_sel = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->pattern_level_sel), (pRKDLpConf->gainsArray), counts, fSensorGain);
	prkDLpResult->edge_level_sel = (uint8_t)AdpfRKLpInterpolate((pRKDLpConf->edge_level_sel), (pRKDLpConf->gainsArray), counts, fSensorGain);
	
	return ( result );
}

static RESULT AdpfCalcRKIESharpFilterCoe
(
	AdpfContext_t*           pAdpfCtx,
    const float             fSensorGain,
	CamIesharpenProfile_t *pRKIESharpProfile,
	RKIESharpResult_t *pRKIEsharpResult
)
{
    (void) pAdpfCtx;
	RESULT result = RET_SUCCESS;
   	CamIesharpenGridConf_t *pGridConf = NULL;

	if (pRKIESharpProfile == NULL) {
		LOGV("%s: pRKIESharpProfile == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}

	if (fSensorGain < 1.0f) {

		LOGV("%s: sensorgain wrong(enter)\n", __func__);
		return (RET_INVALID_PARM);
	}
  	
	if (pRKIEsharpResult == NULL) {
		LOGV("%s: pRKIEsharpResult == NULL \n", __func__);
		return (RET_INVALID_PARM);
	}

	if(pRKIESharpProfile->gain_dvide<= 0)
		pRKIESharpProfile->gain_dvide = RK_IESHARP_GIAN;
	
	if(fSensorGain <= pRKIESharpProfile->gain_dvide){
		pGridConf = &pRKIESharpProfile->lgridconf;
	}else{
		pGridConf = &pRKIESharpProfile->hgridconf;
	}

	MEMCPY(pRKIEsharpResult->p_grad, pGridConf->p_grad, sizeof(pRKIEsharpResult->p_grad));
	MEMCPY(pRKIEsharpResult->sharp_factor, pGridConf->sharp_factor, sizeof(pRKIEsharpResult->sharp_factor));
	MEMCPY(pRKIEsharpResult->line1_filter_coe, pGridConf->line1_filter_coe, sizeof(pRKIEsharpResult->line1_filter_coe));
	MEMCPY(pRKIEsharpResult->line2_filter_coe, pGridConf->line2_filter_coe, sizeof(pRKIEsharpResult->line2_filter_coe));
	MEMCPY(pRKIEsharpResult->line3_filter_coe, pGridConf->line3_filter_coe, sizeof(pRKIEsharpResult->line3_filter_coe));	
	MEMCPY(pRKIEsharpResult->lap_mat_coe, pGridConf->lap_mat_coe, sizeof(pRKIEsharpResult->lap_mat_coe));	

	return result;
}

/******************************************************************************
 * AdpfApplyConfiguration()
 *****************************************************************************/
static RESULT AdpfApplyConfiguration
(
    AdpfContext_t*   pAdpfCtx,
    AdpfConfig_t*    pConfig
) {
  RESULT result = RET_SUCCESS;

  CamerIcDpfConfig_t  DpfConfig;
  CamerIcGains_t      NfGains;

  LOGV("%s: (enter)\n", __func__);

  // clear
  MEMSET(&DpfConfig, 0, sizeof(DpfConfig));
  MEMSET(&NfGains, 0, sizeof(NfGains));

  // configuration with data from calibration database
  if (pConfig->type == ADPF_USE_CALIB_DATABASE) {
    CamDpfProfile_t* pDpfProfile = NULL;

    // initial check
    if (NULL == pConfig->data.db.hCamCalibDb) {
      LOGV("%s:  pConfig->data.db.hCamCalibDb NULL", __func__);
      return (RET_INVALID_PARM);
    }

    // initialize calibration database access
    result = AdpfPrepareCalibDbAccess(pAdpfCtx,
                                      pConfig->data.db.hCamCalibDb,
                                      pConfig->data.db.width,
                                      pConfig->data.db.height,
                                      pConfig->data.db.framerate);
    if (result != RET_SUCCESS) {
      LOGV( "%s: Can't prepare database access\n",  __func__);
      return (result);
    }

    // get dpf-profile from calibration database
    result = CamCalibDbGetDpfProfileByResolution(pAdpfCtx->hCamCalibDb, pAdpfCtx->ResName, &pDpfProfile);
    if (result != RET_SUCCESS) {
      LOGV( "%s: Getting DPF profile for resolution %s from calibration database failed (%d)\n",
            __func__, pAdpfCtx->ResName, result);
      return (result);
    }
    DCT_ASSERT(NULL != pDpfProfile);

    // initialize Adpf context with values from calibration database
    pAdpfCtx->gain                  = pConfig->fSensorGain;
    pAdpfCtx->SigmaGreen            = pDpfProfile->SigmaGreen;
    pAdpfCtx->SigmaRedBlue          = pDpfProfile->SigmaRedBlue;
    pAdpfCtx->fGradient             = pDpfProfile->fGradient;
    pAdpfCtx->fOffset               = pDpfProfile->fOffset;
    pAdpfCtx->fMin                  = 2.0f;
    pAdpfCtx->fDiv                  = 64.0f;

    pAdpfCtx->NfGains.fRed          = pDpfProfile->NfGains.fCoeff[CAM_4CH_COLOR_COMPONENT_RED];
    pAdpfCtx->NfGains.fGreenR       = pDpfProfile->NfGains.fCoeff[CAM_4CH_COLOR_COMPONENT_GREENR];
    pAdpfCtx->NfGains.fGreenB       = pDpfProfile->NfGains.fCoeff[CAM_4CH_COLOR_COMPONENT_GREENB];
    pAdpfCtx->NfGains.fBlue         = pDpfProfile->NfGains.fCoeff[CAM_4CH_COLOR_COMPONENT_BLUE];
    pAdpfCtx->ADPFEnable = pDpfProfile->ADPFEnable;

	pAdpfCtx->LightMode = LIGHT_MODE_DAY;
	int no_filter=0;
	result = CamCalibDbGetNoOfFilterProfile(pAdpfCtx->hCamCalibDb, pDpfProfile, &no_filter);
	if (result != RET_SUCCESS) {
      LOGV( "%s: Getting number of filter profile for resolution %s from calibration database failed (%d)\n",
            __func__, pAdpfCtx->ResName, result);
      return (result);
    }

	if(no_filter){
		for(int i=0; i<no_filter && i<LIGHT_MODE_MAX ; i++){
			CamFilterProfile_t* pFilterProfile = NULL;
			result = CamCalibDbGetFilterProfileByIdx(pAdpfCtx->hCamCalibDb, pDpfProfile, i, &pFilterProfile);
			if (result != RET_SUCCESS) {
		      LOGV( "%s: Getting idx(%d) filter profile for resolution %s from calibration database failed (%d)\n",
		            __func__, i, pAdpfCtx->ResName, result);
		      return (result);
		    }
		    DCT_ASSERT(NULL != pFilterProfile);
			pAdpfCtx->FilterProfile[i] = *pFilterProfile;
		}
		pAdpfCtx->pFilterProfile = &(pAdpfCtx->FilterProfile[pAdpfCtx->LightMode]);
	}
	
	
    if (pAdpfCtx->pFilterProfile && pAdpfCtx->pFilterProfile->FilterEnable >= 1.0) {
      result = AdpfCalculateDenoiseLevel(pAdpfCtx, pAdpfCtx->gain, &pAdpfCtx->pFilterProfile->DenoiseLevelCurve, &pAdpfCtx->denoise_level);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
      result = AdpfCalculateSharpeningLevel(pAdpfCtx, pAdpfCtx->gain, &pAdpfCtx->pFilterProfile->SharpeningLevelCurve, &pAdpfCtx->sharp_level);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);	  
	  result = AdpfCalculateDemosaicThLevel(pAdpfCtx, pAdpfCtx->gain, &pAdpfCtx->pFilterProfile->DemosaicThCurve, &pAdpfCtx->demosaic_th);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
	  pAdpfCtx->FltEnable = BOOL_TRUE;
	  pAdpfCtx->actives |= ADPF_DENOISE_SHARP_LEVEL_MASK;
	  pAdpfCtx->actives |= ADPF_DEMOSAIC_TH_MASK;
	  
    } else{
	  pAdpfCtx->FltEnable = BOOL_FALSE;
	  result = AdpfCalculateDemosaicThLevel(pAdpfCtx, pAdpfCtx->gain, &pAdpfCtx->pFilterProfile->DemosaicThCurve, &pAdpfCtx->demosaic_th);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
	  pAdpfCtx->actives |= ADPF_DENOISE_SHARP_LEVEL_MASK;
	  pAdpfCtx->actives |= ADPF_DEMOSAIC_TH_MASK;
	}

	if (pAdpfCtx->pFilterProfile && pAdpfCtx->pFilterProfile->DemosaicLpConf.lp_en >= 1) {
		result = AdpfRKLpCalMatrix(pAdpfCtx, pAdpfCtx->gain, &(pAdpfCtx->pFilterProfile->DemosaicLpConf), &pAdpfCtx->RKDemosaicLpResult);
		RETURN_RESULT_IF_DIFFERENT( result, RET_SUCCESS );
		pAdpfCtx->RKDemosaicLpResult.lp_en = 1;
		pAdpfCtx->RKDemosaicLpResult.rb_filter_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.rb_filter_en;
		pAdpfCtx->RKDemosaicLpResult.hp_filter_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.hp_filter_en;
		pAdpfCtx->RKDemosaicLpResult.th_grad_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.th_grad_en;
		pAdpfCtx->RKDemosaicLpResult.th_diff_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.th_diff_en;
		pAdpfCtx->RKDemosaicLpResult.th_csc_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.th_csc_en;
		pAdpfCtx->RKDemosaicLpResult.th_var_en = pAdpfCtx->pFilterProfile->DemosaicLpConf.th_var_en;		
		pAdpfCtx->RKDemosaicLpResult.use_old_lp = pAdpfCtx->pFilterProfile->DemosaicLpConf.use_old_lp;
		MEMCPY(pAdpfCtx->RKDemosaicLpResult.lu_divided, pAdpfCtx->pFilterProfile->DemosaicLpConf.lu_divided, sizeof(pAdpfCtx->RKDemosaicLpResult.lu_divided));
		pAdpfCtx->actives |= ADPF_DEMOSAICLP_MASK;
	}else{
		pAdpfCtx->RKDemosaicLpResult.lp_en = 0;
		pAdpfCtx->RKDemosaicLpResult.rb_filter_en = 0;
		pAdpfCtx->RKDemosaicLpResult.hp_filter_en = 0;
		pAdpfCtx->actives |= ADPF_DEMOSAICLP_MASK;
	}
		
	int no_3dnr=0;
	result = CamCalibDbGetNoOfDsp3DNRSetting(pAdpfCtx->hCamCalibDb, pDpfProfile, &no_3dnr);
	if (result != RET_SUCCESS) {
      LOGV( "%s: Getting number of 3ndr profile for resolution %s from calibration database failed (%d)\n",
            __func__, pAdpfCtx->ResName, result);
      return (result);
    }
	
	if(no_3dnr){
		for(int i=0; i<no_3dnr && i<LIGHT_MODE_MAX ; i++){
			CamDsp3DNRSettingProfile_t* pDsp3dnrSetting = NULL;
			result = CamCalibDbGetDsp3DNRByIdx(pAdpfCtx->hCamCalibDb, pDpfProfile, i, &pDsp3dnrSetting);
			if (result != RET_SUCCESS) {
		      LOGV( "%s: Getting idx(%d)3ndr profile for resolution %s from calibration database failed (%d)\n",
		            __func__, i, pAdpfCtx->ResName, result);
		      return (result);
		    }
		    DCT_ASSERT(NULL != pDsp3dnrSetting);
			pAdpfCtx->Dsp3DNRSettingProfile[i] = *pDsp3dnrSetting;
		}
		pAdpfCtx->pDsp3DNRSettingProfile = &(pAdpfCtx->Dsp3DNRSettingProfile[pAdpfCtx->LightMode]);
	}
	
	if (pAdpfCtx->pDsp3DNRSettingProfile && pAdpfCtx->pDsp3DNRSettingProfile->Enable == 1) {
	  pAdpfCtx->Dsp3DnrResult.Enable = 1;
      pAdpfCtx->Dsp3DnrResult.luma_sp_nr_en = pAdpfCtx->pDsp3DNRSettingProfile->sDefaultLevelSetting.luma_sp_nr_en;
      pAdpfCtx->Dsp3DnrResult.luma_te_nr_en = pAdpfCtx->pDsp3DNRSettingProfile->sDefaultLevelSetting.luma_te_nr_en;
      pAdpfCtx->Dsp3DnrResult.chrm_sp_nr_en = pAdpfCtx->pDsp3DNRSettingProfile->sDefaultLevelSetting.chrm_sp_nr_en;
	  pAdpfCtx->Dsp3DnrResult.chrm_te_nr_en = pAdpfCtx->pDsp3DNRSettingProfile->sDefaultLevelSetting.chrm_te_nr_en;
	  pAdpfCtx->Dsp3DnrResult.shp_en = pAdpfCtx->pDsp3DNRSettingProfile->sDefaultLevelSetting.shp_en;
      result = AdpfCalculate3DNRResult(pAdpfCtx, pAdpfCtx->gain, pAdpfCtx->pDsp3DNRSettingProfile, &pAdpfCtx->Dsp3DnrResult);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
	  
	}else{
	  pAdpfCtx->Dsp3DnrResult.Enable = 0;
      pAdpfCtx->Dsp3DnrResult.luma_sp_nr_en = 0;
      pAdpfCtx->Dsp3DnrResult.luma_te_nr_en = 0;
      pAdpfCtx->Dsp3DnrResult.chrm_sp_nr_en = 0;
	  pAdpfCtx->Dsp3DnrResult.chrm_te_nr_en = 0;
	  pAdpfCtx->Dsp3DnrResult.shp_en = 0;
	}

	// new 3dnr para
	int no_new_3dnr=0;
	result = CamCalibDbGetNoOfNewDsp3DNRSetting(pAdpfCtx->hCamCalibDb, pDpfProfile, &no_new_3dnr);
	if (result != RET_SUCCESS) {
      LOGV(  "%s: Getting number of new 3ndr profile for resolution %s from calibration database failed (%d)\n",
            __func__, pAdpfCtx->ResName, result);
      return (result);
    }
	if(no_new_3dnr){
		for(int i=0; i<no_new_3dnr && i<LIGHT_MODE_MAX ; i++){
			CamNewDsp3DNRProfile_t* pNewDsp3dnrSetting = NULL;
			result = CamCalibDbGetNewDsp3DNRByIdx(pAdpfCtx->hCamCalibDb, pDpfProfile, i, &pNewDsp3dnrSetting);
			if (result != RET_SUCCESS) {
		      LOGV(  "%s: Getting idx(%d) new 3ndr profile for resolution %s from calibration database failed (%d)\n",
		            __func__, i, pAdpfCtx->ResName, result);
		      return (result);
		    }
		    DCT_ASSERT(NULL != pNewDsp3dnrSetting);
			pAdpfCtx->newDsp3DNRProfile[i] = *pNewDsp3dnrSetting;
		}
	}
	pAdpfCtx->pNew3DNRProfile = &(pAdpfCtx->newDsp3DNRProfile[pAdpfCtx->LightMode]);
	
	if (pAdpfCtx->pNew3DNRProfile->enable_3dnr == 1) {
	  pAdpfCtx->NewDsp3DnrResult.enable_3dnr = 1;
      pAdpfCtx->NewDsp3DnrResult.enable_dpc = pAdpfCtx->pNew3DNRProfile->enable_dpc;
      pAdpfCtx->NewDsp3DnrResult.enable_ynr = pAdpfCtx->pNew3DNRProfile->ynr.enable_ynr;
      pAdpfCtx->NewDsp3DnrResult.enable_tnr = pAdpfCtx->pNew3DNRProfile->ynr.enable_tnr;
	  pAdpfCtx->NewDsp3DnrResult.enable_iir = pAdpfCtx->pNew3DNRProfile->ynr.enable_iir;
	  pAdpfCtx->NewDsp3DnrResult.enable_uvnr = pAdpfCtx->pNew3DNRProfile->uvnr.enable_uvnr;
	  pAdpfCtx->NewDsp3DnrResult.enable_sharp = pAdpfCtx->pNew3DNRProfile->sharp.enable_sharp;
      result = AdpfCalculateNew3DNRResult(pAdpfCtx, pAdpfCtx->gain, pAdpfCtx->pNew3DNRProfile, &pAdpfCtx->NewDsp3DnrResult);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
	  
	}else{
	  pAdpfCtx->NewDsp3DnrResult.enable_3dnr = 0;
      pAdpfCtx->NewDsp3DnrResult.enable_dpc = 0;
      pAdpfCtx->NewDsp3DnrResult.enable_ynr = 0;
      pAdpfCtx->NewDsp3DnrResult.enable_tnr = 0;
	  pAdpfCtx->NewDsp3DnrResult.enable_iir = 0;
	  pAdpfCtx->NewDsp3DnrResult.enable_uvnr = 0;
	  pAdpfCtx->NewDsp3DnrResult.enable_sharp = 0;
	}
	
	CamIesharpenProfile_t *pRkIeSharpenProfile = NULL;
    result = CamCalibDbGetRKsharpenProfileByResolution( pAdpfCtx->hCamCalibDb, pAdpfCtx->ResName, &pRkIeSharpenProfile );
    if (pRkIeSharpenProfile != NULL) {
        pAdpfCtx->rkSharpenProfile = *pRkIeSharpenProfile;
    }
	if(pAdpfCtx->rkSharpenProfile.iesharpen_en){
        result = AdpfCalcRKIESharpFilterCoe(pAdpfCtx, pAdpfCtx->gain, &(pAdpfCtx->rkSharpenProfile), &pAdpfCtx->RKIESharpResult);
        RETURN_RESULT_IF_DIFFERENT( result, RET_SUCCESS );
		pAdpfCtx->RKIESharpResult.iesharpen_en = 1;
		pAdpfCtx->RKIESharpResult.coring_thr = pAdpfCtx->rkSharpenProfile.coring_thr;
		pAdpfCtx->RKIESharpResult.full_range = pAdpfCtx->rkSharpenProfile.full_range;
		pAdpfCtx->RKIESharpResult.switch_avg = pAdpfCtx->rkSharpenProfile.switch_avg;
		MEMCPY(pAdpfCtx->RKIESharpResult.yavg_thr, pAdpfCtx->rkSharpenProfile.yavg_thr, sizeof(pAdpfCtx->RKIESharpResult.yavg_thr));
		MEMCPY(pAdpfCtx->RKIESharpResult.delta1, pAdpfCtx->rkSharpenProfile.P_delta1, sizeof(pAdpfCtx->RKIESharpResult.delta1));
		MEMCPY(pAdpfCtx->RKIESharpResult.delta2, pAdpfCtx->rkSharpenProfile.P_delta2, sizeof(pAdpfCtx->RKIESharpResult.delta2));
		MEMCPY(pAdpfCtx->RKIESharpResult.maxnumber, pAdpfCtx->rkSharpenProfile.pmaxnumber, sizeof(pAdpfCtx->RKIESharpResult.maxnumber));
		MEMCPY(pAdpfCtx->RKIESharpResult.minnumber, pAdpfCtx->rkSharpenProfile.pminnumber, sizeof(pAdpfCtx->RKIESharpResult.minnumber));
		MEMCPY(pAdpfCtx->RKIESharpResult.gauss_flat_coe, pAdpfCtx->rkSharpenProfile.gauss_flat_coe, sizeof(pAdpfCtx->RKIESharpResult.gauss_flat_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.gauss_noise_coe, pAdpfCtx->rkSharpenProfile.gauss_noise_coe, sizeof(pAdpfCtx->RKIESharpResult.gauss_noise_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.gauss_other_coe, pAdpfCtx->rkSharpenProfile.gauss_other_coe, sizeof(pAdpfCtx->RKIESharpResult.gauss_other_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.uv_gauss_flat_coe, pAdpfCtx->rkSharpenProfile.uv_gauss_flat_coe, sizeof(pAdpfCtx->RKIESharpResult.uv_gauss_flat_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.uv_gauss_noise_coe, pAdpfCtx->rkSharpenProfile.uv_gauss_noise_coe, sizeof(pAdpfCtx->RKIESharpResult.uv_gauss_noise_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.uv_gauss_other_coe, pAdpfCtx->rkSharpenProfile.uv_gauss_other_coe, sizeof(pAdpfCtx->RKIESharpResult.uv_gauss_other_coe));
		MEMCPY(pAdpfCtx->RKIESharpResult.lap_mat_coe, pAdpfCtx->rkSharpenProfile.lgridconf.lap_mat_coe, sizeof(pAdpfCtx->RKIESharpResult.lap_mat_coe));
		pAdpfCtx->actives |= ADPF_RKIESHARP_MASK;
    }else{
		pAdpfCtx->RKIESharpResult.iesharpen_en = 0;
		pAdpfCtx->actives |= ADPF_RKIESHARP_MASK;
    }
	
    switch (pDpfProfile->nll_segmentation) {
      case 0U:
        pAdpfCtx->Nll.xScale = CAMERIC_NLL_SCALE_LINEAR;
        break;
      case 1U:
        pAdpfCtx->Nll.xScale = CAMERIC_NLL_SCALE_LOGARITHMIC;
        break;
      default:
        LOGV( "%s: NLL x-scale not supported (%d)\n", __func__, pDpfProfile->nll_segmentation);
        return (RET_OUTOFRANGE);
    }

    for (int32_t i = 0; i < CAMERIC_DPF_MAX_NLF_COEFFS; i++)
      pAdpfCtx->Nll.NllCoeff[i] = (pDpfProfile->nll_coeff.uCoeff[i] >> 2);

  } else if (pConfig->type == ADPF_USE_DEFAULT_CONFIG) {
    // initialize Adpf context with values from calibration database
    pAdpfCtx->gain                  = pConfig->fSensorGain;
    pAdpfCtx->SigmaGreen            = pConfig->data.def.SigmaGreen;
    pAdpfCtx->SigmaRedBlue          = pConfig->data.def.SigmaRedBlue;
    pAdpfCtx->fGradient             = pConfig->data.def.fGradient;
    pAdpfCtx->fOffset               = pConfig->data.def.fOffset;
    pAdpfCtx->fMin                  = pConfig->data.def.fMin;
    pAdpfCtx->fDiv                  = pConfig->data.def.fDiv;

    pAdpfCtx->NfGains.fRed          = pConfig->data.def.NfGains.fRed;
    pAdpfCtx->NfGains.fGreenR       = pConfig->data.def.NfGains.fGreenR;
    pAdpfCtx->NfGains.fGreenB       = pConfig->data.def.NfGains.fGreenB;
    pAdpfCtx->NfGains.fBlue         = pConfig->data.def.NfGains.fBlue;

    result = AdpfCalculateNllCoefficients(pAdpfCtx, pConfig->fSensorGain, &pAdpfCtx->Nll);
    if (result != RET_SUCCESS) {
	LOGV("%s: (enter) AdpfCalculateNllCoefficients failed\n", __func__);
      return (result);
    }
    int32_t i;
    for (i = 0; i < CAMERIC_DPF_MAX_NLF_COEFFS; i++)
      pAdpfCtx->Nll.NllCoeff[i] >>= 2;
  } else {
    LOGV( "%s: unsupported ADPF configuration\n",  __func__);
    return (RET_OUTOFRANGE);
  }

  // initialize CamerIc driver dpf-config
  pAdpfCtx->DpfMode.GainUsage             = CAMERIC_DPF_GAIN_USAGE_AWB_LSC_GAINS;
  pAdpfCtx->DpfMode.RBFilterSize          = CAMERIC_DPF_RB_FILTERSIZE_13x9;
  pAdpfCtx->DpfMode.ProcessRedPixel       = BOOL_TRUE;
  pAdpfCtx->DpfMode.ProcessGreenRPixel    = BOOL_TRUE;
  pAdpfCtx->DpfMode.ProcessGreenBPixel    = BOOL_TRUE;
  pAdpfCtx->DpfMode.ProcessBluePixel      = BOOL_TRUE;

  // compute initial spatial weights
  result = AdpfCalculateSpatialWeights(pAdpfCtx,
                                       pAdpfCtx->SigmaGreen,
                                       pAdpfCtx->SigmaRedBlue,
                                       &pAdpfCtx->DpfMode.SpatialG,
                                       &pAdpfCtx->DpfMode.SpatialRB);
  if (result != RET_SUCCESS) {
    LOGV( "%s: Initial calcultion of spatial weights failed (%d)\n",  __func__, result);
    return (result);
  }

  if (pAdpfCtx->ADPFEnable) {
    switch (pConfig->mode) {
      case ADPF_MODE_CONTROL_BY_GAIN:
        // caluclate init strength
        result = AdpfCalculateStrength(pAdpfCtx,
                                       pConfig->fSensorGain,
                                       pAdpfCtx->fGradient,
                                       pAdpfCtx->fOffset,
                                       pAdpfCtx->fMin,
                                       pAdpfCtx->fDiv,
                                       &pAdpfCtx->DynInvStrength);
        if (result != RET_SUCCESS) {
          LOGV("%s : AdpfCalculateStrength failed",
                __func__);
          return (result);
        }
        break;
      case ADPF_MODE_CONTROL_EXT:
        pAdpfCtx->DynInvStrength = pConfig->dynInvStrength;
        break;
      default:
        LOGV("%s: pConfig->mode: %d isn't support",
              __func__, pConfig->mode);
        break;
    }

    pAdpfCtx->actives |= (ADPF_MASK | ADPF_STRENGTH_MASK);
  }
  /* save configuration into context */
  pAdpfCtx->Config = *pConfig;

  LOGV( "%s: (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * Implementation of AEC API Functions
 *****************************************************************************/

/******************************************************************************
 * AdpfGains2CamerIcGains()
 *****************************************************************************/
RESULT AdpfGains2CamerIcGains
(
    AdpfGains_t*     pAdpfGains,
    CamerIcGains_t*  pCamerIcGains
) {
  RESULT result = RET_SUCCESS;

  LOGV( "%s: (enter)\n", __func__);

  if ((pAdpfGains != NULL) && (pCamerIcGains != NULL)) {
    if ((pAdpfGains->fRed > 15.995f)
        || (pAdpfGains->fBlue > 15.995f)
        || (pAdpfGains->fGreenR > 15.995f)
        || (pAdpfGains->fGreenB > 15.995f)) {
      result = RET_OUTOFRANGE;
    } else {
      pCamerIcGains->Red      = UtlFloatToFix_U0408(pAdpfGains->fRed);
      pCamerIcGains->GreenR   = UtlFloatToFix_U0408(pAdpfGains->fGreenR);
      pCamerIcGains->GreenB   = UtlFloatToFix_U0408(pAdpfGains->fGreenB);
      pCamerIcGains->Blue     = UtlFloatToFix_U0408(pAdpfGains->fBlue);
    }
  } else {
    result = RET_NULL_POINTER;
  }

  LOGV( "%s: (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamerIcGains2AdpfGains()
 *****************************************************************************/
RESULT CamerIcGains2AdpfGains
(
    CamerIcGains_t*  pCamerIcGains,
    AdpfGains_t*     pAdpfGains
) {
  RESULT result = RET_SUCCESS;

  LOGV( "%s: (enter)\n", __func__);

  if ((pAdpfGains != NULL) && (pCamerIcGains != NULL)) {
    pAdpfGains->fRed      = UtlFixToFloat_U0408(pCamerIcGains->Red);
    pAdpfGains->fGreenR   = UtlFixToFloat_U0408(pCamerIcGains->GreenR);
    pAdpfGains->fGreenB   = UtlFixToFloat_U0408(pCamerIcGains->GreenB);
    pAdpfGains->fBlue     = UtlFixToFloat_U0408(pCamerIcGains->Blue);
  } else {
    result = RET_NULL_POINTER;
  }

  LOGV( "%s: (exit %d)\n", __func__, result);

  return (result);
}



/******************************************************************************
 * AdpfInit()
 *****************************************************************************/
RESULT AdpfInit
(
    AdpfHandle_t* handlePtr,
    AdpfConfig_t* pConfig
) {
  RESULT result = RET_SUCCESS;

  AdpfContext_t* pAdpfCtx;

  LOGV( "%s: (enter)\n", __func__);

  /* allocate auto exposure control context */
  pAdpfCtx = (AdpfContext_t*)malloc(sizeof(AdpfContext_t));
  if (NULL == pAdpfCtx) {
    LOGV( "%s: Can't allocate ADPF context\n",  __func__);
    return (RET_OUTOFMEM);
  }

  /* pre-initialize context */
  MEMSET(pAdpfCtx, 0, sizeof(*pAdpfCtx));
  pAdpfCtx->state         = ADPF_STATE_INITIALIZED;

  /* return handle */
  *handlePtr = (AdpfHandle_t)pAdpfCtx;

  /* config adpf during initialize */
  AdpfConfigure(*handlePtr, pConfig);

  LOGV( "%s: (exit)\n", __func__);

  return (result);
}

/******************************************************************************
 * AdpfRelease()
 *****************************************************************************/
RESULT AdpfRelease
(
    AdpfHandle_t handle
) {
  AdpfContext_t* pAdpfCtx = (AdpfContext_t*)handle;

  LOGV( "%s: (enter)\n", __func__);

  /* initial checks */
  if (NULL == pAdpfCtx) {
    return (RET_WRONG_HANDLE);
  }

#if 0
  /* check state */
  if ((ADPF_STATE_RUNNING == pAdpfCtx->state) ||
      (ADPF_STATE_LOCKED == pAdpfCtx->state)) {
    return (RET_BUSY);
  }
#endif
  MEMSET(pAdpfCtx, 0, sizeof(AdpfContext_t));
  free(pAdpfCtx);

  LOGV( "%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * AdpfConfigure()
 *****************************************************************************/
RESULT AdpfConfigure
(
    AdpfHandle_t handle,
    AdpfConfig_t* pConfig
) {
  RESULT result = RET_SUCCESS;

  AdpfContext_t* pAdpfCtx = (AdpfContext_t*) handle;

  LOGV("%s: (enter)\n", __func__);

  /* initial checks */
  if (NULL == pAdpfCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pConfig) {
    return (RET_INVALID_PARM);
  }

  if (pConfig->mode == ADPF_MODE_INVALID)
    pConfig->mode = ADPF_MODE_CONTROL_BY_GAIN;
  if (pConfig->type == ADPF_USE_CALIB_INVALID)
    pConfig->type = ADPF_USE_CALIB_DATABASE;
  if (pConfig->fSensorGain == 0.0)
    pConfig->fSensorGain = 1.0f;

  if (memcmp(pConfig, &pAdpfCtx->Config, sizeof(AdpfConfig_t)))  {
    /* apply new configuration */
    result = AdpfApplyConfiguration(pAdpfCtx, pConfig);
    if (result != RET_SUCCESS) {
      LOGV( "%s: Can't configure CamerIc DPF (%d)\n",  __func__, result);
      return (result);
    }
  }

  LOGV( "%s: (exit)\n", __func__);
  return (result);
}

/******************************************************************************
 * AdpfProcessFrame()
 *****************************************************************************/
RESULT AdpfRun
(
    AdpfHandle_t    handle,
    const float     gain,
    enum LIGHT_MODE LightMode
) {
  AdpfContext_t* pAdpfCtx = (AdpfContext_t*)handle;

  RESULT result = RET_SUCCESS;

  float dgain = 0.0f; /* gain difference */

  LOGV( "%s: (enter)\n", __func__);

  /* initial checks */
  if (pAdpfCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  dgain = (gain > pAdpfCtx->gain) ? (gain - pAdpfCtx->gain) : (pAdpfCtx->gain - gain);
  if (pAdpfCtx->ADPFEnable) {
    if (pAdpfCtx->Config.mode == ADPF_MODE_CONTROL_BY_GAIN) {
      if (dgain > 0.15f) {
        /* caluclate new strength */
        result = AdpfCalculateStrength(pAdpfCtx, gain, pAdpfCtx->fGradient, pAdpfCtx->fOffset, pAdpfCtx->fMin, pAdpfCtx->fDiv, &pAdpfCtx->DynInvStrength);
        RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);

        pAdpfCtx->actives |= (ADPF_STRENGTH_MASK);
        LOGV( "%s: gain: %8.3f, %d, %d, %d", __func__,
              pAdpfCtx->gain,
              pAdpfCtx->DynInvStrength.WeightB,
              pAdpfCtx->DynInvStrength.WeightG,
              pAdpfCtx->DynInvStrength.WeightR);
      } else {
        result = RET_CANCELED;
      }
    }
  }

  if(LightMode <= LIGHT_MODE_MIN || LightMode >= LIGHT_MODE_MAX ){
	ALOGW( "%s: light mode %d is wrong, so use day mode instead\n",
         __func__, LightMode);
	LightMode = LIGHT_MODE_DAY;
  }
  pAdpfCtx->pFilterProfile = &(pAdpfCtx->FilterProfile[LightMode]);
  pAdpfCtx->pDsp3DNRSettingProfile = &(pAdpfCtx->Dsp3DNRSettingProfile[LightMode]);
  
  if (pAdpfCtx->pFilterProfile->FilterEnable >= 1.0) {
  	if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){  
      /*caluclate denoise level     */
      CamerIcIspFltDeNoiseLevel_t deNoiseLevel;
      result = AdpfCalculateDenoiseLevel(pAdpfCtx, gain, &pAdpfCtx->pFilterProfile->DenoiseLevelCurve, &deNoiseLevel);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
  
      /*calucate sharnping level    */
      CamerIcIspFltSharpeningLevel_t sharpningLevel;
      result = AdpfCalculateSharpeningLevel(pAdpfCtx, gain, &pAdpfCtx->pFilterProfile->SharpeningLevelCurve, &sharpningLevel);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);

	  uint8_t demosaic_th;
	  result = AdpfCalculateDemosaicThLevel(pAdpfCtx, gain, &pAdpfCtx->pFilterProfile->DemosaicThCurve, &demosaic_th);
      RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
  
      
      pAdpfCtx->actives |= ADPF_DENOISE_SHARP_LEVEL_MASK;
      pAdpfCtx->denoise_level = deNoiseLevel;
      pAdpfCtx->sharp_level = sharpningLevel;
	  pAdpfCtx->FltEnable = BOOL_TRUE;
     

	  if(pAdpfCtx->demosaic_th != demosaic_th){
		pAdpfCtx->actives |= ADPF_DEMOSAIC_TH_MASK;
		pAdpfCtx->demosaic_th = demosaic_th;
	  }
	}  
  }else{
  	if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
    	pAdpfCtx->actives |= ADPF_DENOISE_SHARP_LEVEL_MASK;
		pAdpfCtx->FltEnable = BOOL_FALSE;
  	}
  }


  if (pAdpfCtx->Dsp3DnrResult.Enable == 1) {
  	if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
	    result = AdpfCalculate3DNRResult(pAdpfCtx, gain, pAdpfCtx->pDsp3DNRSettingProfile, &pAdpfCtx->Dsp3DnrResult);
	    RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
	    pAdpfCtx->actives |= ADPF_DSP_3DNR_MASK;
	}
  }else{
	pAdpfCtx->Dsp3DnrResult.Enable = 0;
    pAdpfCtx->Dsp3DnrResult.luma_sp_nr_en = 0;
    pAdpfCtx->Dsp3DnrResult.luma_te_nr_en = 0;
    pAdpfCtx->Dsp3DnrResult.chrm_sp_nr_en = 0;
    pAdpfCtx->Dsp3DnrResult.chrm_te_nr_en = 0;
    pAdpfCtx->Dsp3DnrResult.shp_en = 0;
	pAdpfCtx->actives |= ADPF_DSP_3DNR_MASK;
  }

  
  //new 3dnr para calc
  if (pAdpfCtx->pNew3DNRProfile->enable_3dnr) {
	 if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
		 result = AdpfCalculateNew3DNRResult(pAdpfCtx, gain, pAdpfCtx->pNew3DNRProfile, &pAdpfCtx->NewDsp3DnrResult);
		 RETURN_RESULT_IF_DIFFERENT(result, RET_SUCCESS);
		 pAdpfCtx->actives |= ADPF_NEW_DSP_3DNR_MASK;
	 }
   }else{
	 pAdpfCtx->NewDsp3DnrResult.enable_3dnr = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_dpc = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_ynr = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_tnr = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_iir = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_uvnr = 0;
	 pAdpfCtx->NewDsp3DnrResult.enable_sharp = 0;
	 pAdpfCtx->actives |= ADPF_NEW_DSP_3DNR_MASK;
   }

  //rk demosaic LP 
  if (pAdpfCtx->pFilterProfile->DemosaicLpConf.lp_en == 1) {
  	if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
		result = AdpfRKLpCalMatrix(pAdpfCtx, gain, &(pAdpfCtx->pFilterProfile->DemosaicLpConf), &pAdpfCtx->RKDemosaicLpResult);
		RETURN_RESULT_IF_DIFFERENT( result, RET_SUCCESS );
		pAdpfCtx->actives |= ADPF_DEMOSAICLP_MASK;
	}
  }else{
	pAdpfCtx->RKDemosaicLpResult.lp_en = 0;
	pAdpfCtx->actives |= ADPF_DEMOSAICLP_MASK;
  }

  //rk IE sharpen
  if (pAdpfCtx->rkSharpenProfile.iesharpen_en == 1) {
  	if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
		result = AdpfCalcRKIESharpFilterCoe(pAdpfCtx, gain, &(pAdpfCtx->rkSharpenProfile), &pAdpfCtx->RKIESharpResult);
        RETURN_RESULT_IF_DIFFERENT( result, RET_SUCCESS );
		pAdpfCtx->actives |= ADPF_RKIESHARP_MASK;
	}
  }else{
	pAdpfCtx->RKIESharpResult.iesharpen_en = 0;
	pAdpfCtx->actives |= ADPF_RKIESHARP_MASK;
  }
  
  if (dgain > 0.15f || pAdpfCtx->LightMode != LightMode){
  	pAdpfCtx->gain = gain;
	pAdpfCtx->LightMode = LightMode;
  }
  LOGV( "%s: (exit)\n", __func__);

  return (result);
}

RESULT AdpfGetResult
(
    AdpfHandle_t    handle,
    AdpfResult_t*  result
) {
  AdpfContext_t* pAdpfCtx = (AdpfContext_t*)handle;

  if (pAdpfCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  if (pAdpfCtx->actives & ADPF_MASK) {
    result->DpfMode = pAdpfCtx->DpfMode;
    AdpfGains2CamerIcGains(
        &pAdpfCtx->NfGains,
        &result->NfGains);
    result->Nll = pAdpfCtx->Nll;
    result->actives |= ADPF_MASK;
    //pAdpfCtx->actives &= ~ADPF_MASK;
	LOGV( "%s: oyyf gain(%f) dpf mode(%d) nll(%d)\n", __func__, 
		pAdpfCtx->gain,pAdpfCtx->DpfMode, pAdpfCtx->Nll.NllCoeff[0]);
  }

  if (pAdpfCtx->actives & ADPF_STRENGTH_MASK) {
    result->DynInvStrength = pAdpfCtx->DynInvStrength;
    result->actives |= ADPF_STRENGTH_MASK;
    //pAdpfCtx->actives &= ~ADPF_STRENGTH_MASK;
  }

  if (pAdpfCtx->actives & ADPF_DENOISE_SHARP_LEVEL_MASK) 
  {
    result->denoise_level = pAdpfCtx->denoise_level;
    result->sharp_level = pAdpfCtx->sharp_level;
    result->actives |= ADPF_DENOISE_SHARP_LEVEL_MASK;
    //pAdpfCtx->actives &= ~ADPF_DENOISE_SHARP_LEVEL_MASK;
	result->FltEnable = pAdpfCtx->FltEnable;
    LOGV( "%s: gain(%f) denoise(%d) sharp(%d)\n", __func__, 
		pAdpfCtx->gain, result->denoise_level, result->sharp_level);
  }

  if(pAdpfCtx->actives & ADPF_DEMOSAIC_TH_MASK) 
  {
	result->demosaic_th = pAdpfCtx->demosaic_th;
	result->actives |= ADPF_DEMOSAIC_TH_MASK;
    //pAdpfCtx->actives &= ~ADPF_DEMOSAIC_TH_MASK;	
	LOGV( "%s: gain(%f) demosaic_th(%d)\n", __func__, 
		pAdpfCtx->gain, result->demosaic_th);
  }

  if(pAdpfCtx->actives & ADPF_DEMOSAICLP_MASK) 
  {
	result->RKDemosaicLpResult = pAdpfCtx->RKDemosaicLpResult;
	result->actives |= ADPF_DEMOSAICLP_MASK;
    //pAdpfCtx->actives &= ~ADPF_DEMOSAIC_TH_MASK;	
	LOGV( "%s: gain(%f) demosaiclp_en(%d) thgrad: %d, %d, %d, %d, %d\n", __func__, 
		pAdpfCtx->gain, result->RKDemosaicLpResult.lp_en, 
		result->RKDemosaicLpResult.thgrad_divided[0],
		result->RKDemosaicLpResult.thgrad_divided[1],
		result->RKDemosaicLpResult.thgrad_divided[2],
		result->RKDemosaicLpResult.thgrad_divided[3],
		result->RKDemosaicLpResult.thgrad_divided[4]);
  }

  if(pAdpfCtx->actives & ADPF_RKIESHARP_MASK) 
  {
	result->RKIESharpResult = pAdpfCtx->RKIESharpResult;
	result->actives |= ADPF_RKIESHARP_MASK;
    //pAdpfCtx->actives &= ~ADPF_DEMOSAIC_TH_MASK;	
	LOGV( "%s: gain(%f) rkiesharp_en(%d) thgrad: %d, %d, %d, %d, %d, %d\n", __func__, 
		pAdpfCtx->gain, result->RKIESharpResult.iesharpen_en, 
		result->RKIESharpResult.line1_filter_coe[0],
		result->RKIESharpResult.line1_filter_coe[1],
		result->RKIESharpResult.line1_filter_coe[2],
		result->RKIESharpResult.line1_filter_coe[3],
		result->RKIESharpResult.line1_filter_coe[4],
		result->RKIESharpResult.line1_filter_coe[5]);
  }
  
  //if (pAdpfCtx->actives & ADPF_DSP_3DNR_MASK) 
  {
	  result->Dsp3DnrResult = pAdpfCtx->Dsp3DnrResult;
	  result->actives |= ADPF_DSP_3DNR_MASK;
	  pAdpfCtx->actives &= ~ADPF_DSP_3DNR_MASK;
	 LOGV( "%s: oyyf gain=%f, luma_sp:%d luma_te:%d chrm_sp:%d chrm_te:%d shp:%d noise:(%d/%d)\n", 
  		__func__, pAdpfCtx->gain, 
        result->Dsp3DnrResult.luma_sp_nr_level, result->Dsp3DnrResult.luma_te_nr_level, 
        result->Dsp3DnrResult.chrm_sp_nr_level, result->Dsp3DnrResult.chrm_te_nr_level, result->Dsp3DnrResult.shp_level,
        result->Dsp3DnrResult.noise_coef_num, result->Dsp3DnrResult.noise_coef_den);

	 LOGV( "oyyf setting luma:%d %d %d 0x%x 0x%x 0x%x 0x%x 0x%x",
              result->Dsp3DnrResult.luma_default,
              result->Dsp3DnrResult.luma_sp_rad,
              result->Dsp3DnrResult.luma_te_max_bi_num,
              result->Dsp3DnrResult.luma_w0,
              result->Dsp3DnrResult.luma_w1,
              result->Dsp3DnrResult.luma_w2,
              result->Dsp3DnrResult.luma_w3,
              result->Dsp3DnrResult.luma_w4);
	 
	 LOGV( "oyyf setting chrm:%d %d %d 0x%x 0x%x 0x%x 0x%x 0x%x",
              result->Dsp3DnrResult.chrm_default,
              result->Dsp3DnrResult.chrm_sp_rad,
              result->Dsp3DnrResult.chrm_te_max_bi_num,
              result->Dsp3DnrResult.chrm_w0,
              result->Dsp3DnrResult.chrm_w1,
              result->Dsp3DnrResult.chrm_w2,
              result->Dsp3DnrResult.chrm_w3,
              result->Dsp3DnrResult.chrm_w4);
	 
	 LOGV( "oyyf setting sharp:%d %d %d %d %d 0x%x 0x%x 0x%x 0x%x 0x%x",
              result->Dsp3DnrResult.shp_default,
              result->Dsp3DnrResult.src_shp_thr,
              result->Dsp3DnrResult.src_shp_div,
              result->Dsp3DnrResult.src_shp_l,
              result->Dsp3DnrResult.src_shp_c,            
              result->Dsp3DnrResult.src_shp_w0,
              result->Dsp3DnrResult.src_shp_w1,
              result->Dsp3DnrResult.src_shp_w2,
              result->Dsp3DnrResult.src_shp_w3,
              result->Dsp3DnrResult.src_shp_w4);
	 
  }

  //if (pAdpfCtx->actives & ADPF_NEW_DSP_3DNR_MASK) 
  {
	  result->NewDsp3DnrResult = pAdpfCtx->NewDsp3DnrResult;
	  result->actives |= ADPF_NEW_DSP_3DNR_MASK;
	  pAdpfCtx->actives &= ~ADPF_NEW_DSP_3DNR_MASK;
	  LOGV( "%s: gain=%f, ynr_time_weight:%d ynr_spat_weight:%d uvnr_weight:%d sharp_weight:%d\n", 
  		__func__, pAdpfCtx->gain,
  		result->NewDsp3DnrResult.ynr_time_weight,
		result->NewDsp3DnrResult.ynr_spat_weight,
		result->NewDsp3DnrResult.uvnr_weight,
		result->NewDsp3DnrResult.sharp_weight);
	 
  }
  
  return RET_SUCCESS;
}


