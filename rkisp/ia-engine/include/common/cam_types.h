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
 * @file cam_types.h
 *
 * @brief Interface description for image sensor specific implementation (iss).
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup cam_types   Common Camera Type Definitions
 * @{
 *
 */
#ifndef __CAM_TYPES_H__
#define __CAM_TYPES_H__

#include <common/list.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
*Cam_Win:
*
*Configuration coordinate:
*
* (0,0)    ----------------------
*            |                                        |
*            |                                        |
*            |--------------------|(2048,2048)
*
*Result coordinate:
*
* (0,0)    ----------------------
*            |                                        |
*            |                                        |
*            |--------------------|(isp_input_width, isp_input_height)
*/
typedef struct Cam_Win {
  unsigned short h_offs;
  unsigned short v_offs;
  unsigned short h_size;
  unsigned short v_size;
} Cam_Win_t;


/*****************************************************************************/
/**
 * @brief   doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum  CamDoorType_e {
  CAM_DOOR_TYPE_OUTDOOR = 0,
  CAM_DOOR_TYPE_INDOOR  = (!CAM_DOOR_TYPE_OUTDOOR)
} CamDoorType_t;



/*****************************************************************************/
/**
 * @brief   doortype of an illumination profile
 */
/*****************************************************************************/
typedef enum CamAwbType_e {
  CAM_AWB_TYPE_MANUAL  = 0,
  CAM_AWB_TYPE_AUTO    = (!CAM_AWB_TYPE_MANUAL)
} CamAwbType_t;



/*****************************************************************************/
/**
 * @brief   3 channel color components
 */
/*****************************************************************************/
typedef enum Cam3ChColorComponent_e {
  CAM_3CH_COLOR_COMPONENT_RED     = 0,
  CAM_3CH_COLOR_COMPONENT_GREEN   = 1,
  CAM_3CH_COLOR_COMPONENT_BLUE    = 2,
  CAM_3CH_COLOR_COMPONENT_MAX
} Cam3ChColorComponent_t;



/*****************************************************************************/
/**
 * @brief   4 channel color components
 */
/*****************************************************************************/
typedef enum Cam4ChColorComponent_e {
  CAM_4CH_COLOR_COMPONENT_RED     = 0,
  CAM_4CH_COLOR_COMPONENT_GREENR  = 1,
  CAM_4CH_COLOR_COMPONENT_GREENB  = 2,
  CAM_4CH_COLOR_COMPONENT_BLUE    = 3,
  CAM_4CH_COLOR_COMPONENT_MAX
} Cam4ChColorComponent_t;

/******************************************************************************/
/**
 * @brief   A structure to represent a 5x5 matrix.
 *
 *          The 25 values are laid out as follows (zero based index):
 *
 *               | 00 01 02 03 04 | \n
 *               | 05 06 07 08 09 | \n
 *               | 10 11 12 13 14 | \n
 *               | 15 16 17 18 19 | \n
 *               | 20 21 22 23 24 | \n
 *
 * @note    The 25 values are represented as unsigned char numbers.
 *
 *****************************************************************************/
typedef struct Cam5x5UCharMatrix_s {
  uint8_t uCoeff[5 * 5];            /**< array of 5x5 unsigned char values */
} Cam5x5UCharMatrix_t;

typedef struct Cam9x9UCharMatrix_s {
  uint8_t uCoeff[9 * 9];            /**< array of 5x5 unsigned char values */
} Cam9x9UCharMatrix_t;

/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x1FloatMatrix_s {
  float fCoeff[1];
} Cam1x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x3FloatMatrix_s {
  float fCoeff[3];
} Cam1x3FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 4 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x4FloatMatrix_s {
  float fCoeff[4];
} Cam1x4FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 6 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x6FloatMatrix_s {
  float fCoeff[6];
} Cam1x6FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | ... | 15 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam1x16FloatMatrix_s {
  float fCoeff[16];
} Cam1x16FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam2x1FloatMatrix {
  float fCoeff[2];
} Cam2x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |
 *          | 2 | 3 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam2x2FloatMatrix {
  float fCoeff[4];
} Cam2x2FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x1FloatMatrix {
  float fCoeff[3];
} Cam3x1FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *          | 3 | 4 |  5 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x2FloatMatrix_s {
  float fCoeff[6];
} Cam3x2FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |
 *          | 3 | 4 |  5 |
 *          | 6 | 7 |  8 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam3x3FloatMatrix_s {
  float fCoeff[9];
} Cam3x3FloatMatrix_t;


/******************************************************************************/
/**
 * @brief   A structure to represent a 5x5 matrix.
 *
 *          The 25 values are laid out as follows (zero based index):
 *
 *               | 00 01 02 03 04 | \n
 *               | 05 06 07 08 09 | \n
 *               | 10 11 12 13 14 | \n
 *               | 15 16 17 18 19 | \n
 *               | 20 21 22 23 24 | \n
 *
 * @note    The 25 values are represented as float numbers.
 *
 *****************************************************************************/
typedef struct Cam5x5FloatMatrix_s {
  float fCoeff[25U];              /**< array of 5x5 float values */
} Cam5x5FloatMatrix_t;


/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 | ....
 *          |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 | ....
 *          |  34 |  35 |  36 |  37 |  38 |  39 |  40 |  41 | ....
 *          ...
 *          ...
 *          ...
 *          | 271 | 272 | 273 | 274 | 275 | 276 | 277 | 278 | .... | 288 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam17x17FloatMatrix_s {
  float fCoeff[17 * 17];
} Cam17x17FloatMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x3ShortMatrix_s {
  int16_t Coeff[3];
} Cam1x3ShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 | ... | 4 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x4UShortMatrix_s {
  uint16_t uCoeff[4];
} Cam1x4UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 | 2 | ... | 16 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam1x17UShortMatrix_s {
  uint16_t uCoeff[17];
} Cam1x17UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 | ....
 *          |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 | ....
 *          |  34 |  35 |  36 |  37 |  38 |  39 |  40 |  41 | ....
 *          ...
 *          ...
 *          ...
 *          | 271 | 272 | 273 | 274 | 275 | 276 | 277 | 278 | .... | 288 |
 *
 * @note    Coefficients are represented as short numbers
 */
/*****************************************************************************/
typedef struct Cam17x17UShortMatrix_s {
  uint16_t uCoeff[17 * 17];
} Cam17x17UShortMatrix_t;



/*****************************************************************************/
/**
 * @brief   name/identifier of a resolution
 */
/*****************************************************************************/
#define CAM_RESOLUTION_NAME         ( 15U )
typedef char                        CamResolutionName_t[CAM_RESOLUTION_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a resolution
 */
/*****************************************************************************/
#define CAM_FRAMERATE_NAME          ( 25U )
typedef char                        CamFramerateName_t[CAM_FRAMERATE_NAME];



/*****************************************************************************/
/**
 * @brief   name/identifier of an illumination
 */
/*****************************************************************************/
#define CAM_BLS_PROFILE_NAME        ( 10U )
typedef char                        CamBlsProfileName_t[CAM_BLS_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an illumination
 */
/*****************************************************************************/
#define CAM_ILLUMINATION_NAME       ( 20U )
typedef char                        CamIlluminationName_t[CAM_ILLUMINATION_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a lense shade correction profile (LscProfile)
 */
/*****************************************************************************/
#define CAM_LSC_PROFILE_NAME        ( 25U )
typedef char                        CamLscProfileName_t[CAM_LSC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a color correction profile (CcProfile)
 */
/*****************************************************************************/
#define CAM_CC_PROFILE_NAME         ( 20U )
typedef char                        CamCcProfileName_t[CAM_CC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a auto white balance profile (AwbProfile)
 */
/*****************************************************************************/
#define CAM_AWB_PROFILE_NAME        ( 20U )
typedef char                        CamAwbProfileName_t[CAM_AWB_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure scheme (EcmScheme)
 */
/*****************************************************************************/
#define CAM_ECM_SCHEME_NAME         ( 20U )
typedef char                        CamEcmSchemeName_t[CAM_ECM_SCHEME_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure profile (EcmProfile)
 */
/*****************************************************************************/
#define CAM_ECM_PROFILE_NAME        ( 20U )
typedef char                        CamEcmProfileName_t[CAM_ECM_PROFILE_NAME];

/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure scheme (EcmScheme)
 */
/*****************************************************************************/
#define CAM_DYNAMIC_SETPOINT_NAME         ( 20U )
typedef char                        CamDynamicSetpointName_t[CAM_ECM_SCHEME_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of an exposure scheme (EcmScheme)
 */
/*****************************************************************************/
#define CAM_EXP_SEPARATE_NAME         ( 20U )
typedef char                        CamExpSeparateName_t[CAM_ECM_SCHEME_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a chromatic abberation correction profile
 *          (CacProfile)
 */
/*****************************************************************************/
#define CAM_CAC_PROFILE_NAME        ( 20U )
typedef char                        CamCacProfileName_t[CAM_CAC_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a denoising prefilter profile (DpfProfile)
 */
/*****************************************************************************/
#define CAM_DPF_PROFILE_NAME        ( 20U )
typedef char                        CamDpfProfileName_t[CAM_DPF_PROFILE_NAME];

#define CAM_DSP_3DNR_PROFILE_NAME        ( 20U )
typedef char                        CamDsp3dnrProfileName_t[CAM_DSP_3DNR_PROFILE_NAME];

#define CAM_NEW_DSP_3DNR_PROFILE_NAME        ( 20U )
typedef char                        CamNewDsp3dnrProfileName_t[CAM_NEW_DSP_3DNR_PROFILE_NAME];


#define CAM_FILTER_PROFILE_NAME        ( 20U )
typedef char                        CamFilterProfileName_t[CAM_FILTER_PROFILE_NAME];


/*****************************************************************************/
/**
 * @brief   name/identifier of a denoising prefilter profile (DpccProfile)
 */
/*****************************************************************************/
#define CAM_DPCC_PROFILE_NAME       ( 20U )
typedef char                        CamDpccProfileName_t[CAM_DPCC_PROFILE_NAME];

#define CAM_GOC_PROFILE_NAME        ( 20U )
typedef char                        CamGOCProfileName_t[CAM_ECM_PROFILE_NAME];

//werring.wu 2018 1.17
/*****************************************************************************/
/**
 * @brief   name/identifier of a issharpen prefilter profile (Profile)
 */
/*****************************************************************************/
#define CAM_IESHARPEN_PROFILE_NAME        ( 20U )
typedef char  CamIesharpenProfileName_t[CAM_IESHARPEN_PROFILE_NAME];

/*****************************************************************************/
/**
 * @brief   framerate profile
 */
/*****************************************************************************/
typedef struct CamFrameRate_s {
  void*                p_next;            /**< for adding to a list */

  CamFramerateName_t  name;               /**< name of framerate scheme */
  float               fps;                /**< framerate */
} CamFrameRate_t;


/*****************************************************************************/
/**
 * @brief   resolution profile
 */
/*****************************************************************************/
typedef struct CamResolution_t {
  void*                p_next;            /**< for adding to a list */

  CamResolutionName_t name;               /**< desctriptor */
  uint16_t            width;              /**< resolution width */
  uint16_t            height;             /**< resolution height */
  uint32_t            id;                 /**< bitmask identifier */

  List                framerates;         /**< list of framerates */
} CamResolution_t;


/*******************************************************************************
 **
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as fix point numbers.
 *
 *****************************************************************************/
typedef struct CamerIcGains_s {
  uint16_t Red;                       /**< gain value for the red channel */
  uint16_t GreenR;                    /**< gain value for the green-red channel */
  uint16_t GreenB;                    /**< gain value for the green-blue channel */
  uint16_t Blue;                      /**< gain value for the blue channel */
} CamerIcGains_t;

/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamSaturationCurve_s {
  uint16_t    ArraySize;
  float*       pSensorGain;
  float*       pSaturation;
} CamSaturationCurve_t;



/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to vignetting (compensation)
 *          interpolation
 */
/*****************************************************************************/
typedef struct CamVignettingCurve_s {
  uint16_t    ArraySize;
  float*       pSensorGain;
  float*       pVignetting;
} CamVignettingCurve_t;



/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to vignetting (compensation)
 *          interpolation
 *
 */
/*****************************************************************************/
typedef struct CamLscMatrix_s {
  Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];
} CamLscMatrix_t;



/*****************************************************************************/
/**
 * @brief   BLS calibration structure
 */
/*****************************************************************************/
typedef struct CamBlsProfile_s {
  void*                        p_next;                /**< for adding to a list */

  CamBlsProfileName_t         name;                   /**< profile name */
  CamResolutionName_t         resolution;             /**< resolution link */

  Cam1x4UShortMatrix_t        level;                  /**< black level for all 4 color components */
} CamBlsProfile_t;



/*****************************************************************************/
/**
 * @brief   AWB_V11 Illumination specific structure.
 */
/*****************************************************************************/
typedef struct CamAwb_V11_IlluProfile_s {
  void*                        p_next;                /**< for adding to a list */

  CamIlluminationName_t       name;                   /**< name of the illumination profile (i.e. "D65", "A", ... )*/
  uint32_t                    id;                     /**< unique id */

  CamDoorType_t               DoorType;               /**< indoor or outdoor profile */
  CamAwbType_t                AwbType;                /**< manual or auto profile */

  /* for manual white balance data */
  Cam3x3FloatMatrix_t         CrossTalkCoeff;         /**< CrossTalk matrix coefficients */
  Cam1x3FloatMatrix_t         CrossTalkOffset;        /**< CrossTalk offsets */
  Cam1x4FloatMatrix_t         ComponentGain;          /**< White Balance Gains*/

  /* gaussian mixture modell */
  //Cam2x1FloatMatrix_t         GaussMeanValue;         /**< */
  //Cam2x2FloatMatrix_t         CovarianceMatrix;       /**< */
  //Cam1x1FloatMatrix_t         GaussFactor;            /**< */
  //Cam2x1FloatMatrix_t         Threshold;              /**< */
  
  /* from awb calibration data */
  Cam1x4FloatMatrix_t         referenceWBgain;        /**< */

  /* adaptive color correctio */
  CamSaturationCurve_t        SaturationCurve;        /**< stauration over gain curve */

  /* adative lense shade correction */
  CamVignettingCurve_t        VignettingCurve;        /**< vignetting over gain curve */

  #define CAM_NO_CC_PROFILES  ( 10 )                  /**< max number of cc-profiles per illumination */
  int32_t                     cc_no;
  CamCcProfileName_t          cc_profiles[CAM_NO_CC_PROFILES];

  #define CAM_NO_LSC_PROFILES ( 5 )
  #define CAM_NO_RESOLUTIONS  ( 4 )
  int32_t                     lsc_res_no;
  int32_t                     lsc_no[CAM_NO_RESOLUTIONS];
  CamLscProfileName_t         lsc_profiles[CAM_NO_RESOLUTIONS][CAM_NO_LSC_PROFILES];
} CamAwb_V11_IlluProfile_t;
/*****************************************************************************/
/**
 * @brief   AWB_V10 Illumination specific structure.
 */
/*****************************************************************************/
typedef struct CamAwb_V10_IlluProfile_s {
  void*                        p_next;                /**< for adding to a list */

  CamIlluminationName_t       name;                   /**< name of the illumination profile (i.e. "D65", "A", ... )*/
  uint32_t                    id;                     /**< unique id */

  CamDoorType_t               DoorType;               /**< indoor or outdoor profile */
  CamAwbType_t                AwbType;                /**< manual or auto profile */

  /* for manual white balance data */
  Cam3x3FloatMatrix_t         CrossTalkCoeff;         /**< CrossTalk matrix coefficients */
  Cam1x3FloatMatrix_t         CrossTalkOffset;        /**< CrossTalk offsets */
  Cam1x4FloatMatrix_t         ComponentGain;          /**< White Balance Gains*/

  /* gaussian mixture modell */
  Cam2x1FloatMatrix_t         GaussMeanValue;         /**< */
  Cam2x2FloatMatrix_t         CovarianceMatrix;       /**< */
  Cam1x1FloatMatrix_t         GaussFactor;            /**< */
  Cam2x1FloatMatrix_t         Threshold;              /**< */

  /* adaptive color correctio */
  CamSaturationCurve_t        SaturationCurve;        /**< stauration over gain curve */

  /* adative lense shade correction */
  CamVignettingCurve_t        VignettingCurve;        /**< vignetting over gain curve */

#define CAM_NO_CC_PROFILES  ( 10 )                  /**< max number of cc-profiles per illumination */
  int32_t                     cc_no;
  CamCcProfileName_t          cc_profiles[CAM_NO_CC_PROFILES];

#define CAM_NO_LSC_PROFILES ( 5 )
#define CAM_NO_RESOLUTIONS  ( 4 )
  int32_t                     lsc_res_no;
  int32_t                     lsc_no[CAM_NO_RESOLUTIONS];
  CamLscProfileName_t         lsc_profiles[CAM_NO_RESOLUTIONS][CAM_NO_LSC_PROFILES];
} CamAwb_V10_IlluProfile_t;



/*****************************************************************************/
/**
 * @brief   LSC profile
 */
/*****************************************************************************/
typedef struct CamLscProfile_s {
  void*                    p_next;                                /**< for adding to a list */

  CamLscProfileName_t     name;                                   /**< profile name */
  CamResolutionName_t     resolution;                             /**< resolution link */
  CamIlluminationName_t   illumination;                           /**< illumination link */
  float                   vignetting;                             /**< vignetting value */

  uint16_t                LscSectors;
  uint16_t                LscNo;
  uint16_t                LscXo;
  uint16_t                LscYo;

  uint16_t                LscXGradTbl[8];
  uint16_t                LscYGradTbl[8];
  uint16_t                LscXSizeTbl[8];
  uint16_t                LscYSizeTbl[8];

  Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];     /**< matrix for different color channels */
} CamLscProfile_t;



/*****************************************************************************/
/**
 * @brief   CC profile
 *
 */
/*****************************************************************************/
typedef struct CamCcProfile_s {
  void*                    p_next;                /**< for adding to a list */

  CamCcProfileName_t      name;                   /**< profile name */
  float                   saturation;             /**< saturation value */

  Cam3x3FloatMatrix_t     CrossTalkCoeff;         /**< CrossTalk matrix coefficients */
  Cam1x3FloatMatrix_t     CrossTalkOffset;        /**< CrossTalk offsets */
  Cam1x4FloatMatrix_t     ComponentGain;          /**< White Balance Gains*/
} CamCcProfile_t;



/*****************************************************************************/
/**
 * @brief   CAC calibration structure
 */
/*****************************************************************************/
typedef struct CamCacProfile_s {
  void*                    p_next;            /**< for adding to a list */

  CamCacProfileName_t     name;               /**< profile name */
  CamResolutionName_t     resolution;         /**< resolution link */

  uint8_t                 x_ns;               /**< horizontal normalization shift */
  uint8_t                 x_nf;               /**< horizontal normalization factor */
  uint8_t                 y_ns;               /**< vertical normalization shift */
  uint8_t                 y_nf;               /**< vertical normalization factor */

  Cam1x3FloatMatrix_t     Red;                /**< coeffciencts A, B and C for red */
  Cam1x3FloatMatrix_t     Blue;               /**< coeffciencts A, B and C for blue */

  int16_t                 hCenterOffset;
  int16_t                 vCenterOffset;
} CamCacProfile_t;

/**
 * @brief   Enumeration type to configure de-noising level.
 *
 */
typedef enum CamerIcIspFltDeNoiseLevel_e {
  CAMERIC_ISP_FLT_DENOISE_LEVEL_INVALID   = 0,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_0         = 1,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_1         = 2,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_2         = 3,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_3         = 4,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_4         = 5,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_5         = 6,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_6         = 7,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_7         = 8,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_8         = 9,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_9         = 10,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_10        = 11,
  CAMERIC_ISP_FLT_DENOISE_LEVEL_MAX
} CamerIcIspFltDeNoiseLevel_t;



/**
 * @brief   Enumeration type to configure sharpening level.
 *
 */
typedef enum CamerIcIspFltSharpeningLevel_e {
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_INVALID   = 0,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_0         = 1,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_1         = 2,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_2         = 3,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_3         = 4,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_4         = 5,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_5         = 6,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_6         = 7,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_7         = 8,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_8         = 9,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_9         = 10,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_10        = 11,
  CAMERIC_ISP_FLT_SHARPENING_LEVEL_MAX
} CamerIcIspFltSharpeningLevel_t;


/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamDenoiseLevelCurve_s {
  uint16_t    ArraySize;
  float*       pSensorGain;
  CamerIcIspFltDeNoiseLevel_t*        pDlevel;
} CamDenoiseLevelCurve_t;

/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamSharpeningLevelCurve_s {
  uint16_t    ArraySize;
  float*       pSensorGain;
  CamerIcIspFltSharpeningLevel_t*      pSlevel;
} CamSharpeningLevelCurve_t;

/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamDemosaicThCurve_s {
  uint16_t    ArraySize;
  float*       pSensorGain;
  uint8_t*      pThlevel;
} CamDemosaicThCurve_t;


/*****************************************************************************/
/**
 * @brief   parameters for a sensorgain to saturation interpolation
 */
/*****************************************************************************/
typedef struct CamFilterLevelRegConf_s {
  uint8_t ArraySize;
  uint8_t FiltLevelRegConfEnable;
  uint8_t* p_FiltLevel;
  uint8_t* p_grn_stage1;    /* ISP_FILT_MODE register fields*/
  uint8_t* p_chr_h_mode;    /* ISP_FILT_MODE register fields*/
  uint8_t* p_chr_v_mode;    /* ISP_FILT_MODE register fields*/
  uint32_t*  p_thresh_bl0;
  uint32_t*  p_thresh_bl1;
  uint32_t*  p_thresh_sh0;
  uint32_t*  p_thresh_sh1;
  uint32_t*  p_fac_sh1;
  uint32_t*  p_fac_sh0;
  uint32_t*  p_fac_mid;
  uint32_t*  p_fac_bl0;
  uint32_t*  p_fac_bl1;
} CamFilterLevelRegConf_t;

typedef struct CamDemosaicLpThreshold_s
{
	uint8_t  sw_thgrad_divided[5];
	uint8_t  sw_thCSC_divided[5];
	uint8_t  sw_thdiff_divided[5];
	uint16_t sw_thVar_divided[5];
}CamDemosaicLpThreshold_t;

//werring.wu add 2018 1 17
typedef struct CamDemosaicLpProfile_s
{
	uint8_t  lp_en;
	uint8_t  rb_filter_en;
	uint8_t  hp_filter_en;
	uint8_t  use_old_lp;// use old version
	uint8_t* lu_divided;//
	uint8_t  lu_divided_ArraySize;
	float* gainsArray;
	uint8_t  gainsArray_ArraySize;
	float* thH_divided0;
	uint8_t  thH_divided0_ArraySize;
	float* thH_divided1;
	uint8_t  thH_divided1_ArraySize;
	float* thH_divided2;
	uint8_t  thH_divided2_ArraySize;
	float* thH_divided3;
	uint8_t  thH_divided3_ArraySize;
	float* thH_divided4;
	uint8_t  thH_divided4_ArraySize;
	float* thCSC_divided0;
	uint8_t  thCSC_divided0_ArraySize;
	float* thCSC_divided1;
	uint8_t  thCSC_divided1_ArraySize;
	float* thCSC_divided2;
	uint8_t  thCSC_divided2_ArraySize;
	float* thCSC_divided3;
	uint8_t  thCSC_divided3_ArraySize;
	float* thCSC_divided4;
	uint8_t  thCSC_divided4_ArraySize;
	float* diff_divided0;
	uint8_t  diff_divided0_ArraySize;
	float* diff_divided1;
	uint8_t  diff_divided1_ArraySize;
	float* diff_divided2;
	uint8_t  diff_divided2_ArraySize;
	float* diff_divided3;
	uint8_t  diff_divided3_ArraySize;
	float* diff_divided4;
	uint8_t  diff_divided4_ArraySize;
	float* varTh_divided0;
	uint8_t  varTh_divided0_ArraySize;
	float* varTh_divided1;
	uint8_t  varTh_divided1_ArraySize;
	float* varTh_divided2;
	uint8_t  varTh_divided2_ArraySize;
	float* varTh_divided3;
	uint8_t  varTh_divided3_ArraySize;
	float* varTh_divided4;
	uint8_t  varTh_divided4_ArraySize;
	float*  thgrad_r_fct;
	uint8_t thgrad_r_fct_ArraySize;
	float*  thdiff_r_fct;
	uint8_t thdiff_r_fct_ArraySize;
	float*  thvar_r_fct;
	uint8_t thvar_r_fct_ArraySize;
	float*  thgrad_b_fct;
	uint8_t thgrad_b_fct_ArraySize;
	float*  thdiff_b_fct;
	uint8_t thdiff_b_fct_ArraySize;
	float*  thvar_b_fct;
	uint8_t thvar_b_fct_ArraySize;
	float*  similarity_th;
	uint8_t similarity_th_ArraySize;
	uint8_t  th_var_en;
	uint8_t  th_csc_en;
	uint8_t  th_diff_en;
	uint8_t  th_grad_en;
	float*  th_var;
	uint8_t th_var_ArraySize;
	float*  th_csc;
	uint8_t th_csc_ArraySize;
	float*  th_diff;
	uint8_t th_diff_ArraySize;
	float*  th_grad;
	uint8_t th_grad_ArraySize;
	float*  flat_level_sel;
	uint8_t flat_level_sel_ArraySize;
	float*  pattern_level_sel;
	uint8_t pattern_level_sel_ArraySize;
	float*  edge_level_sel;
	uint8_t edge_level_sel_ArraySize;
}CamDemosaicLpProfile_t;

typedef struct CamFilterProfile_s{
  void*                    p_next;
  CamFilterProfileName_t name;
  float FilterEnable;
  CamDenoiseLevelCurve_t        DenoiseLevelCurve;
  CamSharpeningLevelCurve_t     SharpeningLevelCurve;
  CamFilterLevelRegConf_t FiltLevelRegConf;

  CamDemosaicThCurve_t DemosaicThCurve;
  CamDemosaicLpProfile_t DemosaicLpConf;
}CamFilterProfile_t;


typedef struct CamDsp3DnrDefaultLevelSetting_s
{
	unsigned char luma_sp_nr_en;			// control the strength of spatial luma denoise enable flag
	unsigned char * pluma_sp_nr_level;		   // control the strength of spatial luma denoise
	unsigned char luma_te_nr_en;			// control the strength of spatial luma denoise enable flag
	unsigned char * pluma_te_nr_level;		   // control the strength of temporal luma denoise
	unsigned char chrm_sp_nr_en;			// control the strength of spatial chroma denoise enable flag
	unsigned char * pchrm_sp_nr_level;		   // control the strength of spatial chroma denoise level
	unsigned char chrm_te_nr_en;			// control the strength of spatial luma denoise enable flag
	unsigned char * pchrm_te_nr_level;		   // control the strength of temporal chroma denoise level
	unsigned char shp_en;
	unsigned char * pshp_level; 		  // control sharpness strenth

}CamDsp3DnrDefaultLevelSetting_t;

#define CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM  (5)
#define CAM_DSP_3DNR_SETTING_WEIGHT_COL_NUM  (5)
#define CAM_CALIBDB_3DNR_WEIGHT_NUM  (CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM*CAM_DSP_3DNR_SETTING_WEIGHT_COL_NUM)


typedef struct CamDsp3DNRLumaSetting_s{
	//5x5 luma spatial weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
	unsigned char luma_default;      // 1 use level define,0 use those parameters below
	unsigned char *pluma_sp_rad;      //spatial bilateral filter size
	unsigned char *pluma_te_max_bi_num;      //temporal max bilateral frame num
	uint8_t *pluma_weight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
}CamDsp3DNRLumaSetting_t;

typedef struct CamDsp3DNRChrmSetting_s{
	//5x5 chroma spatial weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are unsigned int type.
	unsigned char chrm_default;      // 1 use level define,0 use those parameters below
	unsigned char *pchrm_sp_rad;      //chroma spatial bilateral filter size
	unsigned char *pchrm_te_max_bi_num;      //temporal max bilateral frame num
	uint8_t *pchrm_weight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
}CamDsp3DNRChrmSetting_t;

typedef struct CamDsp3DNRShpSetting_s{
	unsigned char shp_default;      // 1 use level define,0 use those parameters below
    //threshold from 0 to 31
    unsigned char *psrc_shp_thr;
    //shift bit of  sum of weight.
    unsigned char *psrc_shp_div;
    //luma sharpness enable flag
    unsigned char *psrc_shp_l;
    //chroma sharpness enable flag
    unsigned char *psrc_shp_c;
	//5x5 sharpness weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
	int8_t *psrc_shp_weight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
}CamDsp3DNRShpSetting_t;

typedef struct CamDsp3DNRSettingProfile_s {
	void*                    p_next;
	CamDsp3dnrProfileName_t name;

	unsigned char Enable;
    int ArraySize;
	float *pgain_Level;
	uint16_t *pnoise_coef_numerator;
	uint16_t *pnoise_coef_denominator;

	CamDsp3DnrDefaultLevelSetting_t sDefaultLevelSetting;
	CamDsp3DNRLumaSetting_t sLumaSetting;
	CamDsp3DNRChrmSetting_t sChrmSetting;
	CamDsp3DNRShpSetting_t sSharpSetting;

} CamDsp3DNRSettingProfile_t;
typedef struct CamNewDsp3DNRYnrParams_s {
	uint32_t enable_ynr;        // Set to 1 by default
	uint32_t enable_tnr;        // Set to 1 by default, it will be disabled when enable_ynr=1
	uint32_t enable_iir;        // Set to 0 by default, it will be disabled when enable_ynr=1
	uint32_t *pynr_time_weight_level;        //Denoise weight of time, valid range: 1 - 4, default 3
	uint32_t *pynr_spat_weight_level;        // Denoise weight of spatial, valid range: 0 - 28, default 16
	uint32_t reserved[4];
}CamNewDsp3DNRYnrParams_t;


typedef struct CamNewDsp3DNRUvnrParams_s {
	uint32_t enable_uvnr;       // Set to 1 by default
	uint32_t *puvnr_weight_level;       // Denoise weight for uvnr, valid range: 4 - 16 default 12
	uint32_t reserved[4];
}CamNewDsp3DNRUvnrParams_t;


typedef struct CamNewDsp3DNRSharpParams_s{
	uint32_t enable_sharp;      // Set to 1 by default, enable DSP sharpness algorithm
	uint32_t *psharp_weight_level;      // Sharpness weight, valid range: 0 - 4 defalut 2
	uint32_t reserved[4];
}CamNewDsp3DNRSharpParams_t;


typedef struct CamNewDsp3DNRProfile_s {
	void*                    p_next;
	CamNewDsp3dnrProfileName_t name;
	
	CamNewDsp3DNRYnrParams_t ynr;
	CamNewDsp3DNRUvnrParams_t uvnr;
	CamNewDsp3DNRSharpParams_t sharp;
	
	uint32_t enable_dpc;        // Set to 1 by default, enable DSP dpc algorithm
	uint32_t reserved[4];

	uint32_t enable_3dnr;			// whole 3dnr function enable bits;
	int ArraySize;
	float *pgain_Level;
}CamNewDsp3DNRProfile_t;

/*****************************************************************************/
/**
 * @brief   MFD calibration structure
 */
/*****************************************************************************/
typedef struct CamMfdProfile_s
{
	uint8_t					enable;			/**< mfd enable*/
	float 					gain[3];		/**< mfd gain */
	float 					frames[3];		/**< mfd frames */
} CamMfdProfile_t;

/*****************************************************************************/
/**
 * @brief   UVNR calibration structure
 */
/*****************************************************************************/
typedef struct CamUvnrProfile_s
{
	uint8_t					enable;			/**< uvnr enable*/
	float 					gain[3];		/**< uvnr gain */
	float 					ratio[3];		/**< uvnr ratio */
	float 					distances[3];	/**< uvnr distances */
} CamUvnrProfile_t;

/*****************************************************************************/
/**
 * @brief   DPF calibration structure
 */
/*****************************************************************************/
typedef struct CamDpfProfile_s {
  void*                    p_next;            /**< for adding to a list */

  CamDpfProfileName_t     name;               /**< profile name */
  CamResolutionName_t     resolution;         /**< resolution link */

  uint16_t                nll_segmentation;
  Cam1x17UShortMatrix_t   nll_coeff;

  uint16_t                SigmaGreen;         /**< */
  uint16_t                SigmaRedBlue;       /**< */
  float                   fGradient;          /**< */
  float                   fOffset;            /**< */
  Cam1x4FloatMatrix_t     NfGains;            /**< */

  uint16_t ADPFEnable;

  List FilterList;
  List Dsp3DNRSettingProfileList;
  List newDsp3DNRProfileList;

  CamMfdProfile_t         Mfd;                /**< mfd struct*/
  CamUvnrProfile_t        Uvnr;               /**< uvnr struct*/
  
} CamDpfProfile_t;



/*****************************************************************************/
/**
 * @brief   DPCC calibration structure
 */
/*****************************************************************************/
typedef struct CamDpccProfile_s {
  void*                    p_next;            /**< for adding to a list */

  CamDpccProfileName_t    name;               /**< profile name */
  CamResolutionName_t     resolution;         /**< resolution link */

  uint32_t                isp_dpcc_mode;
  uint32_t                isp_dpcc_output_mode;
  uint32_t                isp_dpcc_set_use;
  uint32_t                isp_dpcc_methods_set_1;
  uint32_t                isp_dpcc_methods_set_2;
  uint32_t                isp_dpcc_methods_set_3;
  uint32_t                isp_dpcc_line_thresh_1;
  uint32_t                isp_dpcc_line_mad_fac_1;
  uint32_t                isp_dpcc_pg_fac_1;
  uint32_t                isp_dpcc_rnd_thresh_1;
  uint32_t                isp_dpcc_rg_fac_1;
  uint32_t                isp_dpcc_line_thresh_2;
  uint32_t                isp_dpcc_line_mad_fac_2;
  uint32_t                isp_dpcc_pg_fac_2;
  uint32_t                isp_dpcc_rnd_thresh_2;
  uint32_t                isp_dpcc_rg_fac_2;
  uint32_t                isp_dpcc_line_thresh_3;
  uint32_t                isp_dpcc_line_mad_fac_3;
  uint32_t                isp_dpcc_pg_fac_3;
  uint32_t                isp_dpcc_rnd_thresh_3;
  uint32_t                isp_dpcc_rg_fac_3;
  uint32_t                isp_dpcc_ro_limits;
  uint32_t                isp_dpcc_rnd_offs;
} CamDpccProfile_t;

typedef struct CamIesharpenGridConf_s
{
    uint16_t* p_grad;            //lowlight grad segment points
    uint8_t p_grad_ArraySize;
    uint8_t* sharp_factor;    // lowlight sharpen factor 0-64
    uint8_t sharp_factor_ArraySize;
    uint8_t* line1_filter_coe; //prefilter the data 2x3 
    uint8_t line1_filter_coe_ArraySize;
    uint8_t* line2_filter_coe;//                           3x3
    uint8_t line2_filter_coe_ArraySize;
    uint8_t* line3_filter_coe;//                           2x3
    uint8_t line3_filter_coe_ArraySize;
	uint8_t *lap_mat_coe;
	uint8_t lap_mat_coe_ArraySize;
}CamIesharpenGridConf_t;

typedef struct CamIesharpenProfile_s
{
    void                        *p_next;                /**< for adding to a list */
    CamIesharpenProfileName_t   name;//
    CamResolutionName_t         resolution;             /**< resolution link */
    uint8_t iesharpen_en;     // iesharpen_en 0 off, 1 on
    uint8_t coring_thr;         // iesharpen coring_thr is default 0
    uint8_t full_range;         // iesharpen full range(yuv data) 1:full_range(0-255),2:range(16-24?)
    uint8_t switch_avg;       //iesharpen whether is compare center pixel with edge pixel
    uint8_t* yavg_thr;// Y channel is set five segments by the Matrix
    uint8_t yavg_thr_ArraySize;
    uint8_t* P_delta1; 
    uint8_t P_delta1_ArraySize;
    uint8_t* P_delta2;
    uint8_t P_delta2_ArraySize;
    uint8_t* pmaxnumber;
    uint8_t pmaxnumber_ArraySize;
    uint8_t* pminnumber;
    uint8_t pminnumber_ArraySize;
    uint8_t* gauss_flat_coe;    // filter mask for flat 
    uint8_t gauss_flat_coe_ArraySize;
    uint8_t* gauss_noise_coe;//filter mask2 for noise
    uint8_t gauss_noise_coe_ArraySize;
    uint8_t* gauss_other_coe; //filter mask for other
    uint8_t gauss_other_coe_ArraySize;
    CamIesharpenGridConf_t lgridconf;
    CamIesharpenGridConf_t hgridconf;
    uint8_t* uv_gauss_flat_coe; //uv channel faussian filter 3x5 in flat point
    uint8_t uv_gauss_flat_coe_ArraySize;
    uint8_t* uv_gauss_noise_coe;//uv channel faussian filter 3x5 in noise point
    uint8_t uv_gauss_noise_coe_ArraySize;
    uint8_t* uv_gauss_other_coe;//uv channel faussian filter 3x5 in other point
    uint8_t uv_gauss_other_coe_ArraySize;
	float gain_dvide;
} CamIesharpenProfile_t;


/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for Rg/Bg color space
 *          clipping
 */
/*****************************************************************************/
typedef struct CamAwbClipParm_s {
  float*       pRg1;
  float*       pMaxDist1;
  float*       pRg2;
  float*       pMaxDist2;
  uint16_t    ArraySize1;
  uint16_t    ArraySize2;
} CamAwbClipParm_t;



/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for AWB out of range
 *          handling
 *
 */
/*****************************************************************************/
typedef struct CamAwbGlobalFadeParm_s {
  float*       pGlobalFade1;
  float*       pGlobalGainDistance1;
  float*       pGlobalFade2;
  float*       pGlobalGainDistance2;
  uint16_t    ArraySize1;
  uint16_t    ArraySize2;
} CamAwbGlobalFadeParm_t;



/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for near white pixel
 *          parameter calculations
 */
/*****************************************************************************/
typedef struct CamAwb_V11_Fade2Parm_s {
  float*      pFade;

  float*      pMaxCSum_br;
  float*      pMaxCSum_sr;
  float*      pMinC_br;
  float*      pMinC_sr;
  float*      pMaxY_br;
  float*      pMaxY_sr;
  float*      pMinY_br;
  float*      pMinY_sr;
  float*      pRefCb;
  float*      pRefCr;
  uint16_t    ArraySize;
} CamAwb_V11_Fade2Parm_t;


/*****************************************************************************/
/**
 * @brief   Contains pointers to parameter arrays for near white pixel
 *          parameter calculations
 */
/*****************************************************************************/
typedef struct CamAwb_V10_Fade2Parm_s {
  float*      pFade;
  float*      pCbMinRegionMax;
  float*      pCrMinRegionMax;
  float*      pMaxCSumRegionMax;
  float*      pCbMinRegionMin;
  float*      pCrMinRegionMin;
  float*      pMaxCSumRegionMin;
  float*    pMinCRegionMax;
  float*      pMinCRegionMin;
  float*    pMaxYRegionMax;
  float*    pMaxYRegionMin;
  float*    pMinYMaxGRegionMax;
  float*    pMinYMaxGRegionMin;
  float*    pRefCb;
  float*    pRefCr;
  uint16_t    ArraySize;
} CamAwb_V10_Fade2Parm_t;



/*****************************************************************************/
/**
 *          IsiLine_t
 *
 * @brief   Contains parameters for a straight line in Hesse normal form in
 *          Rg/Bg colorspace
 *
 */
/*****************************************************************************/
typedef struct CamCenterLine_s {
  float f_N0_Rg;                                  /**< Rg component of normal vector */
  float f_N0_Bg;                                  /**< Bg component of normal vector */
  float f_d;                                      /**< Distance of normal vector     */
} CamCenterLine_t;



/*****************************************************************************/
/**
 * @brief   Global AWB IIR Filter
 */
/*****************************************************************************/
typedef struct CamCalibIIR_s {
  float       fIIRDampCoefAdd;                        /**< incrementer of damping coefficient */
  float       fIIRDampCoefSub;                        /**< decrementer of damping coefficient */
  float       fIIRDampFilterThreshold;                /**< threshold for incrementing or decrementing of damping coefficient */

  float       fIIRDampingCoefMin;                     /**< minmuim value of damping coefficient */
  float       fIIRDampingCoefMax;                     /**< maximum value of damping coefficient */
  float       fIIRDampingCoefInit;                    /**< initial value of damping coefficient */

  uint16_t    IIRFilterSize;                          /**< number of filter items */
  float       fIIRFilterInitValue;                    /**< initial value of the filter items */
} CamCalibIIR_t;


/*****************************************************************************/
/**
 * @brief  AwbVersion_t
 */
/*****************************************************************************/
typedef enum CAM_AwbVersion_e
{
  CAM_AWB_VERSION_INVALID 		= 0,        /* invalid */
  CAM_AWB_VERSION_10  			= 1,        /*illuminatant estmation by GMM */
  CAM_AWB_VERSION_11  			= 2,  		/*illuminatant estmation by minimun distance method, using direct white point conditon  */
  CAM_AWB_VERSION_MAX
} CAM_AwbVersion_t;

/*****************************************************************************/
/**
 * @brief   Global AWBV11 calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibAwb_V11_Global_s {
  void*                    p_next;                    /**< for adding to a list */

  CamAwbProfileName_t     name;                       /**< profile name */
  CamResolutionName_t     resolution;                 /**< resolution link */

  //Cam3x1FloatMatrix_t     SVDMeanValue;
  //Cam3x2FloatMatrix_t     PCAMatrix;

  CamCenterLine_t         CenterLine;
  Cam1x1FloatMatrix_t     KFactor;

  uint16_t                AwbClipEnable;
  CamAwbClipParm_t        AwbClipParam;               /**< clipping parameter in Rg/Bg space */
  CamAwbGlobalFadeParm_t  AwbGlobalFadeParm;
  CamAwb_V11_Fade2Parm_t  AwbFade2Parm;

  float                   fRgProjIndoorMin;
  float                   fRgProjOutdoorMin;
  float                   fRgProjMax;
  float                   fRgProjMaxSky;

  uint16_t                fRgProjYellowLimitEnable;   //oyyf
  float                   fRgProjALimit;    //oyyf
  float                   fRgProjAWeight;   //oyyf
  float                   fRgProjYellowLimit;   //oyyf
  uint16_t                fRgProjIllToCwfEnable;    //oyyf
  float                   fRgProjIllToCwf;    //oyyf
  float                   fRgProjIllToCwfWeight;  //oyyf

  CamIlluminationName_t   outdoor_clipping_profile;

  float                   fRegionSize;
  float                   fRegionSizeInc;
  float                   fRegionSizeDec;

  CamCalibIIR_t           IIR;
} CamCalibAwb_V11_Global_t;

/*****************************************************************************/
/**
 * @brief   Global AWB_V10 calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibAwb_V10_Global_s {
  void*                    p_next;                    /**< for adding to a list */

  CamAwbProfileName_t     name;                       /**< profile name */
  CamResolutionName_t     resolution;                 /**< resolution link */

  Cam3x1FloatMatrix_t     SVDMeanValue;
  Cam3x2FloatMatrix_t     PCAMatrix;
  CamCenterLine_t         CenterLine;
  Cam1x1FloatMatrix_t     KFactor;

  CamAwbClipParm_t        AwbClipParam;               /**< clipping parameter in Rg/Bg space */
  CamAwbGlobalFadeParm_t  AwbGlobalFadeParm;
  CamAwb_V10_Fade2Parm_t  AwbFade2Parm;

  float                   fRgProjIndoorMin;
  float                   fRgProjOutdoorMin;
  float                   fRgProjMax;
  float                   fRgProjMaxSky;

  uint16_t        fRgProjYellowLimitEnable;   //oyyf
  float           fRgProjALimit;    //oyyf
  float         fRgProjAWeight;   //oyyf
  float           fRgProjYellowLimit;   //oyyf
  uint16_t        fRgProjIllToCwfEnable;    //oyyf
  float         fRgProjIllToCwf;    //oyyf
  float         fRgProjIllToCwfWeight;  //oyyf

  CamIlluminationName_t   outdoor_clipping_profile;

  float                   fRegionSize;
  float                   fRegionSizeInc;
  float                   fRegionSizeDec;

  CamCalibIIR_t           IIR;
} CamCalibAwb_V10_Global_t;

/*****************************************************************************/
/**
 * @brief   AWB Version11 para
 */
/*****************************************************************************/
typedef struct CamAwbPara_V11_s{
	List						awb_global; 	/**< list of supported awb_globals */
	List						illumination;	/**< list of supported illuminations */

}CamAwbPara_V11_t;

/*****************************************************************************/
/**
 * @brief   AWB Version11 para
 */
/*****************************************************************************/
typedef struct CamAwbPara_V10_s{
	List						awb_global; 	/**< list of supported awb_globals */
	List						illumination;	/**< list of supported illuminations */

}CamAwbPara_V10_t;
/*****************************************************************************/
/**
 * @brief   AWB profile
 */
/*****************************************************************************/
typedef struct CamCalibAwbPara_s{
	CAM_AwbVersion_t valid_version;
	CamAwbPara_V10_t Para_V10;
	CamAwbPara_V11_t Para_V11;
}CamCalibAwbPara_t;


/*****************************************************************************/
/**
 * @brief   ECM scheme
 */
/*****************************************************************************/
typedef struct CamEcmScheme_s {
  void*                    p_next;                    /**< for adding to a list */

  CamEcmSchemeName_t      name;                       /**< scheme name */

  float                   OffsetT0Fac;                /**< start flicker avoidance above OffsetT0Fac * Tflicker integration time */
  float                   SlopeA0;                    /**< slope of gain */
} CamEcmScheme_t;



/*****************************************************************************/
/**
 * @brief   ECM profile
 */
/*****************************************************************************/
typedef struct CamEcmProfile_s {
  void*                    p_next;                    /**< for adding to a list */

  CamEcmProfileName_t     name;                       /**< profile name => serves as resolution link as well */

  List                    ecm_scheme;                 /**< list of ECM schemes; at least one item is expected */
} CamEcmProfile_t;

/*****************************************************************************/
/**
 *          AecSemMode_t
 *
 * @brief   mode type of AEC Scene Evaluation
 *
 */
/*****************************************************************************/
typedef enum AecSemMode_e {
  AEC_SCENE_EVALUATION_INVALID    = 0,        /* invalid (only used for initialization) */
  AEC_SCENE_EVALUATION_DISABLED   = 1,        /* Scene Evaluation disabled (fix setpoint) */
  AEC_SCENE_EVALUATION_FIX        = 2,        /* Scene Evaluation fix (static ROI) */
  AEC_SCENE_EVALUATION_ADAPTIVE   = 3,        /* Scene Evaluation adaptive (ROI caluclated by Scene Evaluation */
  AEC_SCENE_EVALUATION_MAX
} AecSemMode_t;

/*****************************************************************************/
/**
 *          AecHdrMode_t
 *
 * @brief   mode type of Hdr-AEC
 *
 */
/*****************************************************************************/
typedef enum AecHdrMode_e {
  AEC_HDR_MODE_INVALID    = 0,        /* invalid (only used for initialization) */
  AEC_HDR_MODE_STAGGER   = 1,        /* HDR DCG MODE (only one integration time value) */
  AEC_HDR_MODE_DCG    = 2,        /* HDR STAGGER MODE (independent integration time value) */
  AEC_HDR_MODE_MAX
} AecHdrMode_t;


/*****************************************************************************/
/**
 * @brief   Matrix coefficients
 *
 *          | 0 | 1 |  2 |3| 4 |  5 |
 *
 * @note    Coefficients are represented as float numbers
 */
/*****************************************************************************/
typedef struct Cam6x1FloatMatrix {
  float fCoeff[6];
} Cam6x1FloatMatrix_t;

typedef struct CamAECGridWeight_s
{
  uint8_t   *pWeight;
  uint16_t  ArraySize;
}CamAECGridWeight_t;

#if 1
/*****************************************************************************/
typedef enum CamECMMode_e {
  CAM_ECM_MODE_INVALID     = 0,    /**< invalid exposure conversion module */
  CAM_ECM_MODE_1   = 1,  /* the original exposure conversion module ,   minimize the value of gain   */
  CAM_ECM_MODE_2    = 2,    /* the newl exposure conversion module ,imcrease the frame rate */
  CAM_ECM_MODE_MAX,
} CamECMMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure CamerIC ISP exposure measuring mode.
 *
 *****************************************************************************/
typedef enum CamExpMeasuringMode_e {
  CAM_EXP_MEASURING_MODE_INVALID    = 0,    /**< invalid histogram measuring mode   */
  CAM_EXP_MEASURING_MODE_1          = 1,    /**< Y = (R + G + B) x (85/256)         */
  CAM_EXP_MEASURING_MODE_2          = 2,    /**< Y = 16 + 0.25R + 0.5G + 0.1094B    */
  CAM_EXP_MEASURING_MODE_MAX,
} CamExpMeasuringMode_t;

typedef enum CamHistMode_e {
  CAM_HIST_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
  CAM_HIST_MODE_RGB_COMBINED  = 1,    /**< RGB combined histogram */
  CAM_HIST_MODE_R             = 2,    /**< R histogram */
  CAM_HIST_MODE_G             = 3,    /**< G histogram */
  CAM_HIST_MODE_B             = 4,    /**< B histogram */
  CAM_HIST_MODE_Y             = 5,    /**< luminance histogram */
  CAM_HIST_MODE_MAX,                                  /**< upper border (only for an internal evaluation) */
} CamHistMode_t;

#endif

typedef struct CamCalibAecInterAdjust_s{
	uint8_t enable;
	float	dluma_high_th;
	float	dluma_low_th;
	uint32_t trigger_frame;
}CamCalibAecInterAdjust_t;

typedef struct CamCalibAecGainRange_s{
	int array_size;
	float *pGainRange;
}CamCalibAecGainRange_t;

typedef struct CamCalibAecDynamicSetpoint_s{
  void*                    p_next;                    /**< for adding to a list */
  CamDynamicSetpointName_t      name;                       /**name */
  uint8_t	enable;
  float* pExpValue;
  float* pDySetpoint;
  int	array_size;
}CamCalibAecDynamicSetpoint_t;


typedef struct CamCalibAecExpSeparate_s{
  void*                    	p_next;                    /**< for adding to a list */
  CamExpSeparateName_t      name;                       /**name */
  Cam6x1FloatMatrix_t 		ecmTimeDot;
  Cam6x1FloatMatrix_t 		ecmGainDot;
  Cam6x1FloatMatrix_t     	ecmLTimeDot;
  Cam6x1FloatMatrix_t     	ecmLGainDot;
  Cam6x1FloatMatrix_t     	ecmSTimeDot;
  Cam6x1FloatMatrix_t     	ecmSGainDot;
}CamCalibAecExpSeparate_t;

typedef struct CamCalibAecNLSC_s{
	uint8_t enable;
	float IR_rg;
	float IR_bg;
	float Max_dis;
	uint8_t C2B_count;
	float C2B_thresh;
	uint8_t B2C_count;
	float B2C_thresh;
	float B2C_vb_percent;
	uint8_t C2B_stable_fps;
}CamCalibAecNLSC_t;

typedef struct CamCalibAecBacklight_s{
  uint8_t	enable;
  float lumaHighTh;
  float lumaLowTh;
  float weightMinTh;
  float weightMaxTh;
}CamCalibAecBacklight_t;

typedef struct CamCalibAecHist2Hal_s{
  uint8_t	enable;
  uint8_t   lowHistBinTh;
}CamCalibAecHist2Hal_t;
typedef struct CamCalibAecHdrCtrl_s{
  uint8_t   Enable;
  uint8_t	Mode;
  uint8_t	FrameNum;
  float	    DCG_Ratio;
  Cam6x1FloatMatrix_t Lgains;
  Cam6x1FloatMatrix_t LExpLevel;
  Cam6x1FloatMatrix_t LSetPoint;
  Cam6x1FloatMatrix_t TargetDarkROILuma;
  Cam6x1FloatMatrix_t Sgains;
  Cam6x1FloatMatrix_t SExpLevel;
  Cam6x1FloatMatrix_t TargetOELuma;
  Cam6x1FloatMatrix_t SSetPoint;
  float 	OETolerance;
  float		OELumaDistTh;
  float 	M2S_Ratio;
  float 	L2M_Ratio;
}CamCalibAecHdrCtrl_t;

/*****************************************************************************/
/**
 * @brief   Global AEC calibration structure
 */
/*****************************************************************************/
typedef struct CamCalibAecGlobal_s {
  float                   SetPoint;                   /**< set point to hit by the ae control system */
  float					  NightSetPoint;
  float                   ClmTolerance;
  float                   DampOverStill;              /**< damping coefficient for still image mode */
  float                   DampUnderStill;             /**< damping coefficient for still image mode */
  float                   DampOverVideo;              /**< damping coefficient for video mode */
  float                   DampUnderVideo;             /**< damping coefficient for video mode */
  float                   AfpsMaxGain;
  float                   GainFactor;
  float                   GainBias;
  float                   AfpsMaxIntTime;//cxf
  //float                 FrameRateVideo;
  CamAECGridWeight_t      GridWeights;//cxf
  CamAECGridWeight_t      NightGridWeights;//cxf
  float                   MeasuringWinWidthScale;//cxf
  float                   MeasuringWinHeightScale;//cxf
  AecSemMode_t            SemMode; //cxf
  CamHistMode_t           CamerIcIspHistMode;//cxf
  CamExpMeasuringMode_t   CamerIcIspExpMeasuringMode;//cxf

  #if 0
  Cam6x1FloatMatrix_t     EcmTimeDot;
  Cam6x1FloatMatrix_t     EcmGainDot;
  Cam6x1FloatMatrix_t     EcmLTimeDot;
  Cam6x1FloatMatrix_t     EcmLGainDot;
  Cam6x1FloatMatrix_t     EcmSTimeDot;
  Cam6x1FloatMatrix_t     EcmSGainDot;
  #endif
  CamECMMode_t            EcmMode;
  //float                   GainRange[42];

  CamCalibAecGainRange_t  GainRange;
  float                   TimeFactor[4];
  Cam6x1FloatMatrix_t     FpsFixTimeDot;
  uint8_t				  isFpsFix;
  uint8_t				  FpsSetEnable;


  float         AOE_Enable;
  float         AOE_Max_point;
  float         AOE_Min_point;
  float         AOE_Y_Max_th;
  float         AOE_Y_Min_th;
  float         AOE_Step_Inc;
  float         AOE_Step_Dec;

  uint8_t       DON_Night_Trigger;
  uint8_t       DON_Night_Mode;
  float         DON_Day2Night_Fac_th;
  float         DON_Night2Day_Fac_th;
  uint8_t       DON_Bouncing_th;

  CamCalibAecInterAdjust_t InterAdjustStrategy;
  /*zlj add for LockAE*/
  List DySetpointList;
  List ExpSeparateList;
  CamCalibAecNLSC_t NLSC_config;
  CamCalibAecBacklight_t backLightConf;
  CamCalibAecHist2Hal_t hist2Hal;

  uint8_t		LockAE_enable;
  Cam3x1FloatMatrix_t GainValue;
  Cam3x1FloatMatrix_t TimeValue;
  /*zlj add for HdrCtrl*/
  CamCalibAecHdrCtrl_t HdrCtrl;
} CamCalibAecGlobal_t;

/*******************************************************************************
 *
 * @AF macro define
 *
 *******************************************************************************/


/*******************************************************************************
 *
 * @struct  CamCalibAfSearchDir_t
 *
 * @brief   camera calibration AF search direction structure
 *
 *******************************************************************************/
typedef enum CamCalibAfSearchDir_e {
  CAM_AFM_POSITIVE_SEARCH     = 0,
  CAM_AFM_NEGATIVE_SEARCH     = 1,
  CAM_AFM_ADAPTIVE_SEARCH     = 2
} CamCalibAfSearchDir_t;

/*******************************************************************************
 *
 * @struct  AfSearchStrategy_t
 *
 * @brief   AF search strategy
 *
 *******************************************************************************/
typedef enum CamCalibAfSS_e {
  CAM_AFM_FSS_INVALID         = 0,
  CAM_AFM_FSS_FULLRANGE       = 1,    /**< scan the full focus range to find the point of best focus */
  CAM_AFM_FSS_HILLCLIMBING    = 2,    /**< use hillclimbing search */
  CAM_AFM_FSS_ADAPTIVE_RANGE  = 3,    /**< similar to full range search, but with multiple subsequent scans
                                         with decreasing range and step size will be performed. */
  CAM_AFM_FSS_MAX,
} CamCalibAfSS_t;

typedef struct CamCalibContrastAf_s {
  uint8_t                 enable;
  CamCalibAfSS_t          Afss;                                   /**< enumeration type for search strategy */
  CamCalibAfSearchDir_t   FullDir;
  uint8_t                 FullSteps;
  uint16_t*               FullRangeTbl;                           /**< full range search table*/
  CamCalibAfSearchDir_t   AdaptiveDir;
  uint8_t                 AdaptiveSteps;
  uint16_t*               AdaptRangeTbl; 	                      /**< adaptive range search table*/
  
  float 				  TrigThers;							  /**< AF trigger threshold */
  uint16_t				  TrigValue;							  /**< AF trigger Value */
  uint16_t				  TrigFrames;							  /**< AF trigger status must hold frames */
  uint16_t				  TrigAntiFlash;						  /**< AF trigger anti one or some figures flash but not target figures*/

  float 				  FirstStableThers; 					  /**< first time AF stable threshold */
  uint16_t				  FirstStableValue; 					  /**< first time AF stable value */
  uint16_t				  FirstStableFrames;					  /**< first time AF stable status must hold frames */
  uint16_t				  FirstStableTime;						  /**< first time AF stable status must hold time */

  float 				  StableThers;							  /**< AF stable threshold */
  uint16_t				  StableValue;							  /**< AF stable value */
  uint16_t				  StableFrames; 						  /**< AF stable  status must hold frames */
  uint16_t				  StableTime;							  /**< AF stable status must hold time */

  float 				  FinishThersMain;						  /**< AF find clearest position main thershold*/
  float 				  FinishThersSub;						  /**< AF find clearest position subject thershold*/
  uint16_t				  FinishThersOffset;					  /**< AF find clearest position offset thershold*/

  float   				  OutFocusValue;						  /**< out of focus vlaue*/
  uint16_t				  OutFocusLuma; 						  /**< out of focus luma*/
  uint16_t				  OutFocusPos;							  /**< out of focus position*/

  uint16_t				  AfHyst[2][5]; 						  /**< the difference between go ahead and go back*/
  uint16_t				  AfBackStep[5];						  /**< avoid a step is too large*/
  
  unsigned int			  rev[16];								  /**< reserve some char*/
} CamCalibContrastAf_t;

typedef struct CamCalibLaserAf_s {
  uint8_t enable;
  float vcmDot[7];
  float distanceDot[7];
} CamCalibLaserAf_t;

typedef struct CamCalibPdaf_s {
  uint8_t enable;  
} CamCalibPdaf_t;

typedef struct CamCalibAfGlobal_s {
  uint8_t		       Window_Num;
  struct Cam_Win	   WindowA;
  struct Cam_Win	   WindowB;
  struct Cam_Win	   WindowC;
  CamCalibContrastAf_t contrast_af;
  CamCalibLaserAf_t    laser_af;
  CamCalibPdaf_t       pdaf;
} CamCalibAfGlobal_t;
typedef struct CamCalibGocProfile_s {
  void*                    p_next;
  CamGOCProfileName_t name;

  uint16_t enable_mode;
  uint16_t def_cfg_mode;
  uint16_t GammaY[34];
  uint16_t WdrOn_GammaY[34];

} CamCalibGocProfile_t;

typedef struct CamCalibWdrMaxGainLevelCurve_s {
  uint16_t nSize;
  uint16_t filter_enable;
  float*  pfSensorGain_level;
  float*  pfMaxGain_level;
} CamCalibWdrMaxGainLevelCurve_t;

typedef struct CamCalibWdrGlobal_s {
  uint16_t                   Enabled;
  uint16_t                   Mode;
  uint16_t                   LocalCurve[33];
  uint16_t                   GlobalCurve[33];
  uint16_t                   wdr_noiseratio;
  uint16_t                   wdr_bestlight;
  uint32_t                   wdr_gain_off1;
  uint16_t                   wdr_pym_cc;
  uint8_t                    wdr_epsilon;
  uint8_t                    wdr_lvl_en;
  uint8_t                    wdr_flt_sel;
  uint8_t                    wdr_gain_max_clip_enable;
  uint8_t                    wdr_gain_max_value;
  uint8_t                    wdr_bavg_clip;
  uint8_t                    wdr_nonl_segm;
  uint8_t                    wdr_nonl_open;
  uint8_t                    wdr_nonl_mode1;
  uint32_t                   wdr_coe0;
  uint32_t                   wdr_coe1;
  uint32_t                   wdr_coe2;
  uint32_t                   wdr_coe_off;

  CamCalibWdrMaxGainLevelCurve_t  wdr_MaxGain_Level_curve;
} CamCalibWdrGlobal_t;

/*****************************************************************************/
/**
 * @brief   System data structure.
 */
/*****************************************************************************/
typedef struct CamCalibSystemData_s {
  bool_t                  AfpsDefault;
} CamCalibSystemData_t;

enum CamCprocUseCase_e {
  CAM_CPROC_USECASE_PREVIEW,
  CAM_CPROC_USECASE_CAPTURE,
  CAM_CPROC_USECASE_VIDEO,
};


typedef struct  CamCprocData_s {
  char  name[10];
  enum CamCprocUseCase_e cproc_scenario;
  /*range:  0.0  - 1.992*/
  float cproc_contrast;
  /* -90 deg - +87.188*/
  float cproc_hue;
  /*range : -128  - +127 */
  signed char cproc_brightness;
  /*range:  0.0  - 1.992*/
  float cproc_saturation;
} CamCprocData_t;

typedef struct CamCprocProfile_s {
  int num_items;
  struct  CamCprocData_s cproc[3];
} CamCprocProfile_t;

enum LIGHT_MODE {
	LIGHT_MODE_MIN = -1,
	LIGHT_MODE_DAY = 0,
	LIGHT_MODE_NIGHT = 1,
	LIGHT_MODE_MAX = 2,
};

enum NIGHT_TRIGGER_MODE
{
    TRIGGER_OFF = 0,
    LIGHT_SENS = 1,
    NO_LIGHT_SENS = 2,
    NO_LIGHT_SENS_IRCUT = 3,
};

enum NIGHT_MODE
{
    NORMAL = 1,
    WHITE_BLACK = 2  
};

typedef struct camOTPAwbInfo_s
{
	uint8_t  enable;
	uint32_t golden_r_value;
	uint32_t golden_gr_value;
	uint32_t golden_gb_value;
	uint32_t golden_b_value;
	
}camOTPAwbInfo_t;

typedef struct camOTPLscInfo_s
{
	uint8_t  enable;
	
}camOTPLscInfo_t;


typedef struct CamOTPGlobal_s{
	camOTPAwbInfo_t awb;
	camOTPLscInfo_t lsc;
	
}CamOTPGlobal_t;

#ifdef __cplusplus
}
#endif

/* @} cam_types */

#endif /* __CAM_TYPES_H__ */

