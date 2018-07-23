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
 * @file    calibtags.h
 *
 *
 *****************************************************************************/
#ifndef __CALIB_TAGS_H__
#define __CALIB_TAGS_H__


/******************************************************************************
 * XML main section
 *****************************************************************************/
#define CALIB_FILESTART_TAG                 "matfile"

/******************************************************************************
 * XML cell section
 *****************************************************************************/
#define CALIB_CELL_TAG                      "cell"

/******************************************************************************
 * XML header section
 *****************************************************************************/
#define CALIB_HEADER_TAG                    "header"

#define CALIB_HEADER_CREATION_DATE_TAG      "creation_date"
#define CALIB_HEADER_CREATOR_TAG            "creator"
#define CALIB_HEADER_SENSOR_NAME_TAG        "sensor_name"
#define CALIB_HEADER_SAMPLE_NAME_TAG        "sample_name"
#define CALIB_HEADER_GENERATOR_VERSION_TAG  "generator_version"

/******************************************************************************
 * XML resolution tag
 *****************************************************************************/
#define CALIB_HEADER_RESOLUTION_TAG                                 "resolution"
#define CALIB_HEADER_RESOLUTION_FORMAT                              "%dx%d"
#define CALIB_HEADER_RESOLUTION_NO_ELEMENTS                         2

#define CALIB_HEADER_RESOLUTION_NAME_TAG                            "name"
#define CALIB_HEADER_RESOLUTION_ID_TAG                              "id"
#define CALIB_HEADER_RESOLUTION_WIDTH_TAG                           "width"
#define CALIB_HEADER_RESOLUTION_HEIGHT_TAG                          "height"
#define CALIB_HEADER_RESOLUTION_FRATE_TAG                           "framerate"

#define CALIB_HEADER_RESOLUTION_FRATE_NAME_TAG                      "name"
#define CALIB_HEADER_RESOLUTION_FRATE_FPS_TAG                       "fps"

#define CALIB_HEADER_CODE_XML_PARSE_VERSION_TAG                     "code_xml_parse_version"


/******************************************************************************
 * XML sensor section
 *****************************************************************************/
#define CALIB_SENSOR_TAG                                            "sensor"


/******************************************************************************
 * XML AWB section
 *****************************************************************************/
#define CALIB_SENSOR_AWB_TAG                                        "AWB"

#define CALIB_SENSOR_AWB_GLOBALS_TAG                                "globals"
#define CALIB_SENSOR_AWB_GLOBALS_NAME_TAG                           "name"
#define CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG                     "resolution"
#define CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG                "sensorFilename"
#define CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG                   "SVDMeanValue"
#define CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG                      "PCAMatrix"
#define CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG                     "CenterLine"
#define CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG                        "K_Factor"
#define CALIB_SENSOR_AWB_GLOBALS_RG1_TAG                            "afRg1"
#define CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG                       "afMaxDist1"
#define CALIB_SENSOR_AWB_GLOBALS_RG2_TAG                            "afRg2"
#define CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG                       "afMaxDist2"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG                    "afGlobalFade1"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG                "afGlobalGainDistance1"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG                    "afGlobalFade2"
#define CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG                "afGlobalGainDistance2"
#define CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG                          "afFade2"
#define CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG               "afCbMinRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG               "afCrMinRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG             "afMaxCSumRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG               "afCbMinRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG               "afCrMinRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG             "afMaxCSumRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG              "fRgProjIndoorMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG             "fRgProjOutdoorMin"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG                     "fRgProjMax"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG                 "fRgProjMaxSky"
#define CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR                       "awb_clip_outdoor"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE                        "RegionSize"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC                    "RegionSizeInc"
#define CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC                    "RegionSizeDec"
#define CALIB_SENSOR_AWB_GLOBALS_IIR                                "IIR"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD                  "DampCoefAdd"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB                  "DampCoefSub"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD          "DampFilterThreshold"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN               "DampingCoefMin"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX               "DampingCoefMax"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT              "DampingCoefInit"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX      "ExpPriorFilterSizeMax"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN      "ExpPriorFilterSizeMin"
#define CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE               "ExpPriorMiddle"
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_A_LIMIT           "fRgProjALimit"   //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_A_WEIGHT          "fRgProjAWeight"  //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_YELLOW_LIMIT        "fRgProjYellowLimit"  //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF          "fRgProjIllToCwf"   //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF_WEIGHT     "fRgProjIllToCwfWeight" //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_YELLOW_LIMIT_ENABLE     "fRgProjYellowLimitEnable"  //oyyf
#define CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF_ENABLE     "fRgProjIllToCwfEnable"
#define CALIB_SENSOR_AWB_GLOBALS_MINC_REGIONMAX_TAG         "afMinCRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_MINC_REGIONMIN_TAG         "afMinCRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_MAXY_REGIONMAX_TAG         "afMaxYRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_MAXY_REGIONMIN_TAG         "afMaxYRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_MINY_MAXG_REGIONMAX_TAG      "afMinYMaxGRegionMax"
#define CALIB_SENSOR_AWB_GLOBALS_MINY_MAXG_REGIONMIN_TAG      "afMinYMaxGRegionMin"
#define CALIB_SENSOR_AWB_GLOBALS_REFCB_TAG      "afRefCb"
#define CALIB_SENSOR_AWB_GLOBALS_REFCR_TAG      "afRefCr"



#define CALIB_SENSOR_AWB_ILLUMINATION_TAG                           "illumination"
#define CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG                      "name"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG                 "doorType"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR              "INDOOR"
#define CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR             "OUTDOOR"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG                  "awbType"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL               "MANUAL"
#define CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO                 "AUTO"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG                 "manualWB"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG                 "manualccMatrix"
#define CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG                "manualccOffsets"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG                       "GMM"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG       "GaussianMeanValue"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG        "invCovMatrix"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG      "GaussianScalingFactor"
#define CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG                   "tau"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG                    "sat_CT"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG               "gains"
#define CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG                "sat"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG                    "vig_CT"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG               "gains"
#define CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG                "vig"

#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG                      "aLSC"
#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG                  "resolution"
#define CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG "LSC_PROFILE_LIST"

#define CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG                       "aCC"
#define CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG       "CC_PROFILE_LIST"



/******************************************************************************
 * XML LSC section
 *****************************************************************************/
#define CALIB_SENSOR_LSC_TAG                                "LSC"
#define CALIB_SENSOR_LSC_PROFILE_NAME_TAG                   "name"
#define CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG             "resolution"
#define CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG           "illumination"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG            "LSC_sectors"
#define CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG                 "LSC_No"
#define CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG                 "LSC_Xo"
#define CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG                 "LSC_Yo"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG      "LSC_SECT_SIZE_X"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG      "LSC_SECT_SIZE_Y"
#define CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG         "vignetting"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG        "LSC_SAMPLES_red"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG     "LSC_SAMPLES_greenR"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG     "LSC_SAMPLES_greenB"
#define CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG       "LSC_SAMPLES_blue"



/******************************************************************************
 * XML CC section
 *****************************************************************************/
#define CALIB_SENSOR_CC_TAG                                 "CC"
#define CALIB_SENSOR_CC_PROFILE_NAME_TAG                    "name"
#define CALIB_SENSOR_CC_PROFILE_SATURATION_TAG              "saturation"
#define CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG               "ccMatrix"
#define CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG              "ccOffsets"
#define CALIB_SENSOR_CC_PROFILE_WB_TAG                      "wb"


/******************************************************************************
 * XML AF section
 *****************************************************************************/
#define CALIB_SENSOR_AF_TAG                                 "AF"



/******************************************************************************
 * XML AEC section
 *****************************************************************************/
#define CALIB_SENSOR_AEC_TAG                                "AEC"
#define CALIB_SENSOR_AEC_SETPOINT_TAG                       "SetPoint"
#define CALIB_SENSOR_AEC_NIGHT_SETPOINT_TAG					"NightSetpoint"
#define CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG                  "ClmTolerance"
#define CALIB_SENSOR_AEC_DAMP_OVER_TAG                      "DampOver"
#define CALIB_SENSOR_AEC_DAMP_UNDER_TAG                     "DampUnder"
#define CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG                "DampOverVideo"
#define CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG               "DampUnderVideo"
#define CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG                  "aFpsMaxGain"

#define CALIB_SENSOR_AEC_ECM_TAG                            "ECM"
#define CALIB_SENSOR_AEC_ECM_NAME_TAG                       "name"
#define CALIB_SENSOR_AEC_ECM_SCHEMES_TAG                    "PrioritySchemes"
#define CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG                "name"
#define CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG         "OffsetT0Fac"
#define CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG             "SlopeA0"

#define CALIB_SENSOR_AEC_AFPS_GAIN_FACTOR_TAG               "aFpsGainFactor"
#define CALIB_SENSOR_AEC_AFPS_GAIN_BIAS_TAG                 "aFpsGainBias"
#define CALIB_SENSOR_AEC_AFPS_MAX_INTTIME_TAG               "aFpsMaxIntTime"//cxf
#define CALIB_SENSOR_AEC_SEMMODE_TAG                        "SemMode"   //cxf
#define CALIB_SENSOR_AEC_SEMMODE_DISABLED                   "DISABLED"
#define CALIB_SENSOR_AEC_SEMMODE_FIX                        "FIX"
#define CALIB_SENSOR_AEC_SEMMODE_ADAPTIVE                   "ADAPTIVE"
#define CALIB_SENSOR_AEC_GAINRANGE_TAG                      "GainRange"
#define CALIB_SENSOR_AEC_GRIDWEIGHTS_TAG                    "GridWeights"   //cxf
#define CALIB_SENSOR_AEC_TIMEFACTOR_TAG                     "TimeFactor"   //cxf
#define CALIB_SENSOR_AEC_MEASURINGWINWIDTHSCALE_TAG         "MeasuringWindowWidthScale"   //cxf
#define CALIB_SENSOR_AEC_MEASURINGWINHEIGHTSCALE_TAG        "MeasuringWindowHeightScale"   //cxf

#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_TAG             "CamerIcIspHistMode"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_R               "R"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_G               "G"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_B               "B"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_RGB             "RGB"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPHISTMODE_Y               "Y"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_TAG     "CamerIcIspExpMeasuringMode"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_1       "Y"//cxf
#define CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_2       "RGB"//cxf

#define CALIB_SENSOR_AEC_ECMTIMEDOT_TAG                     "EcmTimeDot"
#define CALIB_SENSOR_AEC_ECMGAINDOT_TAG                     "EcmGainDot"
#define CALIB_SENSOR_AEC_ECMMODE_TAG                        "EcmMode"
#define CALIB_SENSOR_AEC_ECMMODE_1                          "MODE_1"
#define CALIB_SENSOR_AEC_ECMMODE_2                          "MODE_2"

#define CALIB_SENSOR_AEC_INTERVAL_ADJUST_STRATEGY		"AEC_Interval_Adjust_Strategy"
#define CALIB_SENSOR_AEC_INTERVAL_ADJUST_ENABLE		"enable"
#define CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_HIGH_TH	"dLuma_high_th"
#define CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_LOW_TH	"dLuma_low_th"
#define CALIB_SENSOR_AEC_INTERVAL_ADJUST_TRIGGER_FRAME	"adjust_trigger_frame"

#define CALIB_SENSOR_AEC_AOE_ENABLE             "AOE_Enable"
#define CALIB_SENSOR_AEC_AOE_MAX_POINT            "AOE_Max_point"
#define CALIB_SENSOR_AEC_AOE_MIN_POINT            "AOE_Min_point"
#define CALIB_SENSOR_AEC_AOE_Y_MAX_TH           "AOE_Y_Max_th"
#define CALIB_SENSOR_AEC_AOE_Y_MIN_TH           "AOE_Y_Min_th"
#define CALIB_SENSOR_AEC_AOE_STEP_INC           "AOE_Step_Inc"
#define CALIB_SENSOR_AEC_AOE_STEP_DEC           "AOE_Step_Dec"

/* Day or Night */
#define CALIB_SENSOR_AEC_DON                  "DON"
#define CALIB_SENSOR_AEC_DON_ENABLE               "DON_Enable"
#define CALIB_SENSOR_AEC_DON_DAY2NIGHT_GAIN_TH        "DON_Day2Night_Gain_th"
#define CALIB_SENSOR_AEC_DON_DAY2NIGHT_INTTIME_TH     "DON_Day2Night_Inttime_th"
#define CALIB_SENSOR_AEC_DON_DAY2NIGHT_LUMA_TH        "DON_Day2Night_Luma_th"
#define CALIB_SENSOR_AEC_DON_NIGHT2DAY_GAIN_TH        "DON_Night2Day_Gain_th"
#define CALIB_SENSOR_AEC_DON_NIGHT2DAY_INTTIME_TH     "DON_Night2Day_Inttime_th"
#define CALIB_SENSOR_AEC_DON_NIGHT2DAY_LUMA_TH        "DON_Night2Day_Luma_th"
#define CALIB_SENSOR_AEC_DON_BOUNCING_TH            "DON_Bouncing_th"

#define CALIB_SENSOR_AEC_FPS_FPS_SET_CONFIG			"FpsSetConfig"
#define CALIB_SENSOR_AEC_FPS_FPS_SET_ENABLE			"FpsSet_enable"
#define CALIB_SENSOR_AEC_FPS_IS_FPS_FIX				"isFpsFix"
#define CALIB_SENSOR_AEC_FPS_FPS_FIX_TIMEDOT		"FpsFix_TimeDot"


/******************************************************************************
 * XML BLS section
 *****************************************************************************/
#define CALIB_SENSOR_BLS_TAG                                "BLS"
#define CALIB_SENSOR_BLS_NAME_TAG                           "name"
#define CALIB_SENSOR_BLS_RESOLUTION_TAG                     "resolution"
#define CALIB_SENSOR_BLS_DATA_TAG                           "blsData"



/******************************************************************************
 * XML DEGAMMA section
 *****************************************************************************/
#define CALIB_SENSOR_DEGAMMA_TAG                            "DEGAMMA"

/******************************************************************************
 * XML GOC section
 *****************************************************************************/
#define CALIB_SENSOR_GOC_TAG                            "GOC"
#define CALIB_SENSOR_GOC_NAME_TAG                            "name"
#define CALIB_SENSOR_GOC_ENABLE_MODE_TAG                     "enable_mode"
#define CALIB_SENSOR_GOC_DEF_CFG_MODE_TAG                     "def_cfg_mode"
#define CALIB_SENSOR_GOC_GAMMAY_TAG                     "gamma_y"
#define CALIB_SENSOR_GOC_WDRON_GAMMAY_TAG                     "wdron_gamma_y"



/******************************************************************************
 * XML WDR section
 *****************************************************************************/
#define CALIB_SENSOR_WDR_TAG                                "WDR"
#define CALIB_SENSOR_WDR_ENABLE_TAG                         "enabled"
#define CALIB_SENSOR_WDR_MODE_TAG                           "mode"
#define CALIB_SENSOR_WDR_LOCAL_CURVE_TAG                    "local_curve"
#define CALIB_SENSOR_WDR_GLOBAL_CURVE_TAG                   "global_curve"

#define CALIB_SENSOR_WDR_NOISE_RATIO_TAG                    "wdr_noiseratio"
#define CALIB_SENSOR_WDR_BEST_LIGHT_TAG                     "wdr_bestlight"
#define CALIB_SENSOR_WDR_GAIN_OFF1_TAG                      "wdr_gain_off1"
#define CALIB_SENSOR_WDR_PYM_TAG                            "wdr_pym_cc"
#define CALIB_SENSOR_WDR_EPSILON_TAG                        "wdr_epsilon"
#define CALIB_SENSOR_WDR_LVL_TAG                            "wdr_lvl_en"
#define CALIB_SENSOR_WDR_FLT_TAG                            "wdr_flt_sel"
#define CALIB_SENSOR_WDR_GAIN_MAX_CLIP_ENABLE_TAG           "wdr_gain_max_clip_enable"
#define CALIB_SENSOR_WDR_GAIN_MAX_VALUE_TAG                 "wdr_gain_max_value"
#define CALIB_SENSOR_WDR_BAVG_TAG                           "wdr_bavg_clip"
#define CALIB_SENSOR_WDR_NONL_SEGM_TAG                      "wdr_nonl_segm"
#define CALIB_SENSOR_WDR_NONL_OPEN_TAG                      "wdr_nonl_open"
#define CALIB_SENSOR_WDR_NONL_MODE1_TAG                     "wdr_nonl_mode1"
#define CALIB_SENSOR_WDR_COE0_TAG                           "wdr_coe0"
#define CALIB_SENSOR_WDR_COE1_TAG                           "wdr_coe1"
#define CALIB_SENSOR_WDR_COE2_TAG                           "wdr_coe2"
#define CALIB_SENSOR_WDR_COE_OFF_TAG                        "wdr_coe_off"

#define CALIB_SENSOR_WDR_MAXGAIN_FILTER_TAG                 "wdr_maxgain_filter"
#define CALIB_SENSOR_WDR_MAXGAIN_FILTER_ENABLE              "wdr_maxgain_filter_enable"
#define CALIB_SENSOR_WDR_MAXGAIN_SENSOR_GAIN_LEVEL_TAG      "wdr_sensor_gain_level"
#define CALIB_SENSOR_WDR_MAXGAIN_MAXGAIN_LEVEL_TAG          "wdr_maxgain_level"



/******************************************************************************
 * XML CAC section
 *****************************************************************************/
#define CALIB_SENSOR_CAC_TAG                                "CAC"
#define CALIB_SENSOR_CAC_NAME_TAG                           "name"
#define CALIB_SENSOR_CAC_RESOLUTION_TAG                     "resolution"
#define CALIB_SESNOR_CAC_X_NORMSHIFT_TAG                    "x_normshift"
#define CALIB_SESNOR_CAC_X_NORMFACTOR_TAG                   "x_normfactor"
#define CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG                    "y_normshift"
#define CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG                   "y_normfactor"
#define CALIB_SESNOR_CAC_X_OFFSET_TAG                       "x_offset"
#define CALIB_SESNOR_CAC_Y_OFFSET_TAG                       "y_offset"
#define CALIB_SESNOR_CAC_RED_PARAMETERS_TAG                 "red_parameters"
#define CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG                "blue_parameters"



/******************************************************************************
 * XML DPF section
 *****************************************************************************/
#define CALIB_SENSOR_DPF_TAG                                "DPF"
#define CALIB_SENSOR_DPF_NAME_TAG                           "name"
#define CALIB_SENSOR_DPF_RESOLUTION_TAG                     "resolution"
#define CALIB_SENSOR_DPF_ADPF_ENABLE_TAG                    "adpf_enable"
#define CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG               "NLL_SEGMENTATION"
#define CALIB_SENSOR_DPF_NLL_COEFF_TAG                      "nll_coeff_n"
#define CALIB_SENSOR_DPF_SIGMA_GREEN_TAG                    "SigmaGreen"
#define CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG                 "SigmaRedBlue"
#define CALIB_SENSOR_DPF_GRADIENT_TAG                       "Gradient"
#define CALIB_SENSOR_DPF_OFFSET_TAG                         "Offset"
#define CALIB_SENSOR_DPF_NLGAINS_TAG                        "NlGains"

#define CALIB_SENSOR_DPF_FILTERSETTING_TAG                   "FilterSetting"
#define CALIB_SENSOR_DPF_FILTERSETTING_NAME_TAG                   "name"
#define CALIB_SENSOR_DPF_FILTERENABLE_TAG                   "FilterEnable"
#define CALIB_SENSOR_DPF_DENOISELEVEL_TAG           "DeNoiseLevel"
#define CALIB_SENSOR_DPF_DENOISELEVEL_GAINS_TAG       "gains"
#define CALIB_SENSOR_DPF_DENOISELEVEL_DLEVEL_TAG      "dlevel"
#define CALIB_SENSOR_DPF_SHARPENINGLEVEL_TAG        "SharpeningLevel"
#define CALIB_SENSOR_DPF_SHARPENINGLEVEL_GAINS_TAG      "gains"
#define CALIB_SENSOR_DPF_SHARPENINGLEVEL_SLEVEL_TAG     "slevel"

#define CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_TAG  "FilterLevelRegConf"
#define CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_ENABLE_TAG  "FilterLevelRegConfEnable"
#define CALIB_SENSOR_DPF_FILT_LEVEL_TAG  "FilterLevel"
#define CALIB_SENSOR_DPF_FILT_CHR_H_MODE_TAG  "flt_chr_h_mode"
#define CALIB_SENSOR_DPF_FILT_CHR_V_MODE_TAG  "flt_chr_v_mode"
#define CALIB_SENSOR_DPF_FILT_GRN_STAGE1_TAG  "flt_grn_stage1"
#define CALIB_SENSOR_DPF_FILT_THRESH_BL0_TAG  "flt_thresh_bl0"
#define CALIB_SENSOR_DPF_FILT_THRESH_BL1_TAG  "flt_thresh_bl1"
#define CALIB_SENSOR_DPF_FILT_FAC_BL0_TAG  "flt_fac_bl0"
#define CALIB_SENSOR_DPF_FILT_FAC_BL1_TAG  "flt_fac_bl1"
#define CALIB_SENSOR_DPF_FILT_THRESH_SH0_TAG  "flt_thresh_sh0"
#define CALIB_SENSOR_DPF_FILT_THRESH_SH1_TAG  "flt_thresh_sh1"
#define CALIB_SENSOR_DPF_FILT_FAC_SH0_TAG  "flt_fac_sh0"
#define CALIB_SENSOR_DPF_FILT_FAC_SH1_TAG  "flt_fac_sh1"
#define CALIB_SENSOR_DPF_FILT_FAC_MID_TAG  "flt_fac_mid"

#define CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_CONF_TAG  "Demosaic_th_conf"
#define CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_GAIN_TAG  "gains"
#define CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_LEVEL_TAG  "demosaic_th_level"


#define CALIB_SENSOR_DSP_3DNR_SETTING_TAG  "DSP_3DNR_Setting"
#define CALIB_SENSOR_DSP_3DNR_SETTING_NAME_TAG  "name"
#define CALIB_SENSOR_DSP_3DNR_SETTING_ENABLE_TAG  "Enable"
#define CALIB_SENSOR_DSP_3DNR_SETTING_GAIN_LEVEL_TAG  "gain_level"
#define CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_NUMERATOR_TAG  "noise_coef_numerator"
#define CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_DENOMINATOR_TAG  "noise_coef_denominator"


#define CALIB_SENSOR_DSP_3DNR_SETTING_LEVEL_SETTING_TAG  "Level_Setting"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_EN_TAG  "luma_sp_nr_en"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_LEVEL_TAG  "luma_sp_nr_level"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_EN_TAG  "luma_te_nr_en"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_LEVEL_TAG  "luma_te_nr_level"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_EN_TAG  "chrm_sp_nr_en"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_LEVEL_TAG  "chrm_sp_nr_level"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_EN_TAG  "chrm_te_nr_en"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_LEVEL_TAG  "chrm_te_nr_level"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SHP_EN_TAG  "shp_en"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SHP_LEVEL_TAG  "shp_level"




#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SETTING_TAG  "Luma_Setting"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_DEFAULT_TAG  "luma_default"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_RAD_TAG  "luma_sp_rad"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_MAX_BI_NUM_TAG  "luma_te_max_bi_num"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_WEIGHT_TAG  "luma_w"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W00_TAG  "luma_w00"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W01_TAG  "luma_w01"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W02_TAG  "luma_w02"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W03_TAG  "luma_w03"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W04_TAG  "luma_w04"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W10_TAG  "luma_w10"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W11_TAG  "luma_w11"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W12_TAG  "luma_w12"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W13_TAG  "luma_w13"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W14_TAG  "luma_w14"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W20_TAG  "luma_w20"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W21_TAG  "luma_w21"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W22_TAG  "luma_w22"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W23_TAG  "luma_w23"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W24_TAG  "luma_w24"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W30_TAG  "luma_w30"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W31_TAG  "luma_w31"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W32_TAG  "luma_w32"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W33_TAG  "luma_w33"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W34_TAG  "luma_w34"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W40_TAG  "luma_w40"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W41_TAG  "luma_w41"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W42_TAG  "luma_w42"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W43_TAG  "luma_w43"
#define CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_W44_TAG  "luma_w44"

#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SETTING_TAG  "Chrm_Setting"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_DEFAULT_TAG  "chrm_default"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_RAD_TAG  "chrm_sp_rad"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_MAX_BI_NUM_TAG  "chrm_te_max_bi_num"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_WEIGHT_TAG  "chrm_w"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W00_TAG  "chrm_w00"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W01_TAG  "chrm_w01"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W02_TAG  "chrm_w02"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W03_TAG  "chrm_w03"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W04_TAG  "chrm_w04"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W10_TAG  "chrm_w10"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W11_TAG  "chrm_w11"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W12_TAG  "chrm_w12"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W13_TAG  "chrm_w13"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W14_TAG  "chrm_w14"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W20_TAG  "chrm_w20"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W21_TAG  "chrm_w21"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W22_TAG  "chrm_w22"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W23_TAG  "chrm_w23"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W24_TAG  "chrm_w24"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W30_TAG  "chrm_w30"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W31_TAG  "chrm_w31"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W32_TAG  "chrm_w32"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W33_TAG  "chrm_w33"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W34_TAG  "chrm_w34"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W40_TAG  "chrm_w40"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W41_TAG  "chrm_w41"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W42_TAG  "chrm_w42"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W43_TAG  "chrm_w43"
#define CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_W44_TAG  "chrm_w44"

#define CALIB_SENSOR_DSP_3DNR_SETTING_SHP_SETTING_TAG  "Shp_Setting"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SHP_DEFAULT_TAG  "shp_default"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_THR_TAG  "src_shp_thr"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_DIV_TAG  "src_shp_div"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_L_TAG  "src_shp_l"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_C_TAG  "src_shp_c"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_WEIGHT_TAG  "src_shp_w"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W00_TAG  "src_shp_w00"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W01_TAG  "src_shp_w01"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W02_TAG  "src_shp_w02"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W03_TAG  "src_shp_w03"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W04_TAG  "src_shp_w04"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W10_TAG  "src_shp_w10"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W11_TAG  "src_shp_w11"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W12_TAG  "src_shp_w12"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W13_TAG  "src_shp_w13"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W14_TAG  "src_shp_w14"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W20_TAG  "src_shp_w20"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W21_TAG  "src_shp_w21"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W22_TAG  "src_shp_w22"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W23_TAG  "src_shp_w23"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W24_TAG  "src_shp_w24"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W30_TAG  "src_shp_w30"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W31_TAG  "src_shp_w31"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W32_TAG  "src_shp_w32"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W33_TAG  "src_shp_w33"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W34_TAG  "src_shp_w34"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W40_TAG  "src_shp_w40"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W41_TAG  "src_shp_w41"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W42_TAG  "src_shp_w42"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W43_TAG  "src_shp_w43"
#define CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_W44_TAG  "src_shp_w44"







/******************************************************************************
 * XML DPCC section
 *****************************************************************************/
#define CALIB_SENSOR_DPCC_TAG                               "DPCC"
#define CALIB_SENSOR_DPCC_NAME_TAG                          "name"
#define CALIB_SENSOR_DPCC_RESOLUTION_TAG                    "resolution"
#define CALIB_SENSOR_DPCC_REGISTER_TAG                      "register"

#define CALIB_SENSOR_DPCC_REGISTER_NAME_TAG                 "name"
#define CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG                "value"

#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_MODE            "ISP_DPCC_MODE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_OUTPUT_MODE     "ISP_DPCC_OUT_MODE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_SET_USE         "ISP_DPCC_SET_USE"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_1   "ISP_DPCC_METHODS_SET1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_2   "ISP_DPCC_METHODS_SET2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_3   "ISP_DPCC_METHODS_SET3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_1   "ISP_DPCC_LINE_THRESH_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_1  "ISP_DPCC_LINE_MAD_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_1        "ISP_DPCC_PG_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_1    "ISP_DPCC_RND_THRESH_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_1        "ISP_DPCC_RG_FAC_1"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_2   "ISP_DPCC_LINE_THRESH_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_2  "ISP_DPCC_LINE_MAD_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_2        "ISP_DPCC_PG_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_2    "ISP_DPCC_RND_THRESH_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_2        "ISP_DPCC_RG_FAC_2"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_3   "ISP_DPCC_LINE_THRESH_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_3  "ISP_DPCC_LINE_MAD_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_3        "ISP_DPCC_PG_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_3    "ISP_DPCC_RND_THRESH_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_3        "ISP_DPCC_RG_FAC_3"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RO_LIMITS       "ISP_DPCC_RO_LIMITS"
#define CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_OFFS        "ISP_DPCC_RND_OFFS"


/******************************************************************************
 * XML CPROC section
 *****************************************************************************/
#define CALIB_SENSOR_CPROC_TAG                                       "CPROC"
#define CALIB_SENSOR_CPROC_NAME_TAG                                  "name"
#define CALIB_SENSOR_CPROC_SATURATION_TAG                            "saturation"
#define CALIB_SENSOR_CPROC_CONTRAST_TAG                              "contrast"
#define CALIB_SENSOR_CPROC_BRIGHTNESS_TAG                            "brightness"
#define CALIB_SENSOR_CPROC_HUE_TAG                                   "hue"

/******************************************************************************
 * XML system section
 *****************************************************************************/
#define CALIB_SYSTEM_TAG                                            "system"


/******************************************************************************
 * XML AWB section
 *****************************************************************************/
#define CALIB_SYSTEM_AFPS_TAG                                       "AFPS"

#define CALIB_SYSTEM_AFPS_DEFAULT_TAG                               "aFpsDefault"


/******************************************************************************
 * XML tag attributes
 *****************************************************************************/
#define CALIB_ATTRIBUTE_TYPE                                "type"
#define CALIB_ATTRIBUTE_TYPE_CHAR                           "char"
#define CALIB_ATTRIBUTE_TYPE_DOUBLE                         "double"
#define CALIB_ATTRIBUTE_TYPE_STRUCT                         "struct"
#define CALIB_ATTRIBUTE_TYPE_CELL                           "cell"

#define CALIB_ATTRIBUTE_SIZE                                "size"
#define CALIB_ATTRIBUTE_SIZE_FORMAT                         "[%d %d]"
#define CALIB_ATTRIBUTE_SIZE_NO_ELEMENTS                    2

#define CALIB_ATTRIBUTE_INDEX                               "index"

#endif /* __CALIB_TAGS_H__ */

