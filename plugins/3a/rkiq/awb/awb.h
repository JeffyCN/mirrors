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
#ifndef __AWB_H__
#define __AWB_H__

/**
 * @file awb.h
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
 * @defgroup AWBM Auto white Balance Module
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
#include <cam_ia_api/cameric.h>
#include <cam_calibdb/cam_calibdb_api.h>
#include <base/xcam_log.h>

//#include "awb_independent.h"

/*
 ***************** AWB LIB VERSION NOTE *****************
 * v0.0.1
 *  - platform independence, support rkisp v10 and v12
 * v0.0.2
 *  - remove unnecessary lib dependancy
 * v0.0.3
 *  - unify isp v10, v12 LSC parameter
 * v0.0.4
 *  - sync awb with calibdb v0.2.0
 * v0.0.5
 *  - suport illu BW
 * v0.0.6
 *  - change LOGMIN from 0.0001f to 0.00001f to avoid out of
 *    range error.
 *  - no need to destroy awb context when some not fatal
 *    errors happen
 * v0.0.7
 *  - multiply wbgain by blsgain
 * v0.0.8
 *  - use the current effecting isp params along with the stats
 *  - to calculate next awb params
 * v0.0.9
* 1) enable ALOGV/ALOGW for Android
 * v0.0.a
 * 1) mdoify awb converged threshold
  *2) only awb is converged and wp number bigger than threshold ,then
        enter AwbSetValues function to update awb value to IC register.
 * v0.0.b
 * 1) fix output wrong ccoffset when awb converged
 * v0.0.c
 * 1) support flash ,
 *    only one led is used in flash light
 * v0.0.d
 * - support awb_v11
  * v0.0.e
 * - IQ_Tool v2.2 support awb_v11
 */

#define CONFIG_AWB_LIB_VERSION "v0.0.e"


#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/**
 * @brief
 */
/*****************************************************************************/
typedef struct AwbContext_s* AwbHandle_t;           /**< handle to AWB context */



/*****************************************************************************/
/**
 * @brief
 */
/*****************************************************************************/
typedef enum AwbWorkingFlags_e {
  AWB_WORKING_FLAG_USE_DAMPING        = 0x01,
  AWB_WORKING_FLAG_USE_CC_OFFSET      = 0x02
} AwbWorkingFlags_t;



/*****************************************************************************/
/**
 * @brief
 *
 */
/*****************************************************************************/
typedef enum AwbRunMode_e {
  AWB_MODE_INVALID                    = 0,        /**< initialization value */
  AWB_MODE_MANUAL                     = 1,        /**< run manual white balance */
  AWB_MODE_AUTO                       = 2,        /**< run auto white balance */
  AWB_MODE_MAX
} AwbMode_t;



/*****************************************************************************/
/**
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as float numbers.
 */
/*****************************************************************************/
typedef struct AwbGains_s {
  float fRed;         /**< gain value for the red channel */
  float fGreenR;      /**< gain value for the green channel in red lines */
  float fGreenB;      /**< gain value for the green channel in blue lines */
  float fBlue;        /**< gain value for the blue channel */
} AwbGains_t;


/******************************************************************************/
/**
 *          WbGainsOverG_t
 *
 * @brief   context structure for function interpolation function Interpolate
 *
 ******************************************************************************/
typedef struct WbGainsOverG_s {
  float GainROverG;                           /**< (Gain-Red / Gain-Green) */
  float GainBOverG;                           /**< (Gain-Blue / Gain-Green) */
} WbGainsOverG_t;


/*****************************************************************************/
/**
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as signed numbers.
 */
/*****************************************************************************/
typedef struct AwbXTalkOffset_s {
  float fRed;         /**< value for the red channel */
  float fGreen;       /**< value for the green channel in red lines */
  float fBlue;        /**< value for the blue channel */
} AwbXTalkOffset_t;

enum AwbMeasuringResultType_e {
  RESULT_RGB,
  RESULT_YCbCr
};

/*****************************************************************************/
/**
 * @brief   This macro defines the number of used bins.
 *
 *****************************************************************************/
#define AWB_HIST_NUM_BINS           32  /**< number of bins */

/*****************************************************************************/
/**
 * @brief   This typedef represents the histogram which is measured by the
 *          CamerIC ISP histogram module.
 *
 *****************************************************************************/
typedef uint32_t AWBHistBins_t[AWB_HIST_NUM_BINS];

typedef struct AwbMeasuringResult_s {
  uint32_t    NoWhitePixel;           /**< number of white pixel */
  float     MeanY__G;               /**< Y/G  value in YCbCr/RGB Mode */
  float     MeanCb__B;              /**< Cb/B value in YCbCr/RGB Mode */
  float     MeanCr__R;              /**< Cr/R value in YCbCr/RGB Mode */
  enum AwbMeasuringResultType_e type;
} AwbMeasuringResult_t;

/*****************************************************************************/
/**
 * @brief This enum type specifies the different possible states of the Flash in AWB.
 *
 */
/*****************************************************************************/
typedef enum AwbFlashState_e {
  AWB_AE_FLASH_INVALID      = 0,        /**< initialization value */
  AWB_AE_FLASH_OFF          = 1,      /*all flash are off*/
  AWB_AE_FLASH_PRE_ON       = 2,      /*pre flash is on*/
  AWB_AE_FLASH_MAIN_ON       = 3,     /*main flash is on*/
  AWB_AE_STATE_MAX
} AwbFlashState_t;

typedef enum AwbFrameStatus_e{
  AWB_FRAME_STATUS_OK,
  AWB_FRAME_STATUS_CORRUPTED,
  AWB_FRAME_STATUS_FLASH_EXPOSED,   /*pre flash is fullly open*/
  AWB_FRAME_STATUS_FLASH_PARTIAL,   /*pre flash is partial open*/
  AWB_FRAME_STATUS_FLASH_FAILED,    /*pre flash failed to  open*/
} AwbFrameStatus_t;


typedef struct AwbRunningInputParams_s {
  AwbMeasuringResult_t            MesureResult;
  //histogram
  AWBHistBins_t                   HistBins;
  int32_t                         DominateIlluProfileIdx; /**< current DominateIlluProfileIdx from hardware */
  AwbGains_t                      Gains;          /**< current gains from hardware */

  Cam3x3FloatMatrix_t             CtMatrix;        /**< current cross talk matrix from hardware */
  AwbXTalkOffset_t                CtOffset;        /**< current cross talk offset from hardware */

  float                           fGain;
  float                           fIntegrationTime;

 //for flash
  AwbFlashState_t                 flashModeSetting;  /*store flash state*/
  bool                            aeConverge;       /*AEC converged flag*/
  float                           meanLuma;         /*mean luma calculatiion by AEC*/
  AwbFrameStatus_t                frame_status;    /*store falsh open state*/


} AwbRunningInputParams_t;

enum AwbReconfigParams_e {
  AWB_RECONFIG_NONE,
  AWB_RECONFIG_GAINS        = 0x1,
  AWB_RECONFIG_CCMATRIX     = 0x1 << 1,
  AWB_RECONFIG_CCOFFSET     = 0x1 << 2,
  AWB_RECONFIG_LSCMATRIX    = 0x1 << 3,
  AWB_RECONFIG_LSCSECTOR    = 0x1 << 4,
  AWB_RECONFIG_MEASMODE     = 0x1 << 5,
  AWB_RECONFIG_MEASCFG      = 0x1 << 6,
  AWB_RECONFIG_AWBWIN       = 0x1 << 7,
  AWB_RECONFIG_DOMILLIDX    = 0x1 << 8,

};
typedef struct AwbRunningOutputResult_s {
  uint32_t                          validParam;
  int32_t                           DominateIlluProfileIdx;
  AwbGains_t                        WbGains;
  Cam3x3FloatMatrix_t               CcMatrix;             /**< damped color correction matrix */
  Cam1x3FloatMatrix_t               CcOffset;             /**< damped color correction offset */
  CamLscMatrix_t                    LscMatrixTable;       /**< damped lsc matrix */
  CamerIcIspLscSectorConfig_t       SectorConfig;               /**< lsc grid */
  CamerIcIspAwbMeasuringMode_t      MeasMode;           /**< specifies the means measuring mode (YCbCr or RGB) */
  CamerIcAwbMeasuringConfig_t       MeasConfig;         /**< measuring config */
  Cam_Win_t                         awbWin;
  uint8_t                           DoorType;
  bool                              converged;
  int                               err_code;
  char                              IllName[20];
  CamCcProfileName_t                CcNameUp;
  CamCcProfileName_t                CcNameDn;
  CamLscProfileName_t               LscNameUp;
  CamLscProfileName_t               LscNameDn;
  bool_t                            forceWbGainFlag;
  AwbGains_t                        forceWbGains;
  bool_t                            forceIlluFlag;
  CamIlluminationName_t             forceIllName;
  float                             RgProj;
  WbGainsOverG_t                    WbGainsOverG;
  WbGainsOverG_t                    WbClippedGainsOverG;
  float                             RegionSize;
  Cam1x4FloatMatrix_t               refWbgain;
  CamIlluminationName_t             curIllName;
  int                               Region;
  float                             ExpPriorIn;
  float                             ExpPriorOut;
  float                             likehood[32];
  float                             weight[32];
  float                             Wb_s;
  float                             Wb_s_max1;
  float                             Wb_s_max2;
  float                             Wb_bg;
  float                             Wb_rg;

} AwbRunningOutputResult_t;


/*****************************************************************************/
/**
 *          AwbInstanceConfig_t
 *
 * @brief   AWB Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AwbInstanceConfig_s {
  AwbHandle_t                     hAwb;               /**< handle returns by AwbInit() */
  int                             isp_ver;
} AwbInstanceConfig_t;

enum AwbTuningParam_e{
    AWB_TUNING_DISABLE = 0,
    AWB_TUNING_ENABLE = 0x1f
};

typedef struct AwbTuningConfig_s{
    uint8_t    forceGainSet;
    uint8_t    forceMeasSet;
    bool_t     forceGainEnable;
    AwbGains_t forceGains;
    bool_t     forceIlluEnable;
    char       ill_name[20];
    bool_t     forceMeasuredFlag;
    AwbMeasuringResult_t forceMeasuredMeans;
    uint8_t    forceUpdateAwb;
}AwbTuningConfig_t;


/*****************************************************************************/
/**
 *          AwbConfig_t
 *
 * @brief   AWB Module configuration structure
 *
 *****************************************************************************/
typedef struct AwbConfig_s {
  AwbMode_t                         Mode;               /**< White Balance working mode (MANUAL | AUTO) */
  uint32_t                          idx;
  bool_t                            damp;

  uint16_t                          width;              /**< picture width */
  uint16_t                          height;             /**< picture height */
  float                             framerate;          /**< frame rate */
  Cam_Win_t                         awbWin;
  uint32_t                          Flags;              /**< working flags (@see AwbWorkingFlags_e) */
  CamCalibDbHandle_t                hCamCalibDb;        /**< calibration database handle */
  CamerIcIspAwbMeasuringMode_t      MeasMode;           /**< specifies the means measuring mode (YCbCr or RGB) */
  CamerIcAwbMeasuringConfig_t       MeasConfig;         /**< measuring config */
  float                             fStableDeviation;   /**< min deviation in percent to enter stable state */
  float                             fRestartDeviation;  /**< max tolerated deviation in precent for staying in stable state */
  uint8_t                           validHistBinsNum;
  AwbTuningConfig_t                 awbTuning;
} AwbConfig_t;



/*****************************************************************************/
/**
 *          AwbRgProj_t
 *
 * @brief   AWB Projection Borders in R/G Layer
 *
 *****************************************************************************/
typedef struct AwbRgProj_s {
  float                             fRgProjIndoorMin;
  float                             fRgProjOutdoorMin;
  float                             fRgProjMax;
  float                             fRgProjMaxSky;

  float                             fRgProjALimit;    //oyyf
  float                             fRgProjAWeight;   //oyyf
  float                             fRgProjYellowLimit;   //oyyf
  float                             fRgProjIllToCwf;    //oyyf
  float                             fRgProjIllToCwfWeight;  //oyyf
} AwbRgProj_t;

/*****************************************************************************/
/**
 *          AwbWhitePoint_t
 *
 * @brief
 *
 *****************************************************************************/
typedef struct AwbWhitePoint_s {
  uint16_t win_h_offs;
  uint16_t win_v_offs;
  uint16_t win_width;
  uint16_t win_height;
  uint8_t awb_mode;
  uint32_t cnt;
  uint8_t mean_y;
  uint8_t mean_cb;
  uint8_t mean_cr;
  uint16_t mean_r;
  uint16_t mean_b;
  uint16_t mean_g;

  uint8_t RefCr;
  uint8_t RefCb;
  uint8_t MinY;
  uint8_t MaxY;
  uint8_t MinC;
  uint8_t MaxCSum;

  float RgProjection;
  float RegionSize;
  float Rg_clipped;
  float Rg_unclipped;
  float Bg_clipped;
  float Bg_unclipped;
} AwbWhitePoint_t;


/*****************************************************************************/
/**
 * @brief   This function initializes the Auto White Balance Module.
 *
 * @param   handle      AWB instance handle
 * @param   pInstConfig pointer instance configuration structure
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT AwbInit
(
    AwbInstanceConfig_t* pInstConfig
);

/******************************************************************************
 * AwbConfigure()
 *****************************************************************************/
RESULT AwbConfigure
(
    AwbHandle_t handle,
    AwbConfig_t* pConfig
);

/******************************************************************************
 * AwbReConfigure()
 *****************************************************************************/
RESULT AwbReConfigure
(
    AwbHandle_t handle,
    AwbConfig_t* pConfig
);



/*****************************************************************************/
/**
 * @brief   The function releases/frees the Auto White Balance module.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbRelease
(
    AwbHandle_t handle
);


/*****************************************************************************/
/**
 * @brief   This function returns true if the AWB hit the convergence criteria.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT AwbStable
(
    AwbHandle_t handle,
    bool_t*      pStable
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbStart
(
    AwbHandle_t         handle,
    AwbConfig_t*     pcfg
);



/*****************************************************************************/
/**
 * @brief   The function stops the auto white balance.
 *
 * @param   handle      AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbStop
(
    AwbHandle_t         handle
);


/*****************************************************************************/
/**
 * @brief   The function returns current status values of the AWB.
 *
 * @param   handle      AWB instance handle
 * @param   pRunning    pointer to return current run-state of AWB module
 * @param   pMode       pointer to return current operation mode of AWB module
 * @param   pIlluIdx    pointer to return current start profile index
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbStatus
(
    AwbHandle_t     handle,
    bool_t*          pRunning,      /**< BOOL_TRUE: running, BOOL_FALSE: stopped */
    AwbMode_t*       pMode,
    uint32_t*        pIlluIdx,
    AwbRgProj_t*     pRgProj
);


/******************************************************************************
 * AwbSettled()
 *****************************************************************************/
RESULT AwbSettled
(
    AwbHandle_t handle,
    bool_t*      pSettled,
    uint32_t*  pDNoWhitePixel
);

/*****************************************************************************/
/**
 * @brief   The function starts AWB processing
 *
 * @param   handle      AWB instance handle
 * @param   pMeasResult pointer tu current AWB measuring data from hardware
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbRun
(
    AwbHandle_t                         handle,
    const AwbRunningInputParams_t* pMeasResult,
    AwbRunningOutputResult_t* pOutResult
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbTryLock
(
    AwbHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AWB instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AwbUnLock
(
    AwbHandle_t handle
);

RESULT AwbSetForceGains
(
    AwbHandle_t handle,
    AwbConfig_t *pConfig
);

RESULT AwbSetWhitePoint
(
    AwbHandle_t handle,
    AwbConfig_t *pConfig
);
#ifdef __cplusplus
}
#endif

/* @} AWBM */


#endif /* __AWB_H__*/
