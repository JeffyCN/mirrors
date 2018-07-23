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
#ifndef __AF_H__
#define __AF_H__

/**
 * @file af.h
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
 * @defgroup AFM Auto Focus Module
 * @{
 *
 */
#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
#include <oslayer/oslayer.h>
#include <common/list.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct AfContext_s* AfHandle_t;     /**< handle to AEC context */

#define AF_TYPE_CONTRAST (1 << 0)
#define AF_TYPE_LASER (1 << 1)
#define AF_TYPE_PDAF (1 << 2)

/*******************************************************************************
 *
 *          AfSearchStrategy_t
 *
 * @brief   CamerIc AF Module search strategy
 *
 */
typedef enum AfSearchStrategy_e {
  AFM_FSS_INVALID         = 0,
  AFM_FSS_FULLRANGE       = 1,    /**< scan the full focus range to find the point of best focus */
  AFM_FSS_HILLCLIMBING    = 2,    /**< use hillclimbing search */
  AFM_FSS_ADAPTIVE_RANGE  = 3,    /**< similar to full range search, but with multiple subsequent scans
                                         with decreasing range and step size will be performed. */
  AFM_FSS_MAX,
} AfSearchStrategy_t;



/*******************************************************************************
 *
 *          AfSearchTriggerMode_t
 *
 * @brief   CamerIc AF Module search strategy
 *
 */
typedef enum AfSearchTriggerMode_e {
  AFM_FSTM_INVALID         = 0,
  AFM_FSTM_ONE_SHOT        = 1,   /**< (re-)start a one-shot search */
  AFM_FSTM_CONTINUOUSlY    = 2,   /**< (re-)start a continous search */
  AFM_FSTM_STOPPED         = 3,
  AFM_FSTM_MAX,
} AfSearchTriggerMode_t;

typedef enum CamerIcIspAfmWindowId_e {
  CAMERIC_ISP_AFM_WINDOW_INVALID  = 0,	  /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_AFM_WINDOW_A        = 1,    /**< Window A (1st window) */
  CAMERIC_ISP_AFM_WINDOW_B        = 2,    /**< Window B (2nd window) */
  CAMERIC_ISP_AFM_WINDOW_C        = 3,    /**< Window C (3rd window) */
  CAMERIC_ISP_AFM_WINDOW_MAX,             /**< upper border (only for an internal evaluation) */
} CamerIcIspAfmWindowId_t;

typedef enum CamerIcIspVcmMoveRes_e {
  CAMERIC_ISP_VCM_INVAL        = 0,
  CAMERIC_ISP_VCM_MOVE_START   = 1,
  CAMERIC_ISP_VCM_MOVE_RUNNING = 2,
  CAMERIC_ISP_VCM_MOVE_END     = 3
} CamerIcIspAfmVcmMoveRes_t;

/******************************************************************************/
/**
 *          CamerIcEventCb_t
 *
 *  @brief  Event callback
 *
 *  This callback is used to signal something to the application software,
 *  e.g. an error or an information.
 *
 *  @return void
 *
 *****************************************************************************/
typedef struct CamerIcEventCb_s {
  void*                pUserContext;  /**< user context */
} CamerIcEventCb_t;

/******************************************************************************/
/**
 * @struct  CamerIcAfmMeasuringResult_s
 *
 * @brief   A structure to represent a complete set of measuring values.
 *
 *****************************************************************************/
typedef struct LaserMeas_s {
  uint32_t  distance;  /* unit: mm */
  uint32_t  rev[16];
} LaserMeas_t;

typedef struct CamerIcAfmMeasuringResult_s {
  uint32_t    SharpnessA;         /**< sharpness of window A */
  uint32_t    SharpnessB;         /**< sharpness of window B */
  uint32_t    SharpnessC;         /**< sharpness of window C */

  uint32_t    LuminanceA;         /**< luminance of window A */
  uint32_t    LuminanceB;         /**< luminance of window B */
  uint32_t    LuminanceC;         /**< luminance of window C */

  CamerIcIspAfmVcmMoveRes_t MoveRes;
} CamerIcAfmMeasuringResult_t;

typedef struct AfMeas_s {
  LaserMeas_t laser;
  CamerIcAfmMeasuringResult_t cameric;

  uint32_t meas_type;
} AfMeas_t;

/******************************************************************************/
/**
 * @struct  CamerIcAfmOutputResult_s
 *
 * @brief   A structure to represent a complete set of output values.
 *
 *****************************************************************************/
typedef struct CamerIcAfmOutputResult_s {
  int32_t 		 NextLensePos;
  uint32_t		 Window_Num;
  struct Cam_Win WindowA;
  struct Cam_Win WindowB;
  struct Cam_Win WindowC;
} CamerIcAfmOutputResult_t;

/******************************************************************************/
/**
 * @struct  CamerIcAfmType_t
 *
 * @brief 
 *
 *****************************************************************************/
typedef struct CamerIcAfmType {
  unsigned int contrast_af:1;
  unsigned int laser_af:1;
  unsigned int pdaf:1;
} CamerIcAfmType_t;

typedef struct CameraIcContrastAfCfg_s {
  unsigned int rev[16];
} CameraIcContrastAfCfg_t;

typedef struct CameraIcLaserAfCfg_s {
  float vcmDot[7];
  float distanceDot[7];
} CameraIcLaserAfCfg_t;

typedef struct CameraIcPdafCfg_s {
  unsigned int rev[16];
} CameraIcPdafCfg_t;


/*****************************************************************************/
/**
 *          AfInstanceConfig_t
 *
 * @brief   AF Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AfInstanceConfig_s {
  AfHandle_t              hAf;            /**< handle returned by AfInit() */
} AfInstanceConfig_t;

/*****************************************************************************/
/**
 *          AfConfig_t
 *
 * @brief   AF Module configuration structure
 *
 *****************************************************************************/
typedef struct AfConfig_s {
  CamerIcAfmType_t AfType;
  CameraIcContrastAfCfg_t ContrastAf;
  CameraIcLaserAfCfg_t LaserAf;
  CameraIcPdafCfg_t Pdaf;
  
  uint32_t Window_Num;
  struct Cam_Win WindowA;
  struct Cam_Win WindowB;
  struct Cam_Win WindowC;

  int32_t 	  LensePos;
  AfSearchStrategy_t Afss;           /**< focus search strategy */
} AfConfig_t;



typedef enum AfEvtId_e {
  AFM_AUTOFOCUS_MOVE        = 0,  /* <Notify on autofocus start and stop. This is useful in continuous > */
  AFM_AUTOFOCUS_FINISHED    = 1,
} AfEvtId_t;

typedef struct AfMoveEvt_s {
  bool_t start;
} AfMoveEvt_t;

typedef struct AfFinshEvt_s {
  bool_t focus;
} AfFinshEvt_t;

typedef struct AfEvt_s {
  AfEvtId_t              evnt_id;
  union {
    AfMoveEvt_t         mveEvt;
    AfFinshEvt_t        fshEvt;
  } info;
  void*                   pEvntCtx;
} AfEvt_t;


typedef struct AfEvtQue_s {
  List                   list;
  osQueue                queue;
} AfEvtQue_t;

/*****************************************************************************/
/**
 *          AfInit()
 *
 * @brief   This function initializes the Auto Focus Module.
 *
 * @param   pConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_INVALID_PARM
 * @retval  RET_OUTOFMEM
 *
 *****************************************************************************/
RESULT AfInit
(
    AfInstanceConfig_t* pInstConfig
);



/*****************************************************************************/
/**
 *          AfRelease()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfRelease
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 *          AfConfigure()
 *
 * @brief   This function configures the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 * @param   pConfig
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_INVALID_PARM
 * @retval  RET_WRONG_STATE
 * @retval  RET_NOTSUPP         sensor doesn't support AF
 *
 *****************************************************************************/
RESULT AfConfigure
(
    AfHandle_t handle,
    AfConfig_t* pConfig
);



/*****************************************************************************/
/**
 *          AfReConfigure()
 *
 * @brief   This function re-configures the Auto Focus Module
 *          after e.g. resolution change
 *
 * @param   handle  Handle to AFM
 *
 * @return  Returns the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 * @retval  RET_WRONG_HANDLE
 * @retval  RET_WRONG_STATE
 *
 *****************************************************************************/
RESULT AfReConfigure
(
	AfHandle_t handle,
	AfConfig_t* pConfig
);



/*****************************************************************************/
/**
 * @brief   This function returns BOOL_TRUE if the AF is settled.
 *
 * @param   handle      AF instance handle
 * @param   pSettled    pointer to settled value
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfSettled
(
    AfHandle_t handle,
    bool_t*      pSettled
);



/*****************************************************************************/
/**
 *          AfStart()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStart
(
    AfHandle_t                handle,
    const AfSearchStrategy_t  fss
);



/*****************************************************************************/
/**
 *          AfOneShot()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfOneShot
(
    AfHandle_t                handle,
    const AfSearchStrategy_t  fss
);


/*****************************************************************************/
/**
 *          AfStop()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStop
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 *          AfStop()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfStatus
(
    AfHandle_t          handle,
    bool_t*              pRunning,
    AfSearchStrategy_t*  pFss,
    float*               sharpness
);

/******************************************************************************
 * AfMeasureCbRestart()
 *****************************************************************************/
RESULT AfMeasureCbRestart
(
    AfHandle_t                  handle
);

/*****************************************************************************/
/**
 *          AfProcessFrame()
 *
 * @brief   The function releases/frees the Auto Focus Module.
 *
 * @param   handle  Handle to AFM
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfProcessFrame
(
    AfHandle_t                  handle,
    AfMeas_t* pMeasResults
);

/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AF instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfGetResult
(
    AfHandle_t                  handle,
    CamerIcAfmOutputResult_t*   pOutputResults
);


/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AF instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfTryLock
(
    AfHandle_t handle
);



/*****************************************************************************/
/**
 * @brief   The function
 *
 * @param   handle  AF instance handle
 *
 * @return  Return the result of the function call.
 * @retval  RET_SUCCESS
 * @retval  RET_FAILURE
 *
 *****************************************************************************/
RESULT AfUnLock
(
    AfHandle_t handle
);




/******************************************************************************
 * AfShotCheck()
 *****************************************************************************/
RESULT AfShotCheck
(
    AfHandle_t                  handle,
    bool_t*                      shot
);



/******************************************************************************
 * AfRegisterEvtQue
 *****************************************************************************/
RESULT AfRegisterEvtQue
(
    AfHandle_t                  handle,
    AfEvtQue_t*                  evtQue
);

/******************************************************************************
 * AfReset()
 *****************************************************************************/
RESULT AfReset
(
    AfHandle_t                handle,
    int32_t                   len_pos,
    const AfSearchStrategy_t  fss
);

#ifdef __cplusplus
}
#endif

/* @} AFM */


#endif /* __AF_H__*/
