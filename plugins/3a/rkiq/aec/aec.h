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
#ifndef __AEC_H__
#define __AEC_H__

/**
 * @file aec.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup AECM Auto white Balance Module
 * @{
 *
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>
#include <common/return_codes.h>
#include <common/misc.h>
#include <common/cam_types.h>
#include <cam_ia_api/cameric.h>
#include "base/log.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AEC_AFPS_MASK (1 << 0)

/*****************************************************************************/
/**
 * @brief   The number of mean and hist.
 */
/*****************************************************************************/
#define AEC_AE_MEAN_MAX 81
#define AEC_HIST_BIN_N_MAX 32

typedef enum AecMeasuringMode_e {
  AEC_MEASURING_MODE_INVALID    = 0,    /**< invalid histogram measuring mode   */
  AEC_MEASURING_MODE_1          = 1,    /**< Y = (R + G + B) x (85/256)         */
  AEC_MEASURING_MODE_2          = 2,    /**< Y = 16 + 0.25R + 0.5G + 0.1094B    */
  AEC_MEASURING_MODE_MAX,
} AecMeasuringMode_t;

/*****************************************************************************/
/**
 *          AecDampingMode_t
 *
 * @brief   mode type of AEC Damping
 *
 */
/*****************************************************************************/
typedef enum AecDampingMode_e {
  AEC_DAMPING_MODE_INVALID        = 0,        /* invalid (only used for initialization) */
  AEC_DAMPING_MODE_STILL_IMAGE    = 1,        /* damping mode still image */
  AEC_DAMPING_MODE_VIDEO          = 2,        /* damping mode video */
  AEC_DAMPING_MODE_MAX
} AecDampingMode_t;


/*****************************************************************************/
/**
 *          AecEcmMode_t
 *
 * @brief   mode type of AEC Exposure Conversion
 *
 */
/*****************************************************************************/
typedef enum AecEcmMode_e {
  AEC_EXPOSURE_CONVERSION_INVALID = 0,        /* invalid (only used for initialization) */
  AEC_EXPOSURE_CONVERSION_LINEAR  = 1,         /* Exposure Conversion uses a linear function (eq. 38) */
  AEC_EXPOSURE_CONVERSION_MAX
} AecEcmMode_t;


/*****************************************************************************/
/**
 *          AecEcmFlickerPeriod_t
 *
 * @brief   flicker period types for the AEC algorithm
 *
 */
/*****************************************************************************/
typedef enum AecEcmFlickerPeriod_e {
  AEC_EXPOSURE_CONVERSION_FLICKER_OFF   = 0x00,
  AEC_EXPOSURE_CONVERSION_FLICKER_100HZ = 0x01,
  AEC_EXPOSURE_CONVERSION_FLICKER_120HZ = 0x02
} AecEcmFlickerPeriod_t;

typedef struct Aec_daynight_th_s {
  float         gain_th;
  float         time_th;
  float         luma_th;
  uint8_t         holdon_times_th;
} Aec_daynight_th_t;

typedef struct AecInterAdjust_s{
	uint8_t enable;
	float	dluma_high_th;
	float	dluma_low_th;
	uint32_t  trigger_frame;
}AecInterAdjust_t;

/*****************************************************************************/
/**
 *          AecConfig_t
 *
 * @brief   AEC Module configuration structure; used for re-configuration as well
 *
 *****************************************************************************/
typedef struct AecConfig_s {

  Cam9x9UCharMatrix_t         GridWeights;
  CamerIcIspHistMode_t  HistMode;
  AecMeasuringMode_t    meas_mode;

  float                       SetPoint;                   /**< set point to hit by the ae control system */
  float                       ClmTolerance;
  float                       DampOverStill;              /**< damping coefficient for still image mode */
  float                       DampUnderStill;             /**< damping coefficient for still image mode */
  float                       DampOverVideo;              /**< damping coefficient for video mode */
  float                       DampUnderVideo;             /**< damping coefficient for video mode */
  Cam6x1FloatMatrix_t         EcmTimeDot;
  Cam6x1FloatMatrix_t         EcmGainDot;
  Cam6x1FloatMatrix_t         FpsFixTimeDot;
  uint8_t				      isFpsFix;
  uint8_t				      FpsSetEnable;
  AecDampingMode_t            DampingMode;              /**< damping mode */
  AecSemMode_t                SemMode;                  /**< scene evaluation mode */
  AecEcmFlickerPeriod_t       EcmFlickerSelect;         /**< flicker period selection */
  uint8_t                         StepSize;
  float                           GainFactor;
  float                           GainBias;
  float                           LinePeriodsPerField;
  float                           PixelClockFreqMHZ;
  float                           PixelPeriodsPerLine;
  /* gain range */

  uint32_t				  GainRange_size;
  float               *pGainRange;
  float               TimeFactor[4];

  float         AOE_Enable;
  float         AOE_Max_point;
  float         AOE_Min_point;
  float         AOE_Y_Max_th;
  float         AOE_Y_Min_th;
  float         AOE_Step_Inc;
  float         AOE_Step_Dec;
  uint8_t         DON_Enable;
  float         DON_Day2Night_Gain_th;
  float         DON_Day2Night_Inttime_th;
  float         DON_Day2Night_Luma_th;
  float         DON_Night2Day_Gain_th;
  float         DON_Night2Day_Inttime_th;
  float         DON_Night2Day_Luma_th;
  uint8_t         DON_Bouncing_th;

  AecInterAdjust_t IntervalAdjStgy;
  uint8_t       Valid_GridWeights_Num;
  uint8_t       Valid_GridWeights_W; /* H has the same dimension as W */
  uint8_t       Valid_HistBins_Num;
} AecConfig_t;

/*****************************************************************************/
/**
 *          Aec_stat_t
 *
 * @brief   AEC Module Hardware statistics structure
 *
 *****************************************************************************/
typedef struct AecStat_s {
  unsigned char  exp_mean[AEC_AE_MEAN_MAX];
  unsigned int   hist_bins[AEC_HIST_BIN_N_MAX];
} AecStat_t;


struct AecDyCfg {
  AecEcmFlickerPeriod_t  flicker;
  struct Cam_Win win;
};

/*****************************************************************************/
/**
*     AecResult_t
 * @brief   Aec_Result.
 */
/*****************************************************************************/
typedef struct AecResult_s {
  float coarse_integration_time;
  float analog_gain_code_global;
  int regIntegrationTime;
  int regGain;
  float PixelClockFreqMHZ;
  float PixelPeriodsPerLine;
  float LinePeriodsPerField;

  AecMeasuringMode_t meas_mode;
  struct Cam_Win meas_win;
  unsigned int actives;
  unsigned char GridWeights[AEC_AE_MEAN_MAX];
  uint8_t stepSize;
  CamerIcIspHistMode_t HistMode;
  float gainFactor;
  float gainBias;
  bool_t aoe_enable;
  bool_t night;
  bool auto_adjust_fps;
  double aperture_fn;
  bool converged;
} AecResult_t;

// aec ctrl
typedef enum AecState_e {
  AEC_STATE_INVALID       = 0,
  AEC_STATE_INITIALIZED   = 1,
  AEC_STATE_STOPPED       = 2,
  AEC_STATE_RUNNING       = 3,
  AEC_STATE_LOCKED        = 4,
  AEC_STATE_MAX
} AecState_t;


/*****************************************************************************/
/**
 * @brief   This typedef represents the histogram which is measured by the
 *          CamerIC ISP histogram module.
 *
 *****************************************************************************/
typedef uint32_t CamerIcHistBins_t[AEC_HIST_BIN_N_MAX];

/*****************************************************************************/
/**
 * @brief   Array type for ISP EXP measurment values.
 *
 *****************************************************************************/
typedef uint8_t CamerIcMeanLuma_t[AEC_AE_MEAN_MAX];


typedef struct AecContext_s {
  AecState_t          state;

  float               StartExposure;      /**< start exposure value of the ae control system */
  float               Exposure;           /**< current exposure value of the ae control system */
  float               MinExposure;        /**< minmal exposure value (sensor specific) */
  float               MaxExposure;        /**< maximal exposure value (sensor specific) */

  float               MinGain;            /**< minmal exposure value (sensor specific) */
  float               MaxGain;            /**< maximal exposure value (sensor specific) */

  float               MinIntegrationTime; /**< minmal exposure value (sensor specific) */
  float               MaxIntegrationTime; /**< maximal exposure value (sensor specific) */

  /* scene evaluation module */
  AecSemMode_t        SemMode;            /**< scene evaluation mode */

  float               SetPoint;           /**< set point to hit by the ae control system */
  float               SemSetPoint;        /**< set point calculated by scene evaluation module */

  AecDampingMode_t    DampingMode;        /**< damping mode */
  float               DampOverStill;      /**< Damping coefficient for still image mode */
  float               DampUnderStill;     /**< Damping coefficient for still image mode */
  float               DampOverVideo;      /**< Damping coefficient for video mode */
  float               DampUnderVideo;     /**< Damping coefficient for video mode */

  float               MeanLuma;           /**< mean luminace calculated */
  float               MeanLumaObject;
  float               *MeanLumas;
  uint32_t         MeanLumaIndex;
  uint32_t         AecTrigFrames;

  float               d;
  float               z;
  float               m0;

  float               EcmOldAlpha;        /**< for ECM lock range >*/
  float               EcmOldGain;         /**< for ECM lock range >*/
  float               EcmOldTint;         /**< for ECM lock range >*/

  union SemCtx_u {
    struct sem_s {
      float MeanLumaRegion0;
      float MeanLumaRegion1;
      float MeanLumaRegion2;
      float MeanLumaRegion3;
      float MeanLumaRegion4;
    } sem;

    struct asem_s {
      float               OtsuThreshold;  /**< threshold calculated with otsu */

      CamerIcMeanLuma_t   Binary0;        /**< thresholded luma matrix */
      CamerIcMeanLuma_t   Cc0;            /**< matrix of labeled 0-conponents */
      CamerIcMeanLuma_t   Ll0;            /**< matrix of labeled 1-conponents */
      uint32_t            NumCc0;         /**< number of 0-conponents */
      uint32_t            LblBiggestCc0;  /**< label of biggest 0-connected-component */
      float               MaxCenterX0;
      float               MaxCenterY0;

      CamerIcMeanLuma_t   Binary1;        /**< thresholded luma matrix */
      CamerIcMeanLuma_t   Cc1;            /**< matrix of labeled 1-conponents */
      CamerIcMeanLuma_t   Ll1;            /**< matrix of labeled 1-conponents */
      uint32_t            NumCc1;         /**< number of 1-conponents */
      uint32_t            LblBiggestCc1;  /**< label of biggest 1-connected-component */
      float               MaxCenterX1;
      float               MaxCenterY1;

      CamerIcMeanLuma_t   ObjectRegion;   /**< resulting matrix  (0 = Object, 1 = Background ) */
    } asem;

  } SemCtx;

  /* control loop module */
#define CLM_HIST_NUM_BINS   ( 3 * AEC_HIST_BIN_N_MAX )

  float               ClmTolerance;
  float               LumaDeviation;
  uint32_t            ClmHistogram[CLM_HIST_NUM_BINS];
  uint32_t            ClmHistogramSize;
  float               ClmHistogramX[CLM_HIST_NUM_BINS];
  uint32_t            SumHistogram;
  float               MeanHistogram;

  /* debug output */
  CamerIcHistBins_t   Histogram;
  CamerIcMeanLuma_t   Luma;
  unsigned char        GridWeights[AEC_AE_MEAN_MAX];
  CamerIcIspHistMode_t HistMode;
  AecMeasuringMode_t   meas_mode;

  /* ECM module (incl. AFPS) */
#define ECM_DOT_NO         (6)
  float               EcmTimeDot[ECM_DOT_NO];
  float               EcmGainDot[ECM_DOT_NO];
  float               EcmVtsDot[ECM_DOT_NO];
  AecEcmFlickerPeriod_t EcmFlickerSelect;
  float               EcmTflicker;        /**< flicker period */
  float         EcmLockRange;

  float               Gain;
  float               IntegrationTime;
  uint8_t             StepSize;
  float               GainFactor;
  float               GainBias;
  uint32_t            curFrameId;
  float               LinePeriodsPerField;
  float               PixelClockFreqMHZ;
  float               PixelPeriodsPerLine;
  int         regIntegrationTime;
  int         regGain;


  /* gain range */
  uint32_t					GainRange_size;
  float               *pGainRange;
  float               TimeFactor[4];

  float         AOE_Enable;
  float         AOE_Max_point;
  float         AOE_Min_point;
  float         AOE_Y_Max_th;
  float         AOE_Y_Min_th;
  float         AOE_Step_Inc;
  float         AOE_Step_Dec;
  /* curent state */
  bool_t          night;
  /* last state */
  bool_t                  night_last;
  uint8_t                 DON_Enable;
  uint32_t                ae_daynight_holdon_times;
  Aec_daynight_th_t       day2nitht_th;
  Aec_daynight_th_t       night2day_th;

  unsigned int actives;
  bool auto_adjust_fps;

  AecInterAdjust_t IntervalAdjStgy;
  bool converged;
  uint8_t       Valid_GridWeights_Num;
  uint8_t       Valid_GridWeights_W; /* H has the same dimension as W */
  uint8_t       Valid_HistBins_Num;
} AecContext_t;

//end of aec ctrl

/*****************************************************************************/
/**
 * @brief   This function init the AEC instance.
 *
 * @param   AecConfig
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
RESULT AecInit
(
    AecContext_t** pAecCtx,
    AecConfig_t* AecConfig
);

RESULT AecUpdateConfig
(
    AecContext_t* pAecCtx,
    AecConfig_t*         pConfig
);

RESULT AecStart
(
    AecContext_t* pAecCtx
);

RESULT AecStop
(
    AecContext_t* pAecCtx
);

/*****************************************************************************/
/**
 * @brief   This function single run the AEC instance.
 *
 * @param   Aec_stat_t
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
RESULT AecRun
(
    AecContext_t* pAecCtx,
    AecStat_t* ae_stat,
    AecResult_t* AecResult
);

/*****************************************************************************/
/**
 * @brief   This function get the AEC result.
 *
 * @param
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
RESULT AecGetResults
(
    AecContext_t* pAecCtx,
    AecResult_t* AecResult
);


/*****************************************************************************/
/**
 * @brief   This function release the AEC instance.
 *
 * @param   AecConfig
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_OUTOFRANGE
 *
 *****************************************************************************/
RESULT AecRelease
(
    AecContext_t* pAecCtx
);


#ifdef __cplusplus
}
#endif

/* @} AECM */


#endif /* __AEC_H__*/
