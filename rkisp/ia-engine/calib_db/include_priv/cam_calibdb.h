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
 * @file cam_calibdb.h
 *
 * @brief
 *   Internal interface of the CamCalibDb.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_calibdb CamCalibDb internal interface
 * @{
 *
 */
#ifndef __CAM_CALIBDB_H__
#define __CAM_CALIBDB_H__

#include <ebase/types.h>
//#include <oslayer/oslayer.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
#include <common/list.h>

#include "cam_calibdb_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief   Internal context of the Cam-Calibration Database
 *
 * @note
 *
 */
typedef struct CamCalibDbContext_s {
  creation_date_t             cdate;          /**< creation date */
  creator_name_t              cname;          /**< name of creator */
  creator_version_t           cversion;       /**< version of creation tool (matlab generator) */
  sensor_name_t               sname;          /**< sesor name */
  sensor_sample_id_t          sid;            /**< sensor sample id */
  Isp_output_type_t           isp_output_type;/**<calib xml for one type of isp output*/
  
  List                        resolution;     /**< list of supported resolutions */
  CamCalibAwbPara_t*          pAwbProfile;     /* AWB  profile*/
  //List                        awb_global;     /**< list of supported awb_globals */
  CamCalibAfGlobal_t*          pAfGlobal;
  CamCalibAecGlobal_t*         pAecGlobal;    /**< AEC global settings */
  List         				  gocProfile;    /**< GOC global settings */
  CamCalibWdrGlobal_t*         pWdrGlobal;    /**< WDR global settings */
  CamCprocProfile_t*           pCprocGlobal;   /**< CPROC global settings */
  List                        ecm_profile;    /**< list of supported ECM profiles */
  //List                        illumination;   /**< list of supported illuminations */
  List                        lsc_profile;    /**< list of supported LSC profiles */
  List                        cc_profile;     /**< list of supported CC profiles */
  List                        bls_profile;    /**< list of supported BLS profiles */
  List                        cac_profile;    /**< list of supported CAC profiles */
  List                        dpf_profile;    /**< list of supported DPF profiles */
  List                        dpcc_profile;   /**< list of supported DPCC profiles */
  List                        iesharpen_profile;/**< list of supported IE-SHARPEN profiles */
  CamOTPGlobal_t*			  pOTPGlobal;

  CamCalibSystemData_t        system;
} CamCalibDbContext_t;


#ifdef __cplusplus
}
#endif


/* @} cam_calibdb */

#endif /* __CAM_CALIBDB_H__ */

