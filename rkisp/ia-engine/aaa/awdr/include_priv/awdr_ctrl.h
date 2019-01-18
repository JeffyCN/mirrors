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
#ifndef __AWDR_CTRL_H__
#define __AWDR_CTRL_H__

/**
 * @file awdr_ctrl.h
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
 * @defgroup AWDR Auto wdr Maxgain with ae
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>


#include "awdr.h"

#ifdef __cplusplus
extern "C"
{
#endif



/*****************************************************************************/
/**
 * @brief   This enum type specifies the different possible states of
 *          the AWDR module.
 */
/*****************************************************************************/
typedef enum AwdrState_e {
  AWDR_STATE_INVALID           = 0,                   /**< initialization value */
  AWDR_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
  AWDR_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
  AWDR_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
  AWDR_STATE_LOCKED            = 4,                   /**< instance is locked (for taking snapshots) */
  AWDR_STATE_MAX                                      /**< max */
} AwdrState_t;



/*****************************************************************************/
/**
 * @brief   Context of the AWDR module.
 */
/*****************************************************************************/
typedef struct AwdrContext_s {
  AwdrState_t                     state;
  CamCalibDbHandle_t              hCamCalibDb;        /**< calibration database handle */

  AwdrConfig_t                    Config;
  float                           gain;               /**< current sensor gain */

  AwdrMode_t mode;
  unsigned int actives;

  bool WdrEnable;
  bool WdrMaxGainEnable;

  CamCalibWdrGlobal_t* pWdrGlobal;

  //wdr max gain dynamic set with ae gain    --oyyf add
  //CamCalibWdrMaxGainLevelCurve_t *pWdrMaxGainLevelCurve;
  AwdrResult_t awdr_result;
} AwdrContext_t;



#ifdef __cplusplus
}
#endif

/* @} AWDR*/


#endif /* __AWDR_CTRL_H__*/

