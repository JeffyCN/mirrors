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
 * @file cam_calibdb_types.h
 *
 * @brief
 *   Interface description of the CamEngine.
 *
 *****************************************************************************/
/**
 *
 * @defgroup cam_calibdb_types CamCalibDb Types
 * @{
 */

#ifndef __CAM_CALIBDB_TYPES_H__
#define __CAM_CALIBDB_TYPES_H__

#include <ebase/types.h>

#ifdef __cplusplus
extern "C"
{
#endif



/*******************************************************************************
 * @brief The standard calibration database parameter values
 *
 * This enumeration lists the possible standard calibration values.
 */
typedef enum CamCalibDbParameterId_e {
  CAM_CALIBDB_ID_INVALID                          = 0,    /**< invalid id to avoid an uninitialized id */
  CAM_CALIBDB_ID_CREATION_DATE                    = 1,    /**< creation date */
  CAM_CALIBDB_ID_CREATOR                          = 2,    /**< nick of creator */
  CAM_CALIBDB_ID_CREATOR_VERSION                  = 3,    /**< creator/generator version */
  CAM_CALIBDB_ID_SENSOR_NAME                      = 4,    /**< sensor name */
  CAM_CALIBDB_ID_SENSOR_SAMPLE                    = 5,    /**< sensor sample identifier */
  CAM_CALIBDB_ID_RESOLUTION                       = 6,

  /* white balnace parameter */
  CAM_CALIBDB_ID_AWB_BASE                         = 100,
  CAM_CALIBDB_ID_AWB_SVDMEANVALUE                 = (CAM_CALIBDB_ID_AWB_BASE + 0),
  CAM_CALIBDB_ID_AWB_PCAMATRIX                    = (CAM_CALIBDB_ID_AWB_BASE + 1),
  CAM_CALIBDB_ID_AWB_CENTERLINE                   = (CAM_CALIBDB_ID_AWB_BASE + 2),
  CAM_CALIBDB_ID_AWB_KFACTOR                      = (CAM_CALIBDB_ID_AWB_BASE + 3),
  CAM_CALIBDB_ID_AWB_RG1                          = (CAM_CALIBDB_ID_AWB_BASE + 4),
  CAM_CALIBDB_ID_AWB_MAXDIST1                     = (CAM_CALIBDB_ID_AWB_BASE + 5),
  CAM_CALIBDB_ID_AWB_RG2                          = (CAM_CALIBDB_ID_AWB_BASE + 6),
  CAM_CALIBDB_ID_AWB_MAXDIST2                     = (CAM_CALIBDB_ID_AWB_BASE + 7),
  CAM_CALIBDB_ID_AWB_GLOBALFADE1                  = (CAM_CALIBDB_ID_AWB_BASE + 8),
  CAM_CALIBDB_ID_AWB_GLOBALGAINDISTANCE1          = (CAM_CALIBDB_ID_AWB_BASE + 9),
  CAM_CALIBDB_ID_AWB_GLOBALFADE2                  = (CAM_CALIBDB_ID_AWB_BASE + 10),
  CAM_CALIBDB_ID_AWB_GLOBALGAINDISTANCE2          = (CAM_CALIBDB_ID_AWB_BASE + 11),
  CAM_CALIBDB_ID_AWB_GLOBALS_FADE2                = (CAM_CALIBDB_ID_AWB_BASE + 12),
  CAM_CALIBDB_ID_AWB_GLOBALS_CB_MIN_REGIONMAX     = (CAM_CALIBDB_ID_AWB_BASE + 13),
  CAM_CALIBDB_ID_AWB_GLOBALS_CR_MIN_REGIONMAX     = (CAM_CALIBDB_ID_AWB_BASE + 14),
  CAM_CALIBDB_ID_AWB_GLOBALS_MAX_CSUM_REGIONMAX   = (CAM_CALIBDB_ID_AWB_BASE + 15),
  CAM_CALIBDB_ID_AWB_GLOBALS_CB_MIN_REGIONMIN     = (CAM_CALIBDB_ID_AWB_BASE + 16),
  CAM_CALIBDB_ID_AWB_GLOBALS_CR_MIN_REGIONMIN     = (CAM_CALIBDB_ID_AWB_BASE + 17),
  CAM_CALIBDB_ID_AWB_GLOBALS_MAX_CSUM_REGIONMIN   = (CAM_CALIBDB_ID_AWB_BASE + 18),
  CAM_CALIBDB_ID_AWB_GLOBALS_RGPROJ_INDOOR_MIN    = (CAM_CALIBDB_ID_AWB_BASE + 19),
  CAM_CALIBDB_ID_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN   = (CAM_CALIBDB_ID_AWB_BASE + 20),
  CAM_CALIBDB_ID_AWB_GLOBALS_RGPROJ_MAX           = (CAM_CALIBDB_ID_AWB_BASE + 21),
  CAM_CALIBDB_ID_AWB_GLOBALS_RGPROJ_MAX_SKY       = (CAM_CALIBDB_ID_AWB_BASE + 22),
  CAM_CALIBDB_ID_AWB_ILLUMINATION_PROFILE         = (CAM_CALIBDB_ID_AWB_BASE + 23),

  CAM_CALIBDB_ID_MAX
} CamCalibDbParameterId_t;



#define CREATION_DATE_SIZE          ( 15U )
typedef char                        creation_date_t[CREATION_DATE_SIZE];

#define CREATOR_NAME_SIZE           ( 10U )
typedef char                        creator_name_t[CREATOR_NAME_SIZE];

#define CREATOR_VERSION_SIZE        ( 20U )
typedef char                        creator_version_t[CREATOR_VERSION_SIZE];

#define SENSOR_NAME_SIZE            ( 20U )
typedef char                        sensor_name_t[SENSOR_NAME_SIZE];

#define SENSOR_SAMPLE_ID_SIZE       ( 20U )
typedef char                        sensor_sample_id_t[SENSOR_SAMPLE_ID_SIZE];

#define CODE_XML_PARSE_VERSION_SIZE       ( 20U )
typedef char                        code_xml_parse_version_t[SENSOR_SAMPLE_ID_SIZE];

#define SDK_IQ_VERIFY_SIZE       ( 20U )
typedef char                        sdk_IQ_verify_t[SDK_IQ_VERIFY_SIZE];

typedef enum Isp_output_type_e{
	isp_color_output_type = 0,
	isp_gray_output_type  = 1,	
	isp_mixture_output_type  = 2,	
	isp_max_output_type	
}Isp_output_type_t;

/*****************************************************************************/
/**
 * @brief   Global AWB calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibDbMetaData_s {
  creation_date_t         cdate;                  /**< creation date */
  creator_name_t          cname;                  /**< name/id of creator */
  creator_version_t       cversion;               /**< version of creation tool (matlab generator) */
  sensor_name_t           sname;                  /**< sensor name */
  sensor_sample_id_t      sid;                    /**< sensor sample id */
  code_xml_parse_version_t code_xml_parse_version;
  Isp_output_type_t       isp_output_type;
  sdk_IQ_verify_t		  sdk_iq_verify;
} CamCalibDbMetaData_t;



#ifdef __cplusplus
}
#endif


/* @} cam_calibdb_types */


#endif /* __CAM_CALIBDB_TYPES_H__ */

