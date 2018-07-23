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
#ifndef __AWDR_H__
#define __AWDR_H__

/**
 * @file awdr.h
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
 * @defgroup AWDR Auto denoising pre-filter module
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <cam_calibdb/cam_calibdb_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AWDR_MASK (1 << 0)
#define AWDR_WDR_MAXGAIN_LEVEL_MASK (1 << 1)


/*****************************************************************************/
/**
 *          AwdrHandle_t
 *
 * @brief   AWDR Module instance handle
 *
 *****************************************************************************/
typedef struct AwdrContext_s* AwdrHandle_t;         /**< handle to ADPF context */


/*****************************************************************************/
/**
 *          AwdrInstanceConfig_t
 *
 * @brief   AWDR Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AwdrInstanceConfig_s {
  AwdrHandle_t            hAwdr;              /**< handle returned by AdpfInit() */
} AwdrInstanceConfig_t;


typedef struct CamerIcWdrConfig_s {

} CamerIcWdrConfig_t;


/*****************************************************************************/
/**
 *          AwdrConfigType_t
 *
 * @brief   AWDR Configuration type
 *
 *****************************************************************************/

typedef enum AwdrMode_e {
  AWDR_MODE_INVALID = 0,
  AWDR_MODE_CONTROL_BY_GAIN = 1,
  AWDR_MODE_CONTROL_BY_MANUAL = 2,
  AWDR_MODE_CONTROL_EXT = 3
} AwdrMode_t;


/*****************************************************************************/
/**
 *          AwdrConfig_t
 *
 * @brief   AWDR Module configuration structure
 *
 *****************************************************************************/
typedef struct AwdrConfig_s {
  float  fSensorGain;        /**< initial sensor gain */
  AwdrMode_t mode;

  CamCalibDbHandle_t      hCamCalibDb;        /**< calibration database handle */

  uint16_t Wdr_MaxGain_level_RegValue;
} AwdrConfig_t;


typedef struct AwdrResult_s {
  unsigned int actives;

  uint16_t Wdr_MaxGain_level_RegValue;
} AwdrResult_t;


/*****************************************************************************/
/**
 *          AdpfInit()
 *
 * @brief   This function initializes the Auto denoising pre-filter module
 *
 * @param   pInstConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT AwdrInit
(
    AwdrHandle_t* handle,
    AwdrConfig_t* pConfig
);



/*****************************************************************************/
/**
 *          AdpfRelease()
 *
 * @brief   The function releases/frees the Auto denoising pre-filter module
 *
 * @param   handle  Handle to ADPFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwdrRelease
(
    AwdrHandle_t handle
);



/*****************************************************************************/
/**
 *          AdpfConfigure()
 *
 * @brief   This function configures the Auto denoising pre-filter module
 *
 * @param   handle  Handle to ADPFM
 * @param   pConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_INVALID_PARM
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT AwdrConfigure
(
    AwdrHandle_t handle,
    AwdrConfig_t* pConfig
);



/*****************************************************************************/
/**
 *          AdpfRun()
 *
 * @brief   The function calculates and adjusts a new DPF-setup regarding
 *          the current sensor-gain
 *
 * @param   handle  Handle to ADPFM
 *          gain    current sensor-gain
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwdrRun
(
    AwdrHandle_t    handle,
    const float     gain
);

RESULT AwdrGetResult
(
    AwdrHandle_t    handle,
    AwdrResult_t*  result
);

#ifdef __cplusplus
}
#endif


/* @} Awdr */


#endif /* __Awdr_H__*/

