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
 * @defgroup AECM Auto Exposure Control Module
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

/*
 ***************** AE LIB VERSION NOTE *****************
 * v0.0.1
 *  - platform independence, support rkisp v10 and v12
 * v0.0.2
 *  - support rk1608 HDR ae
 *  - remove unnecessary lib dependancy
 *  - do not expose the head file aec_ctrl.h
 * v0.0.3
 *  - sync rkisp aec demo with calibdb v0.2.0
 * v0.0.4
 *  - support manual ae mode
 * v0.0.5
 *  - add DarkROI detection in Hdr AE
 *  - fix some bugs of ratio calculation
 *  - add ExpDot & Setpoint for Hdr AE
 * v0.0.6
 *  - run ae every frame
 *  - split each exposure result to 3 steps, make ae
 *    converge more smooth
 * v0.0.7
 *  - fix always report non-converged AE state in dark enviroment
 * v0.0.8
 *  - fix the min intergration time of 4[H] exposure lines
 * v0.0.9
 *  - enable ALOGV/ALOGW for Android 
 */

#define CONFIG_AE_LIB_VERSION "v0.0.9"

#ifdef __cplusplus
extern "C"
{
#endif

#define AEC_AFPS_MASK (1 << 0)
struct AecContext_s;
typedef struct AecContext_s AecContext_t;
/*****************************************************************************/
/**
 * @brief   The number of mean and hist.
 */
/*****************************************************************************/
#define AEC_AE_MEAN_MAX 81
#define AEC_HIST_BIN_N_MAX 32
#define AEC_HDR_AE_MEAN_MAX 225 //zlj modify for hdr
#define AEC_HDR_HIST_BIN_N_MAX 256 //zlj modify for hdr
/*zlj add*/
#define LockAE_NO	(3)
#define HDRAE_LOGHISTBIN_NUM 100

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
  float         fac_th;
  uint8_t         holdon_times_th;
} Aec_daynight_th_t;


typedef struct AecInterAdjust_s{
	uint8_t enable;
	float	dluma_high_th;
	float	dluma_low_th;
	uint32_t  trigger_frame;
}AecInterAdjust_t;

typedef enum AecMode_e {
  AEC_MODE_INVALID                    = 0,
  AEC_MODE_MANUAL                     = 1,
  AEC_MODE_AUTO                       = 2,
  AEC_MODE_MAX
} AecMode_t;

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
  AecMode_t             AecMode;
  float                 ManExpoSecs;
  float                 ManGains;
  float                       SetPoint;                   /**< set point to hit by the ae control system */
  float                       ClmTolerance;
  float                       DampOverStill;              /**< damping coefficient for still image mode */
  float                       DampUnderStill;             /**< damping coefficient for still image mode */
  float                       DampOverVideo;              /**< damping coefficient for video mode */
  float                       DampUnderVideo;             /**< damping coefficient for video mode */
  Cam6x1FloatMatrix_t         EcmTimeDot;
  Cam6x1FloatMatrix_t         EcmLTimeDot; //zlj add for Hdr
  Cam6x1FloatMatrix_t         EcmSTimeDot;//zlj add for Hdr
  Cam6x1FloatMatrix_t         EcmGainDot;
  Cam6x1FloatMatrix_t         EcmLGainDot; //zlj add for Hdr
  Cam6x1FloatMatrix_t         EcmSGainDot;//zlj add for Hdr
  Cam6x1FloatMatrix_t         FpsFixTimeDot;
  uint8_t				      isFpsFix;
  uint8_t				      FpsSetEnable;
  float                       MinFrameDuration;         /**< minmal frame duration time (for vts control) */
  float                       MaxFrameDuration;         /**< maximal frame duration time (for vts control) */
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

  uint8_t       DON_Night_Trigger;
  uint8_t       DON_Night_Mode;
  float         DON_Day2Night_Fac_th; // yamasaki
  float         DON_Night2Day_Fac_th; // yamasaki
  uint8_t       DON_Bouncing_th;

  AecInterAdjust_t IntervalAdjStgy;
  uint8_t       Valid_GridWeights_Num;
  uint8_t       Valid_GridWeights_W; /* H has the same dimension as W */
  uint8_t       Valid_HistBins_Num;
  //zlj add for HdrAec GridWeights
  uint8_t       Valid_HdrGridWeights_Num;
  uint8_t       Valid_HdrGridWeights_W; /* H has the same dimension as W */
  uint16_t       Valid_HdrHistBins_Num;

  //zlj add for LockAE
  uint8_t		LockAE_enable;
  Cam3x1FloatMatrix_t GainValue;
  Cam3x1FloatMatrix_t TimeValue;
  //zlj add for HdrCtrl
  CamCalibAecHdrCtrl_t HdrCtrl;
  bool          IsHdrAeMode;

  //oyyf
  enum LIGHT_MODE LightMode;
  CamCalibAecExpSeparate_t *pExpSeparate[LIGHT_MODE_MAX];
  CamCalibAecDynamicSetpoint_t *pDySetpoint[LIGHT_MODE_MAX];
} AecConfig_t;

/*****************************************************************************/
/**
 *          Aec_stat_t
 *
 * @brief   AEC Module Hardware statistics structure
 *
 *****************************************************************************/
/*zlj add*/
typedef struct Hdrae_DRIndex_res
{
	uint32_t NormalIndex;
	uint32_t LongIndex;
}Hdrae_DRIndex_res_t;
typedef struct Hdrae_OE_meas_res
{
	uint32_t OE_Pixel;
	uint32_t SumHistPixel;
	uint32_t SframeMaxLuma;
}Hdrae_OE_meas_res_t;
typedef struct Hdrae_stat_s
{
    unsigned int hdr_hist_bins[AEC_HDR_HIST_BIN_N_MAX];
    unsigned short hdr_exp_mean[AEC_HDR_AE_MEAN_MAX];
}Hdrae_stat_t;

typedef struct Hdr_sensor_metadata_s {
	unsigned int exp_time_l;
	unsigned int exp_time;
	unsigned int exp_time_s;
	unsigned int gain_l;
	unsigned int gain;
	unsigned int gain_s;
}Hdr_sensor_metadata_t;

typedef struct Sensor_metadata_s {
    float coarse_integration_time;
    float analog_gain_code_global;
    float LinePeriodsPerField;
    int regIntegrationTime;
    int regGain;
} Sensor_metadata_t;

typedef struct AecStat_s {
  unsigned char  exp_mean[AEC_AE_MEAN_MAX];
  unsigned int   hist_bins[AEC_HIST_BIN_N_MAX];
  Sensor_metadata_t sensor_metadata;
  /*zlj add*/
  bool   is_hdr_stats;
  struct Hdrae_stat_s oneframe[3];
  struct Hdrae_DRIndex_res fDRIndex;
  struct Hdrae_OE_meas_res fOEMeasRes;
  struct Hdr_sensor_metadata_s sensor;
  unsigned int lgmean;
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
  Sensor_metadata_t exp_smooth_results[3];
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
  AecMode_t AecMode;

  /*zlj add for hdr result*/
  bool IsHdrExp;
  float DCG_Ratio;
  float NormalExpRatio;
  float LongExpRatio;
  int RegHdrGains[3];
  int RegHdrTime[3];
  float HdrGains[3];
  float HdrIntTimes[3];
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
    AecConfig_t*         pConfig,
    bool          isReconfig
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
