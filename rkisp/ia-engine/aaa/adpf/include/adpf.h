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
#ifndef __ADPF_H__
#define __ADPF_H__

/**
 * @file adpf.h
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
 * @defgroup ADPF Auto denoising pre-filter module
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

#define ADPF_MASK (1 << 0)
#define ADPF_STRENGTH_MASK (1 << 1)
#define ADPF_DENOISE_SHARP_LEVEL_MASK (1 << 2)
#define ADPF_DSP_3DNR_MASK (1 << 3)
#define ADPF_DEMOSAIC_TH_MASK (1 << 4)
#define ADPF_NEW_DSP_3DNR_MASK (1 << 5)
#define ADPF_DEMOSAICLP_MASK  (1 << 6)
#define ADPF_RKIESHARP_MASK  (1 << 7)




#define CAMERIC_DPF_MAX_NLF_COEFFS      17
#define CAMERIC_DPF_MAX_SPATIAL_COEFFS  6

/*****************************************************************************/
/**
 *          AdpfHandle_t
 *
 * @brief   ADPF Module instance handle
 *
 *****************************************************************************/
typedef struct AdpfContext_s* AdpfHandle_t;         /**< handle to ADPF context */



/*****************************************************************************/
/**
 * @brief   A structure/tupple to represent gain values for four (R,Gr,Gb,B)
 *          channels.
 *
 * @note    The gain values are represented as float numbers.
 */
/*****************************************************************************/
typedef struct AdpfGains_s {
  float   fRed;                               /**< gain value for the red channel */
  float   fGreenR;                            /**< gain value for the green channel in red lines */
  float   fGreenB;                            /**< gain value for the green channel in blue lines */
  float   fBlue;                              /**< gain value for the blue channel */
} AdpfGains_t;

/*****************************************************************************/
/**
 *          AdpfInstanceConfig_t
 *
 * @brief   ADPF Module instance configuration structure
 *
 *****************************************************************************/
typedef struct AdpfInstanceConfig_s {
  AdpfHandle_t            hAdpf;              /**< handle returned by AdpfInit() */
} AdpfInstanceConfig_t;

typedef enum CamerIcDpfNoiseLevelLookUpScale_e {
  CAMERIC_NLL_SCALE_INVALID       = -1,        /**< lower border (only for an internal evaluation) */
  CAMERIC_NLL_SCALE_LINEAR,        /**< use a linear scaling */
  CAMERIC_NLL_SCALE_LOGARITHMIC,        /**< use a logarithmic scaling */
  CAMERIC_NLL_SCALE_MAX                       /**< upper border (only for an internal evaluation) */
} CamerIcDpfNoiseLevelLookUpScale_t;

/*****************************************************************************/
/**
 * @brief   This type defines the
 */
/*****************************************************************************/
typedef struct CamerIcDpfInvStrength_s {
  uint8_t WeightR;
  uint8_t WeightG;
  uint8_t WeightB;
} CamerIcDpfInvStrength_t;

typedef struct CamerIcDpfNoiseLevelLookUp_s {
  uint16_t                            NllCoeff[CAMERIC_DPF_MAX_NLF_COEFFS];   /**< Noise-Level-Lookup coefficients */
  CamerIcDpfNoiseLevelLookUpScale_t   xScale;                                 /**< type of x-axis (logarithmic or linear type) */
} CamerIcDpfNoiseLevelLookUp_t;

typedef struct CamerIcDpfSpatial_s {
  uint8_t WeightCoeff[CAMERIC_DPF_MAX_SPATIAL_COEFFS];
} CamerIcDpfSpatial_t;

typedef enum CamerIcDpfGainUsage_e {
  CAMERIC_DPF_GAIN_USAGE_INVALID       = 0,   /**< lower border (only for an internal evaluation) */
  CAMERIC_DPF_GAIN_USAGE_DISABLED      = 1,   /**< don't use any gains in preprocessing stage */
  CAMERIC_DPF_GAIN_USAGE_NF_GAINS      = 2,   /**< use only the noise function gains  from registers DPF_NF_GAIN_R, ... */
  CAMERIC_DPF_GAIN_USAGE_LSC_GAINS     = 3,   /**< use only the gains from LSC module */
  CAMERIC_DPF_GAIN_USAGE_NF_LSC_GAINS  = 4,   /**< use the moise function gains and the gains from LSC module */
  CAMERIC_DPF_GAIN_USAGE_AWB_GAINS     = 5,   /**< use only the gains from AWB module */
  CAMERIC_DPF_GAIN_USAGE_AWB_LSC_GAINS = 6,   /**< use the gains from AWB and LSC module */
  CAMERIC_DPF_GAIN_USAGE_MAX                  /**< upper border (only for an internal evaluation) */
} CamerIcDpfGainUsage_t;

typedef enum CamerIcDpfRedBlueFilterSize_e {
  CAMERIC_DPF_RB_FILTERSIZE_INVALID   = -1,    /**< lower border (only for an internal evaluation) */
  CAMERIC_DPF_RB_FILTERSIZE_13x9      = 0,    /**< red and blue filter kernel size 13x9 (means 7x5 active pixel) */
  CAMERIC_DPF_RB_FILTERSIZE_9x9       = 1,    /**< red and blue filter kernel size 9x9 (means 5x5 active pixel) */
  CAMERIC_DPF_RB_FILTERSIZE_MAX               /**< upper border (only for an internal evaluation) */
} CamerIcDpfRedBlueFilterSize_t;

typedef struct CamerIcDpfConfig_s {
  CamerIcDpfGainUsage_t           GainUsage;              /**< which gains shall be used in preprocessing stage of dpf module */

  CamerIcDpfRedBlueFilterSize_t   RBFilterSize;           /**< size of filter kernel for red/blue pixel */

  bool_t                          ProcessRedPixel;        /**< enable filter processing for red pixel */
  bool_t                          ProcessGreenRPixel;     /**< enable filter processing for green pixel in red lines */
  bool_t                          ProcessGreenBPixel;     /**< enable filter processing for green pixel in blue lines */
  bool_t                          ProcessBluePixel;       /**< enable filter processing for blux pixel */

  CamerIcDpfSpatial_t             SpatialG;               /**< spatial weights for green pixel */
  CamerIcDpfSpatial_t             SpatialRB;              /**< spatial weights for red/blue pixel */
} CamerIcDpfConfig_t;


/*****************************************************************************/
/**
 *          AdpfConfigType_t
 *
 * @brief   ADPF Configuration type
 *
 *****************************************************************************/
typedef enum AdpfConfigType_e {
  ADPF_USE_CALIB_INVALID  = 0,                /**< invalid (could be zeroed memory) */
  ADPF_USE_CALIB_DATABASE = 1,
  ADPF_USE_DEFAULT_CONFIG = 2
} AdpfConfigType_t;

typedef enum AdpfMode_e {
  ADPF_MODE_INVALID = 0,
  ADPF_MODE_CONTROL_BY_GAIN = 1,
  ADPF_MODE_CONTROL_EXT = 2
} AdpfMode_t;


/*****************************************************************************/
/**
 *          AdpfConfig_t
 *
 * @brief   ADPF Module configuration structure
 *
 *****************************************************************************/
typedef struct AdpfConfig_s {
  float  fSensorGain;        /**< initial sensor gain */
  AdpfConfigType_t type;               /**< configuration type */
  AdpfMode_t mode;

  union AdpfConfigData_u {
    struct AdpfDefaultConfig_s {
      uint32_t                SigmaGreen;         /**< sigma value for green pixel */
      uint32_t                SigmaRedBlue;       /**< sigma value for red/blue pixel */
      float                   fGradient;          /**< gradient value for dynamic strength calculation */
      float                   fOffset;            /**< offset value for dynamic strength calculation */
      float                   fMin;               /**< upper bound for dynamic strength calculation */
      float                   fDiv;               /**< division factor for dynamic strength calculation */
      AdpfGains_t             NfGains;            /**< noise function gains */
    } def;

    struct AdpfDatabaseConfig_s {
      uint16_t                width;              /**< picture width */
      uint16_t                height;             /**< picture height */
      uint16_t                framerate;          /**< frame rate */
      CamCalibDbHandle_t      hCamCalibDb;        /**< calibration database handle */
    } db;
  } data;

  CamerIcDpfInvStrength_t dynInvStrength;

} AdpfConfig_t;


typedef struct Dsp3DnrResult_s{
	unsigned char Enable;
	
	uint16_t noise_coef_num;
	uint16_t noise_coef_den;
	
	unsigned char luma_sp_nr_en;            // control the strength of spatial luma denoise enable flag
    unsigned char luma_sp_nr_level;         // control the strength of spatial luma denoise
	unsigned char luma_te_nr_en;            // control the strength of spatial luma denoise enable flag
    unsigned char luma_te_nr_level;         // control the strength of temporal luma denoise
	unsigned char chrm_sp_nr_en;            // control the strength of spatial chroma denoise enable flag
	unsigned char chrm_sp_nr_level;         // control the strength of spatial chroma denoise level
	unsigned char chrm_te_nr_en;            // control the strength of spatial luma denoise enable flag
	unsigned char chrm_te_nr_level;         // control the strength of temporal chroma denoise level
	unsigned char shp_en;
	unsigned char shp_level;           // control sharpness strenth

    //5x5 luma spatial weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
	unsigned char luma_default;      // 1 use level define,0 use those parameters below
	unsigned char luma_sp_rad;      //spatial bilateral filter size
	unsigned char luma_te_max_bi_num;      //temporal max bilateral frame num
    uint32_t luma_w0;            //
    uint32_t luma_w1;
    uint32_t luma_w2;
    uint32_t luma_w3;
    uint32_t luma_w4;

    //5x5 chroma spatial weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are unsigned int type.
	unsigned char chrm_default;      // 1 use level define,0 use those parameters below
	unsigned char chrm_sp_rad;      //chroma spatial bilateral filter size
	unsigned char chrm_te_max_bi_num;      //temporal max bilateral frame num
    uint32_t chrm_w0;            //
    uint32_t chrm_w1;
    uint32_t chrm_w2;
    uint32_t chrm_w3;
    uint32_t chrm_w4;

	unsigned char shp_default;      // 1 use level define,0 use those parameters below
	//5x5 sharpness weight table,8bit for the center point,6bit for the other point,
	//low 30bit is useful in w0 w1 w3 w4,6 6 8 6 6 in w2,all these weight are int type.
    uint32_t src_shp_w0;            //
    uint32_t src_shp_w1;
    uint32_t src_shp_w2;
    uint32_t src_shp_w3;
    uint32_t src_shp_w4;
    //threshold from 0 to 31
    unsigned char src_shp_thr;
    //shift bit of  sum of weight.
    unsigned char src_shp_div;
    //luma sharpness enable flag
    unsigned char src_shp_l;
    //chroma sharpness enable flag
    unsigned char src_shp_c;
}Dsp3DnrResult_t;

typedef struct NewDsp3DnrResult_s{
	uint32_t enable_3dnr;
	uint32_t enable_dpc;        // Set to 1 by default, enable DSP dpc algorithm
	uint32_t reserved[4];

	uint32_t enable_ynr;        // Set to 1 by default
	uint32_t enable_tnr;        // Set to 1 by default, it will be disabled when enable_ynr=0
	uint32_t enable_iir;        // Set to 0 by default, it will be disabled when enable_ynr=0
	uint32_t ynr_time_weight;        // Denoise weight of time, valid range: 1 - 4, default 3
	uint32_t ynr_spat_weight;        // Denoise weight of spatial, valid range: 0 - 28, default 16
	uint32_t ynr_reserved[4];

	uint32_t enable_uvnr;       // Set to 1 by default
	uint32_t uvnr_weight;       // Denoise weight for uvnr, valid range: 4 - 16, default 12
	uint32_t uvnr_reserved[4];
	
	uint32_t enable_sharp;      // Set to 1 by default, enable DSP sharpness algorithm
	uint32_t sharp_weight;      // Sharpness weight, valid range: 0 - 4, defalut 2
	uint32_t sharp_reserved[4];
	
}NewDsp3DnrResult_t;

typedef struct RKDemosiacLpResult_s{
	uint8_t  lp_en;
	uint8_t  rb_filter_en;
	uint8_t  hp_filter_en;
	uint8_t  lu_divided[4];
	uint8_t thgrad_divided[5];
	uint8_t thdiff_divided[5];
	uint8_t thcsc_divided[5];
	uint16_t thvar_divided[5];
	uint8_t th_grad;
	uint8_t th_diff;
	uint8_t th_csc;
	uint16_t th_var;
	uint8_t th_var_en;
	uint8_t th_csc_en;
	uint8_t th_diff_en;
	uint8_t th_grad_en;
	uint8_t use_old_lp;
	uint8_t similarity_th;
	uint8_t flat_level_sel;
	uint8_t pattern_level_sel;
	uint8_t edge_level_sel;
	uint8_t thgrad_r_fct;
	uint8_t thdiff_r_fct;
	uint8_t thvar_r_fct;
	uint8_t thgrad_b_fct;
	uint8_t thdiff_b_fct;
	uint8_t thvar_b_fct;
	int light_mode; 
}RKDemosiacLpResult_t;

typedef struct RKIESharpResult_s{
	uint8_t iesharpen_en;	  // iesharpen_en 0 off, 1 on
	uint8_t coring_thr; 		// iesharpen coring_thr is default 0
	uint8_t full_range; 		// iesharpen full range(yuv data) 1:full_range(0-255),2:range(16-24?)
	uint8_t switch_avg; 	  //iesharpen whether is compare center pixel with edge pixel
	uint8_t yavg_thr[4];// Y channel is set five segments by the Matrix
	uint8_t delta1[5];
	uint8_t delta2[5];
	uint8_t maxnumber[5];
	uint8_t minnumber[5];
	uint8_t gauss_flat_coe[9];
	uint8_t gauss_noise_coe[9];
	uint8_t gauss_other_coe[9];
	uint8_t uv_gauss_flat_coe[15];
	uint8_t uv_gauss_noise_coe[15];
	uint8_t uv_gauss_other_coe[15];	
	uint16_t p_grad[4]; 
  	uint8_t sharp_factor[5];
  	uint8_t line1_filter_coe[6];
  	uint8_t line2_filter_coe[9];
  	uint8_t line3_filter_coe[6];
	uint8_t lap_mat_coe[9];
}RKIESharpResult_t;


typedef struct AdpfResult_s {
  CamerIcDpfInvStrength_t DynInvStrength;

  CamerIcGains_t NfGains;
  CamerIcDpfNoiseLevelLookUp_t Nll; /**< noise level lookup */
  CamerIcDpfConfig_t DpfMode;

  unsigned int actives;

  CamerIcIspFltDeNoiseLevel_t denoise_level;
  CamerIcIspFltSharpeningLevel_t sharp_level;
  uint8_t demosaic_th;
  bool_t FltEnable;

  Dsp3DnrResult_t Dsp3DnrResult;
  NewDsp3DnrResult_t NewDsp3DnrResult;
  
  //isp12 
  RKDemosiacLpResult_t RKDemosaicLpResult; 
  RKIESharpResult_t	RKIESharpResult;
} AdpfResult_t;

/*****************************************************************************/
/**
 * @brief   This function converts float based gains into CamerIC 4.8 fixpoint
 *          format.
 *
 * @param   pAdpfGains          gains in float based format
 * @param   pCamerIcGains       gains in fix point format
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         gains sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT AdpfGains2CamerIcGains
(
    AdpfGains_t*   pAdpfGains,
    CamerIcGains_t*  pCamerIcGains
);



/*****************************************************************************/
/**
 * @brief   This function converts CamerIC 4.8 fixpoint format into float
 *          based gains.
 *
 * @param   pCamerIcGains       gains in fix point format
 * @param   pAdpfGains          gains in float based format
 *
 * @return                      Returns the result of the function call.
 * @retval  RET_SUCCESS         gains sucessfully converted
 * @retval  RET_NULL_POINTER    null pointer parameter
 *
 *****************************************************************************/
RESULT CamerIcGains2AdpfGains
(
    CamerIcGains_t*  pCamerIcGains,
    AdpfGains_t*     pAdpfGains
);

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
RESULT AdpfInit
(
    AdpfHandle_t* handle,
    AdpfConfig_t* pConfig
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
RESULT AdpfRelease
(
    AdpfHandle_t handle
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
RESULT AdpfConfigure
(
    AdpfHandle_t handle,
    AdpfConfig_t* pConfig
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
RESULT AdpfRun
(
    AdpfHandle_t    handle,
    const float     gain,
    enum LIGHT_MODE LightMode
);

RESULT AdpfGetResult
(
    AdpfHandle_t    handle,
    AdpfResult_t*  result
);

#ifdef __cplusplus
}
#endif


/* @} ADPF */


#endif /* __ADPF_H__*/
