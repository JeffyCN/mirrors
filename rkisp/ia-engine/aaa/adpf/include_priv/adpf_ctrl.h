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
#ifndef __ADPF_CTRL_H__
#define __ADPF_CTRL_H__

/**
 * @file adpf_ctrl.h
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
 * @defgroup ADPF Auto Denoising Pre-Filter
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>


#include "adpf.h"

#ifdef __cplusplus
extern "C"
{
#endif



/*****************************************************************************/
/**
 * @brief   This enum type specifies the different possible states of
 *          the ADPF module.
 */
/*****************************************************************************/
typedef enum AdpfState_e {
  ADPF_STATE_INVALID           = 0,                   /**< initialization value */
  ADPF_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
  ADPF_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
  ADPF_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
  ADPF_STATE_LOCKED            = 4,                   /**< instance is locked (for taking snapshots) */
  ADPF_STATE_MAX                                      /**< max */
} AdpfState_t;



/*****************************************************************************/
/**
 * @brief   Context of the ADPF module.
 */
/*****************************************************************************/
typedef struct AdpfContext_s {
  AdpfState_t                     state;

  CamResolutionName_t             ResName;            /**< identifier for accessing resolution depended calibration data */
  CamCalibDbHandle_t              hCamCalibDb;        /**< calibration database handle */

  uint16_t                        SigmaGreen;         /**< sigma value for green pixel */
  uint16_t                        SigmaRedBlue;       /**< sigma value for red/blue pixel */

  float                           fGradient;          /**< */
  float                           fOffset;            /**< */
  float                           fMin;          /**< */
  float                           fDiv;            /**< */
  AdpfConfig_t                    Config;
  float                           gain;               /**< current sensor gain */

  AdpfMode_t mode;
  CamerIcDpfInvStrength_t         DynInvStrength;     /**< */
  AdpfGains_t   NfGains;            /**< */
  CamerIcDpfNoiseLevelLookUp_t    Nll;                /**< noise level lookup */
  CamerIcDpfConfig_t DpfMode;

  unsigned int actives;
  bool ADPFEnable;

  //oyyf add
  CamFilterProfile_t FilterProfile[2];
  CamFilterProfile_t *pFilterProfile;
  CamerIcIspFltDeNoiseLevel_t denoise_level;
  CamerIcIspFltSharpeningLevel_t sharp_level;
  uint8_t demosaic_th;
  bool_t FltEnable;

  CamDsp3DNRSettingProfile_t Dsp3DNRSettingProfile[2];
  CamDsp3DNRSettingProfile_t *pDsp3DNRSettingProfile;
  Dsp3DnrResult_t Dsp3DnrResult;

  CamNewDsp3DNRProfile_t newDsp3DNRProfile[LIGHT_MODE_MAX];
  CamNewDsp3DNRProfile_t *pNew3DNRProfile;
  NewDsp3DnrResult_t NewDsp3DnrResult;

  CamIesharpenProfile_t rkSharpenProfile;
  RKDemosiacLpResult_t RKDemosaicLpResult; 
  RKIESharpResult_t	RKIESharpResult;

  enum LIGHT_MODE LightMode;

} AdpfContext_t;



#ifdef __cplusplus
}
#endif

/* @} ADPF */


#endif /* __ADPF_CTRL_H__*/
