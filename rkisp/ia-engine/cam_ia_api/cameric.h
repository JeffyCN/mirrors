#ifndef __CAMERA_IC_TYPES_H__
#define __CAMERA_IC_TYPES_H__

#include <ebase/types.h>
#include <common/cam_types.h>

//lsc
/*****************************************************************************/
/**
 * @brief   This macro defines the size of lens shading data table in 16 Bit
 *                      words.
 *
 *****************************************************************************/
#define CAMERIC_DATA_TBL_SIZE      289



/*****************************************************************************/
/**
 * @brief   This macro defines the size of lens shading grad table in 16 Bit
 *                      words.
 *
 *****************************************************************************/
#define CAEMRIC_GRAD_TBL_SIZE        8




/*****************************************************************************/
/**
 * @brief   This macro defines the size of lens shading grad table in 16 Bit
 *                      words.
 *
 *****************************************************************************/
#define CAEMRIC_GRAD_TBL_SIZE        8



/*****************************************************************************/
/**
 * @brief   This macro defines the size of lens shading sectors in x or y
 *                      direction
 *
 *****************************************************************************/
#define CAMERIC_MAX_LSC_SECTORS     16


/*****************************************************************************/
/**
 * @brief   This macro defines the size of lens shading sectors in x or y
 *                      direction
 *
 *****************************************************************************/
#define CAMERIC_MAX_LSC_SECTORS     16


/*****************************************************************************/
/**
 * @brief   Lens shade correction configuration structure
 *
 *****************************************************************************/
typedef struct CamerIcIspLscConfig_s {
  uint16_t LscRDataTbl[CAMERIC_DATA_TBL_SIZE];    /**< correction values of R color part */
  uint16_t LscGRDataTbl[CAMERIC_DATA_TBL_SIZE];   /**< correction values of G (red lines) color part */
  uint16_t LscGBDataTbl[CAMERIC_DATA_TBL_SIZE];   /**< correction values of G (blue lines) color part  */
  uint16_t LscBDataTbl[CAMERIC_DATA_TBL_SIZE];    /**< correction values of B color part  */
  uint16_t LscXGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of x direction  */
  uint16_t LscYGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of y direction  */
  uint16_t LscXSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of x direction            */
  uint16_t LscYSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of y direction            */
} CamerIcIspLscConfig_t;



/*****************************************************************************/
/**
 * @brief   Lens shade sector configuration structure
 *
 *****************************************************************************/
typedef struct CamerIcIspLscSectorConfig_s {
  uint16_t LscXGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of x direction  */
  uint16_t LscYGradTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< multiplication factors of y direction  */
  uint16_t LscXSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of x direction            */
  uint16_t LscYSizeTbl[CAEMRIC_GRAD_TBL_SIZE];    /**< sector sizes of y direction            */
} CamerIcIspLscSectorConfig_t;


/******************************************************************************/
/**
 * @brief   A structure to represent a 3x3 matrix.
 *
 *          The nine values are laid out as follows (zero based index):
 *
 *               | 0 1 2 | \n
 *               | 3 4 5 | \n
 *               | 6 7 8 | \n
 *
 * @note    The values are represented as fix point numbers.
 *
 *****************************************************************************/
typedef struct CamerIc3x3Matrix_s {
  uint32_t    Coeff[9U];               /**< array of 3x3 float values */
} CamerIc3x3Matrix_t;


/******************************************************************************/
/**
 * @brief   A structure/tupple to represent offset values for three (R,G,B)
 *          channels.
 *
 * @note    The offset values are represented as 2's complement integer.
 *          Number ranging from -2048 (0x800) to 2047 (0x7FF).
 *          0 is represented as 0x000.
 *
 *****************************************************************************/
typedef struct CamerIcXTalkOffset_s {
  uint16_t Red;                       /**< offset value for the red channel */
  uint16_t Green;                     /**< offset value for the green channel */
  uint16_t Blue;                      /**< offset value for the blue channel */
} CamerIcXTalkOffset_t;



/******************************************************************************/
/**
 * @brief   A structure to represent a general purpose window. The window is
 *          spanned by a horizontal and vertical offset, counting from zero,
 *          the width and the height.
 *
 * @note    The windows points are represented by 16-bit unsigned integer
 *          numbers.
 *
 *****************************************************************************/
typedef struct CamerIcWindow_s {
  uint16_t    hOffset;
  uint16_t    vOffset;
  uint16_t    width;
  uint16_t    height;
} CamerIcWindow_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure CamerIC ISP measuring mode
 *
 *****************************************************************************/
typedef enum CamerIcIspAwbMeasuringMode_e {
  CAMERIC_ISP_AWB_MEASURING_MODE_INVALID    = 0,      /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_AWB_MEASURING_MODE_YCBCR      = 1,      /**< near white discrimination mode using YCbCr color space */
  CAMERIC_ISP_AWB_MEASURING_MODE_RGB        = 2,      /**< RGB based measurement mode */
  CAMERIC_ISP_AWB_MEASURING_MODE_MAX,                 /**< upper border (only for an internal evaluation) */
} CamerIcIspAwbMeasuringMode_t;



/******************************************************************************/
/**
 * @brief   CamerIC AWB measurement configuration
 *
 ******************************************************************************/
typedef struct CamerIcAwbMeasuringConfig_s {
  uint8_t MaxY;           /**< YCbCr Mode: only pixels values Y <= ucMaxY contribute to WB measurement (set to 0 to disable this feature) */
  /**< RGB Mode  : unused */
  uint8_t RefCr_MaxR;     /**< YCbCr Mode: Cr reference value */
  /**< RGB Mode  : only pixels values R < MaxR contribute to WB measurement */
  uint8_t MinY_MaxG;      /**< YCbCr Mode: only pixels values Y >= ucMinY contribute to WB measurement */
  /**< RGB Mode  : only pixels values G < MaxG contribute to WB measurement */
  uint8_t RefCb_MaxB;     /**< YCbCr Mode: Cb reference value */
  /**< RGB Mode  : only pixels values B < MaxB contribute to WB measurement */
  uint8_t MaxCSum;        /**< YCbCr Mode: chrominance sum maximum value, only consider pixels with Cb+Cr smaller than threshold for WB measurements */
  /**< RGB Mode  : unused */
  uint8_t MinC;           /**< YCbCr Mode: chrominance minimum value, only consider pixels with Cb/Cr each greater than threshold value for WB measurements */
  /**< RGB Mode  : unused */
} CamerIcAwbMeasuringConfig_t;



/******************************************************************************/
/**
 * @brief   CamerIc AWB Module measurement values
 *
 ******************************************************************************/
typedef struct CamerIcAwbMeasuringResult_s {
  uint32_t    NoWhitePixel;           /**< number of white pixel */
  uint8_t     MeanY__G;               /**< Y/G  value in YCbCr/RGB Mode */
  uint8_t     MeanCb__B;              /**< Cb/B value in YCbCr/RGB Mode */
  uint8_t     MeanCr__R;              /**< Cr/R value in YCbCr/RGB Mode */
} CamerIcAwbMeasuringResult_t;


typedef enum CamerIcColorComponent_e {
  CAM_COLOR_COMPONENT_RED     = 0,
  CAM_COLOR_COMPONENT_GREENR  = 1,
  CAM_COLOR_COMPONENT_GREENB  = 2,
  CAM_COLOR_COMPONENT_BLUE    = 3,
  CAM_COLOR_COMPONENT_MAX     = 4
} CamerIcColorComponent_t;

typedef struct CamerIcIspBlsConfig_s {
  bool_t                  enabled;            /**< module enabled */

  uint16_t                isp_bls_a_fixed;    /**< black-level-substraction value */
  uint16_t                isp_bls_b_fixed;    /**< black-level-substraction value */
  uint16_t                isp_bls_c_fixed;    /**< black-level-substraction value */
  uint16_t                isp_bls_d_fixed;    /**< black-level-substraction value */
  uint8_t     num_win;
  CamerIcWindow_t         Window1;            /**< measuring window 1 */
  CamerIcWindow_t         Window2;            /**< measuring window 2 */
} CamerIcIspBlsConfig_t;


/*****************************************************************************/
/**
 * @brief   This type defines the static configuration structure of the
 *          CamerIc DPCC module (register configuration).
 */
/*****************************************************************************/
typedef struct CamerIcDpccConfig_s {
  bool_t    enabled;            /**< module enabled */
  uint32_t isp_dpcc_mode;
  uint32_t isp_dpcc_output_mode;
  uint32_t isp_dpcc_set_use;

  uint32_t isp_dpcc_methods_set_1;
  uint32_t isp_dpcc_methods_set_2;
  uint32_t isp_dpcc_methods_set_3;

  uint32_t isp_dpcc_line_thresh_1;
  uint32_t isp_dpcc_line_mad_fac_1;
  uint32_t isp_dpcc_pg_fac_1;
  uint32_t isp_dpcc_rnd_thresh_1;
  uint32_t isp_dpcc_rg_fac_1;

  uint32_t isp_dpcc_line_thresh_2;
  uint32_t isp_dpcc_line_mad_fac_2;
  uint32_t isp_dpcc_pg_fac_2;
  uint32_t isp_dpcc_rnd_thresh_2;
  uint32_t isp_dpcc_rg_fac_2;

  uint32_t isp_dpcc_line_thresh_3;
  uint32_t isp_dpcc_line_mad_fac_3;
  uint32_t isp_dpcc_pg_fac_3;
  uint32_t isp_dpcc_rnd_thresh_3;
  uint32_t isp_dpcc_rg_fac_3;

  uint32_t isp_dpcc_ro_limits;
  uint32_t isp_dpcc_rnd_offs;
} CamerIcDpccConfig_t;



#define CAMERIC_DEGAMMA_CURVE_SIZE      17U


/*******************************************************************************
 *
 * @brief
 *
 */
typedef struct CamerIcIspDegammaCurve_s {
  bool_t    enabled;            /**< module enabled */
  uint8_t     segment[CAMERIC_DEGAMMA_CURVE_SIZE - 1];    /**< x_i segment size */
  uint16_t    red[CAMERIC_DEGAMMA_CURVE_SIZE];            /**< red point */
  uint16_t    green[CAMERIC_DEGAMMA_CURVE_SIZE];          /**< green point */
  uint16_t    blue[CAMERIC_DEGAMMA_CURVE_SIZE];           /**< blue point */
} CamerIcIspDegammaCurve_t;


typedef struct CamerIcRKDemosaicLP_s {
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
}CamerIcRKDemosaicLP_t;

/* Configuration used by ISP filtering */
typedef struct CamerIcIspFltConfig_s {
  bool_t    enabled;            /**< module enabled */
  uint8_t mode;    /* ISP_FILT_MODE register fields*/
  uint8_t grn_stage1;    /* ISP_FILT_MODE register fields*/
  uint8_t chr_h_mode;    /* ISP_FILT_MODE register fields*/
  uint8_t chr_v_mode;    /* ISP_FILT_MODE register fields*/
  uint32_t  thresh_bl0;
  uint32_t  thresh_bl1;
  uint32_t  thresh_sh0;
  uint32_t  thresh_sh1;
  uint32_t  lum_weight;
  uint32_t fac_sh1;
  uint32_t  fac_sh0;
  uint32_t  fac_mid;
  uint32_t  fac_bl0;
  uint32_t  fac_bl1;
  int denoise_level;
  int sharp_level;
  
} CamerIcIspFltConfig_t;


#define CAMERIC_ISP_GAMMA_CURVE_SIZE        34

/******************************************************************************/
/**
 * @cond    CAMERIC_FULL
 *
 * @brief   A structure to represent a gamma-curve.
 *
 *****************************************************************************/
typedef struct CamerIcGammaCurve_s {
  uint16_t GammaY[CAMERIC_ISP_GAMMA_CURVE_SIZE];      /**< array of y coordinates */
} CamerIcIspGammaCurve_t;
/* @endcond */



/******************************************************************************/
/**
 * @cond    CAMERIC_FULL
 *
 * @brief   Enumeration type to configure the gamma curve segementation.
 *
 *****************************************************************************/
typedef enum CamerIcIspGammaSegmentationMode_e {
  CAMERIC_ISP_SEGMENTATION_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_SEGMENTATION_MODE_LOGARITHMIC   = 1,    /**< logarithmic segmentation from 0 to 4095
                                                             (64,64,64,64,128,128,128,128,256,256,256,512,512,512,512,512) */
  CAMERIC_ISP_SEGMENTATION_MODE_EQUIDISTANT   = 2,    /**< equidistant segmentation from 0 to 4095
                                                             (256, 256, ... ) */
  CAMERIC_ISP_SEGMENTATION_MODE_MAX                   /**< upper border (only for an internal evaluation) */
} CamerIcIspGammaSegmentationMode_t;


/* Configuration used by Gamma Out correction */
typedef struct CamerIcIspGocConfig_s {
  bool_t    enabled;            /**< module enabled */
  CamerIcIspGammaSegmentationMode_t mode;
  CamerIcIspGammaCurve_t gamma_y;
} CamerIcIspGocConfig_t;

/******************************************************************************/
/**
 * @brief   Enumeration type to configure the chrominace output range in the
 *          CamerIC color processing unit.
 *
 *****************************************************************************/
typedef enum CamerIcCprocChrominanceRangeOut_e {
  CAMERIC_CPROC_CHROM_RANGE_OUT_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_CPROC_CHROM_RANGE_OUT_BT601         = 1,    /**< CbCr_out clipping range 16..240 according to ITU-R BT.601 standard */
  CAMERIC_CPROC_CHROM_RANGE_OUT_FULL_RANGE    = 2,    /**< full UV_out clipping range 0..255 */
  CAMERIC_CPROC_CHROM_RANGE_OUT_MAX                   /**< upper border (only for an internal evaluation) */
} CamerIcCprocChrominaceRangeOut_t;


/******************************************************************************/
/**
 * @brief   Enumeration type to configure the luminance output range in the
 *          CamerIC color processing unit.
 *
 *****************************************************************************/
typedef enum CamerIcCprocLuminanceRangeOut_e {
  CAMERIC_CPROC_LUM_RANGE_OUT_INVALID         = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_CPROC_LUM_RANGE_OUT_BT601           = 1,    /**< Y_out clipping range 16..235; offset of 16 is added to Y_out according to ITU-R BT.601 standard */
  CAMERIC_CPROC_LUM_RANGE_OUT_FULL_RANGE      = 2,    /**< Y_out clipping range 0..255; no offset is added to Y_out */
  CAMERIC_CPROC_LUM_RANGE_OUT_MAX                     /**< upper border (only for an internal evaluation) */
} CamerIcCprocLuminanceRangeOut_t;


/******************************************************************************/
/**
 * @brief   Enumeration type to configure the luminance input range in the
 *          CamerIC color processing unit.
 *
 *****************************************************************************/
typedef enum CamerIcCprocLuminanceRangeIn_e {
  CAMERIC_CPROC_LUM_RANGE_IN_INVALID          = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_CPROC_LUM_RANGE_IN_BT601            = 1,    /**< Y_in range 64..940 according to ITU-R BT.601 standard; offset of 64 will be subtracted from Y_in */
  CAMERIC_CPROC_LUM_RANGE_IN_FULL_RANGE       = 2,    /**< Y_in full range 0..1023; no offset will be subtracted from Y_in */
  CAMERIC_CPROC_LUM_RANGE_IN_MAX                      /**< upper border (only for an internal evaluation) */
} CamerIcCprocLuminanceRangeIn_t;


/******************************************************************************/
/**
 * @brief   Structure to configure the color processing module
 *
 *****************************************************************************/
typedef struct CamerIcCprocConfig_s {
  bool_t    enabled;            /**< module enabled */
  CamerIcCprocChrominaceRangeOut_t    ChromaOut;      /**< configuration of color processing chrominance pixel clipping range at output */
  CamerIcCprocLuminanceRangeOut_t     LumaOut;        /**< configuration of color processing luminance input range (offset processing) */
  CamerIcCprocLuminanceRangeIn_t      LumaIn;         /**< configuration of color processing luminance output clipping range */

  uint8_t                             contrast;       /**< contrast value to initially set */
  uint8_t                             brightness;     /**< brightness value to initially set */
  uint8_t                             saturation;     /**< saturation value to initially set */
  uint8_t                             hue;            /**< hue value to initially set */
} CamerIcCprocConfig_t;

/*****************************************************************************/
typedef enum CamerIcIspECMMode_e {
  CAMERIC_ISP_ECM_MODE_INVALID     = 0,    /**< invalid exposure conversion module */
  CAMERIC_ISP_ECM_MODE_1   = 1,  /* the original exposure conversion module ,   minimize the value of gain   */
  CAMERIC_ISP_ECM_MODE_2    = 2,    /* the newl exposure conversion module ,imcrease the frame rate */
  CAMERIC_ISP_ISP_ECM_MODE_MAX,
} CamerIcIspECMMode_t;

/*****************************************************************************/
/**
 * @brief   Enumeration type to configure CamerIC ISP exposure measuring mode.
 *
 *****************************************************************************/
typedef enum CamerIcIspExpMeasuringMode_e {
  CAMERIC_ISP_EXP_MEASURING_MODE_INVALID    = 0,    /**< invalid histogram measuring mode   */
  CAMERIC_ISP_EXP_MEASURING_MODE_1          = 1,    /**< Y = (R + G + B) x (85/256)         */
  CAMERIC_ISP_EXP_MEASURING_MODE_2          = 2,    /**< Y = 16 + 0.25R + 0.5G + 0.1094B    */
  CAMERIC_ISP_EXP_MEASURING_MODE_MAX,
} CamerIcIspExpMeasuringMode_t;

/* set to the maximum usecase for rk3326 */
#define CAMERIC_ISP_HIST_GRID_ITEMS         81/**< number of grid sub windows */
#define CAMERIC_ISP_HIST_GRID_W             9

typedef uint8_t CamerIcHistWeights_t[CAMERIC_ISP_HIST_GRID_ITEMS];

typedef enum CamerIcIspHistMode_e {
  CAMERIC_ISP_HIST_MODE_INVALID       = 0,    /**< lower border (only for an internal evaluation) */
  CAMERIC_ISP_HIST_MODE_RGB_COMBINED  = 1,    /**< RGB combined histogram */
  CAMERIC_ISP_HIST_MODE_R             = 2,    /**< R histogram */
  CAMERIC_ISP_HIST_MODE_G             = 3,    /**< G histogram */
  CAMERIC_ISP_HIST_MODE_B             = 4,    /**< B histogram */
  CAMERIC_ISP_HIST_MODE_Y             = 5,    /**< luminance histogram */
  CAMERIC_ISP_HIST_MODE_MAX,                                  /**< upper border (only for an internal evaluation) */
} CamerIcIspHistMode_t;

typedef struct CamerIcIspHistConfig_s {
  bool_t                  enabled;                            /**< measuring enabled */
  CamerIcIspHistMode_t    mode;                               /**< histogram mode */
  uint16_t                StepSize;                           /**< stepsize calculated from measuirng window */
  CamerIcWindow_t         Window;                             /**< measuring window */
  CamerIcHistWeights_t    Weights;
} CamerIcIspHistConfig_t;


/******************************************************************************/
/**
 * @brief   Enumeration type to configure the IE working mode.
 *
 *****************************************************************************/
typedef enum CamerIcIeMode_e {
  CAMERIC_IE_MODE_INVALID                 = 0,        /**< lower border (only for an internal evaluation) */
  CAMERIC_IE_MODE_GRAYSCALE               = 1,        /**< Set a fixed chrominance of 128 (neutral grey) */
  CAMERIC_IE_MODE_NEGATIVE                = 2,        /**< Luminance and chrominance data is being inverted */
  CAMERIC_IE_MODE_SEPIA                   = 3,        /**< Chrominance is changed to produce a historical like brownish image color */
  CAMERIC_IE_MODE_COLOR                   = 4,        /**< Converting picture to grayscale while maintaining one color component. */
  CAMERIC_IE_MODE_EMBOSS                  = 5,        /**< Edge detection, will look like an relief made of metal */
  CAMERIC_IE_MODE_SKETCH                  = 6,        /**< Edge detection, will look like a pencil drawing */
  CAMERIC_IE_MODE_SHARPEN                 = 7,        /**< Edge detection, will look like a sharper drawing */
  CAMERIC_IE_MODE_RK_SHARPEN			  = 8,
  CAMERIC_IE_MODE_MAX                                 /**< upper border (only for an internal evaluation) */
} CamerIcIeMode_t;



/******************************************************************************/
/**
 * @brief   Enumeration type to configure the IE working range.
 *
 *****************************************************************************/
typedef enum CamerIcIeRange_e {
  CAMERIC_IE_RANGE_INVALID                = 0,        /**< lower border (only for an internal evaluation) */
  CAMERIC_IE_RANGE_BT601                  = 1,        /**< pixel value range accoring to BT.601 */
  CAMERIC_IE_RANGE_FULL_RANGE             = 2,        /**< YCbCr full range 0..255 */
  CAMERIC_IE_RANG_MAX                                 /**< upper border (only for an internal evaluation) */
} CamerIcIeRange_t;

/******************************************************************************/
/**
 * @brief   Enumeration type to configure the color selection effect
 *
 *****************************************************************************/
typedef enum CamerIcIeColorSelection_e {
  CAMERIC_IE_COLOR_SELECTION_INVALID      = 0,        /**< lower border (only for an internal evaluation) */
  CAMERIC_IE_COLOR_SELECTION_RGB          = 1,        /**< red, green and blue */
  CAMERIC_IE_COLOR_SELECTION_B            = 2,        /**< blue */
  CAMERIC_IE_COLOR_SELECTION_G            = 3,        /**< green */
  CAMERIC_IE_COLOR_SELECTION_GB           = 4,        /**< green and blue */
  CAMERIC_IE_COLOR_SELECTION_R            = 5,        /**< red */
  CAMERIC_IE_COLOR_SELECTION_RB           = 6,        /**< red and blue */
  CAMERIC_IE_COLOR_SELECTION_RG           = 7,        /**< red and green */
  CAMERIC_IE_COLOR_SELECTION_MAX                      /**< upper border (only for an internal evaluation) */
} CamerIcIeColorSelection_t;
	
typedef struct CamerIcRKIeSharpConfig_s {
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
}CamerIcRKIeSharpConfig_t;


typedef struct CamerIcIeConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CamerIcIeMode_t                     mode;           /**< working mode (see @ref CamerIcIeMode_e) */
  CamerIcIeRange_t                    range;          /**< working range (see @ref CamerIcIeRange_e) */

  union ModeConfig_u {
    struct Sepia_s {                                /**< active when sepia effect */
      uint8_t                     TintCb;
      uint8_t                     TintCr;
    } Sepia;

    struct ColorSelection_s {                       /**< active when color selection effect */
      CamerIcIeColorSelection_t   col_selection;
      uint8_t                     col_threshold;
    } ColorSelection;

    struct Emboss_s {                               /**< active when emboss effect */
      int8_t                      coeff[9];
    } Emboss;

    struct Sketch_s {                               /**< active when sketch effect */
      int8_t                      coeff[9];
    } Sketch;

    struct Sharpen_s {                              /**< active when sharpen */
      uint8_t                     factor;         /**< sharpen factor */
      uint8_t                     threshold;      /**< corring threshold */
      int8_t                      coeff[9];       /**< convolution coefficients */
    } Sharpen;

  } ModeConfig;

} CamerIcIeConfig_t;


typedef struct CamerIcLscConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CamLscMatrix_t* lsc_result;
  CamerIcIspLscSectorConfig_t* lsc_seg_result;
} CamerIcLscConfig_t;

typedef struct CamerIcAwbMeasConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CamerIcIspAwbMeasuringMode_t* awb_meas_mode_result;
  CamerIcAwbMeasuringConfig_t*  awb_meas_result;
  Cam_Win_t* awb_win;
} CamerIcAwbMeasConfig_t;

typedef struct CameraIcAwbGainConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CamerIcGains_t* awb_gain_result;
} CameraIcAwbGainConfig_t;

typedef struct CameraIcCtkConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CamerIc3x3Matrix_t* ctk_matrix_result;
  CamerIcXTalkOffset_t* ctk_offset_result;
} CameraIcCtkConfig_t;

typedef struct CameraIcAecConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  Cam_Win_t* meas_win;
  int* aec_meas_mode;
} CameraIcAecConfig_t;

typedef struct CameraIcBdmConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  unsigned char demosaic_th;
} CameraIcBdmConfig_t;


#define CAMERIC_WDR_CURVE_SIZE      33U

typedef enum CameraIcWdrMode_e {
  CAMERIC_WDR_MODE_GLOBAL,
  CAMERIC_WDR_MODE_BLOCK
} CameraIcWdrMode_t;

typedef struct CameraIcWdrConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  CameraIcWdrMode_t mode;
  uint8_t     segment[CAMERIC_WDR_CURVE_SIZE - 1];    /**< x_i segment size */
  uint16_t    wdr_global_y[CAMERIC_WDR_CURVE_SIZE];
  uint16_t    wdr_block_y[CAMERIC_WDR_CURVE_SIZE];
  uint16_t wdr_noiseratio;
  uint16_t wdr_bestlight;
  uint32_t wdr_gain_off1;
  uint16_t wdr_pym_cc;
  uint8_t wdr_epsilon;
  uint8_t wdr_lvl_en;
  uint8_t wdr_flt_sel;
  uint8_t wdr_gain_max_clip_enable;
  uint8_t wdr_gain_max_value;
  uint8_t wdr_bavg_clip;
  uint8_t wdr_nonl_segm;
  uint8_t wdr_nonl_open;
  uint8_t wdr_nonl_mode1;
  uint32_t wdr_coe0;
  uint32_t wdr_coe1;
  uint32_t wdr_coe2;
  uint32_t wdr_coe_off;
} CameraIcWdrConfig_t;

typedef struct CameraIcDpfConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
} CameraIcDpfConfig_t;

typedef struct CameraIcDpfStrengthConfig_s {
  bool_t          enabled;                            /**< measuring enabled */
  uint8_t r;
  uint8_t g;
  uint8_t b;
} CameraIcDpfStrengthConfig_t;


#endif

