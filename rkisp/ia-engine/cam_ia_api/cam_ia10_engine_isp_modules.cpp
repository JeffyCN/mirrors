#include "cam_ia10_engine_isp_modules.h"
#include <ebase/utl_fixfloat.h>
#include <string.h>
#include <base/log.h>
#include <math.h>

#define ISP_CHECK_NULL(p) \
  if(!p) { \
    ALOGE("%s:%s is NULL!",__func__,#p);\
    return RET_FAILURE;\
  }

RESULT cam_ia10_isp_bls_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    CamCalibDbHandle_t        hCamCalibDb,
    uint16_t  width,
    uint16_t height,
    struct HAL_ISP_bls_cfg_s* bls_cfg,
    CamerIcIspBlsConfig_t* bls_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(bls_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    bls_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(bls_cfg);
    bls_result->enabled = BOOL_TRUE;
    bls_result->isp_bls_a_fixed = bls_cfg->fixed_red;
    bls_result->isp_bls_b_fixed = bls_cfg->fixed_greenR;
    bls_result->isp_bls_c_fixed = bls_cfg->fixed_greenB;
    bls_result->isp_bls_d_fixed = bls_cfg->fixed_blue;
    //TODO
    //bls_result->num_win
    //bls_result->Window1
    //bls_result->Window2
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    CamResolutionName_t res_name = {0};
    CamBlsProfile_t*         pBlsProfile = NULL;
    //get configs from xml
    bls_result->enabled = BOOL_TRUE;
    result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, width, height,  &res_name);
    if (RET_SUCCESS != result) {
      ALOGE("%s: resolution (%dx%d) not found in database\n", __FUNCTION__, width, height);
      return (result);
    }
    // get BLS calibration profile from database
    result = CamCalibDbGetBlsProfileByResolution(hCamCalibDb, res_name, &pBlsProfile);
    if (result != RET_SUCCESS) {
      ALOGE("%s: BLS profile %s not available (%d)\n", __FUNCTION__, res_name, result);
      return (result);
    }
    DCT_ASSERT(NULL != pBlsProfile);
    bls_result->isp_bls_a_fixed   =  pBlsProfile->level.uCoeff[CAM_4CH_COLOR_COMPONENT_RED];
    bls_result->isp_bls_b_fixed   =  pBlsProfile->level.uCoeff[CAM_4CH_COLOR_COMPONENT_GREENR];
    bls_result->isp_bls_c_fixed   =  pBlsProfile->level.uCoeff[CAM_4CH_COLOR_COMPONENT_GREENB];
    bls_result->isp_bls_d_fixed   =  pBlsProfile->level.uCoeff[CAM_4CH_COLOR_COMPONENT_BLUE];
    //TODO
    //bls_result->num_win
    //bls_result->Window1
    //bls_result->Window2
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_dpcc_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpcc_cfg_s* dpcc_cfg,
    CamCalibDbHandle_t        hCamCalibDb,
    uint16_t  width,
    uint16_t height,
    CamerIcDpccConfig_t* dpcc_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(dpcc_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    dpcc_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(dpcc_cfg);
    dpcc_result->enabled = BOOL_TRUE;
    dpcc_result->isp_dpcc_mode          = dpcc_cfg->isp_dpcc_mode;
    dpcc_result->isp_dpcc_output_mode     = dpcc_cfg->isp_dpcc_output_mode;
    dpcc_result->isp_dpcc_set_use         = dpcc_cfg->isp_dpcc_set_use;

    dpcc_result->isp_dpcc_methods_set_1   = dpcc_cfg->isp_dpcc_methods_set_1;
    dpcc_result->isp_dpcc_methods_set_2   = dpcc_cfg->isp_dpcc_methods_set_2;
    dpcc_result->isp_dpcc_methods_set_3   = dpcc_cfg->isp_dpcc_methods_set_3;

    dpcc_result->isp_dpcc_line_thresh_1   = dpcc_cfg->isp_dpcc_line_thresh_1;
    dpcc_result->isp_dpcc_line_mad_fac_1  = dpcc_cfg->isp_dpcc_line_mad_fac_1;
    dpcc_result->isp_dpcc_pg_fac_1        = dpcc_cfg->isp_dpcc_pg_fac_1;
    dpcc_result->isp_dpcc_rnd_thresh_1    = dpcc_cfg->isp_dpcc_rnd_thresh_1;
    dpcc_result->isp_dpcc_rg_fac_1        = dpcc_cfg->isp_dpcc_rg_fac_1;

    dpcc_result->isp_dpcc_line_thresh_2   = dpcc_cfg->isp_dpcc_line_thresh_2;
    dpcc_result->isp_dpcc_line_mad_fac_2  = dpcc_cfg->isp_dpcc_line_mad_fac_2;
    dpcc_result->isp_dpcc_pg_fac_2        = dpcc_cfg->isp_dpcc_pg_fac_2;
    dpcc_result->isp_dpcc_rnd_thresh_2    = dpcc_cfg->isp_dpcc_rnd_thresh_2;
    dpcc_result->isp_dpcc_rg_fac_2        = dpcc_cfg->isp_dpcc_rg_fac_2;

    dpcc_result->isp_dpcc_line_thresh_3   = dpcc_cfg->isp_dpcc_line_thresh_3;
    dpcc_result->isp_dpcc_line_mad_fac_3  = dpcc_cfg->isp_dpcc_line_mad_fac_3;
    dpcc_result->isp_dpcc_pg_fac_3        = dpcc_cfg->isp_dpcc_pg_fac_3;
    dpcc_result->isp_dpcc_rnd_thresh_3    = dpcc_cfg->isp_dpcc_rnd_thresh_3;
    dpcc_result->isp_dpcc_rg_fac_3        = dpcc_cfg->isp_dpcc_rg_fac_3;

    dpcc_result->isp_dpcc_ro_limits       = dpcc_cfg->isp_dpcc_ro_limits;
    dpcc_result->isp_dpcc_rnd_offs        = dpcc_cfg->isp_dpcc_rnd_offs;
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    CamResolutionName_t res_name = {0};
    CamDpccProfile_t*        pDpccProfile = NULL;
    //get configs from xml
    dpcc_result->enabled = BOOL_TRUE;
    result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, width, height,  &res_name);
    if (RET_SUCCESS != result) {
      ALOGE("%s: resolution (%dx%d) not found in database\n", __FUNCTION__, width, height);
      return (result);
    }
    // get dpf-profile from calibration database
    result = CamCalibDbGetDpccProfileByResolution(hCamCalibDb, res_name, &pDpccProfile);
    if (result != RET_SUCCESS) {
      ALOGE("%s: Getting DPCC profile for resolution %s from calibration database failed (%d)\n",
            __FUNCTION__, res_name, result);
      return (result);
    }
    DCT_ASSERT(NULL != pDpccProfile);
    dpcc_result->isp_dpcc_mode            = pDpccProfile->isp_dpcc_mode;
    dpcc_result->isp_dpcc_output_mode     = pDpccProfile->isp_dpcc_output_mode;
    dpcc_result->isp_dpcc_set_use         = pDpccProfile->isp_dpcc_set_use;

    dpcc_result->isp_dpcc_methods_set_1   = pDpccProfile->isp_dpcc_methods_set_1;
    dpcc_result->isp_dpcc_methods_set_2   = pDpccProfile->isp_dpcc_methods_set_2;
    dpcc_result->isp_dpcc_methods_set_3   = pDpccProfile->isp_dpcc_methods_set_3;

    dpcc_result->isp_dpcc_line_thresh_1   = pDpccProfile->isp_dpcc_line_thresh_1;
    dpcc_result->isp_dpcc_line_mad_fac_1  = pDpccProfile->isp_dpcc_line_mad_fac_1;
    dpcc_result->isp_dpcc_pg_fac_1        = pDpccProfile->isp_dpcc_pg_fac_1;
    dpcc_result->isp_dpcc_rnd_thresh_1    = pDpccProfile->isp_dpcc_rnd_thresh_1;
    dpcc_result->isp_dpcc_rg_fac_1        = pDpccProfile->isp_dpcc_rg_fac_1;

    dpcc_result->isp_dpcc_line_thresh_2   = pDpccProfile->isp_dpcc_line_thresh_2;
    dpcc_result->isp_dpcc_line_mad_fac_2  = pDpccProfile->isp_dpcc_line_mad_fac_2;
    dpcc_result->isp_dpcc_pg_fac_2        = pDpccProfile->isp_dpcc_pg_fac_2;
    dpcc_result->isp_dpcc_rnd_thresh_2    = pDpccProfile->isp_dpcc_rnd_thresh_2;
    dpcc_result->isp_dpcc_rg_fac_2        = pDpccProfile->isp_dpcc_rg_fac_2;

    dpcc_result->isp_dpcc_line_thresh_3   = pDpccProfile->isp_dpcc_line_thresh_3;
    dpcc_result->isp_dpcc_line_mad_fac_3  = pDpccProfile->isp_dpcc_line_mad_fac_3;
    dpcc_result->isp_dpcc_pg_fac_3        = pDpccProfile->isp_dpcc_pg_fac_3;
    dpcc_result->isp_dpcc_rnd_thresh_3    = pDpccProfile->isp_dpcc_rnd_thresh_3;
    dpcc_result->isp_dpcc_rg_fac_3        = pDpccProfile->isp_dpcc_rg_fac_3;

    dpcc_result->isp_dpcc_ro_limits       = pDpccProfile->isp_dpcc_ro_limits;
    dpcc_result->isp_dpcc_rnd_offs        = pDpccProfile->isp_dpcc_rnd_offs;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_sdg_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_sdg_cfg_s* sdg_cfg,
    CamerIcIspDegammaCurve_t* sdg_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(sdg_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    sdg_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    unsigned int ind = 0;
    ISP_CHECK_NULL(sdg_cfg);
    sdg_result->enabled = BOOL_TRUE;

    for (ind = 0; ((ind < sdg_cfg->used_dx_cnt) && \
                   (ind < (CAMERIC_DEGAMMA_CURVE_SIZE - 1)));
         ind++) {
      int j = 11;
      for (j = 11; j >= 5 ; j--)
        if (sdg_cfg->gamma_dx[ind] >= (2 << j))
          break;
      sdg_result->segment[ind] = j - 4;
    }

    for (ind = 0; ((ind < (sdg_cfg->used_dx_cnt + 1)) && \
                   (ind < CAMERIC_DEGAMMA_CURVE_SIZE));
         ind++)  {
      sdg_result->red[ind]    = sdg_cfg->gamma_r[ind];
      sdg_result->green[ind]  = sdg_cfg->gamma_g[ind];
      sdg_result->blue[ind]     = sdg_cfg->gamma_b[ind];
    }

  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //default is active
    unsigned int ind = 0;
    int16_t  def_curve[CAMERIC_DEGAMMA_CURVE_SIZE] = {
      0x0000, 0x0100, 0x0200, 0x0300,
      0x0400, 0x0500, 0x0600, 0x0700,
      0x0800, 0x0900, 0x0A00, 0x0B00,
      0x0C00, 0x0D00, 0x0E00, 0x0F00,
      0x0FFF
    };
    sdg_result->enabled = BOOL_TRUE;
    for (ind = 0; ind < (CAMERIC_DEGAMMA_CURVE_SIZE - 1); ind++)
      sdg_result->segment[ind] = 0x4;
    for (ind = 0; (ind < CAMERIC_DEGAMMA_CURVE_SIZE); ind++) {
      sdg_result->red[ind]    = def_curve[ind];
      sdg_result->green[ind]  = def_curve[ind];
      sdg_result->blue[ind]     = def_curve[ind];
    }

  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_flt_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_flt_cfg_s* flt_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcIspFltConfig_t* flt_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(flt_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    flt_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
  	CamDpfProfile_t* pDpfProfile_t = NULL;
	CamResolutionName_t ResName;   
	result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, drv_width, drv_height, &ResName);
	if (RET_SUCCESS != result) {
	  LOGD("%s: resolution (%dx%d) not found in database\n", drv_width, drv_height);
	}else{
	  result = CamCalibDbGetDpfProfileByResolution(hCamCalibDb, ResName, &pDpfProfile_t);
	  if (RET_SUCCESS != result) {
	    LOGD("%s: get dpf fail (%dx%d) (%s) not found in database\n", drv_width, drv_height, ResName);
	  } 
	}
	
    uint32_t isp_filt_thresh_sh0 = 0U;
    uint32_t isp_filt_thresh_sh1 = 0U;
    uint32_t isp_filt_thresh_bl0 = 0U;
    uint32_t isp_filt_thresh_bl1 = 0U;

    uint32_t isp_filt_fac_sh0 = 0U;
    uint32_t isp_filt_fac_sh1 = 0U;
    uint32_t isp_filt_fac_mid = 0U;
    uint32_t isp_filt_fac_bl0 = 0U;
    uint32_t isp_filt_fac_bl1 = 0U;

    uint32_t isp_filt_mode    = 0UL;

    ISP_CHECK_NULL(flt_cfg);
    flt_result->enabled = BOOL_TRUE;
    /* 1 : dynamic, 0: static*/
    flt_result->mode = 1;
    flt_result->lum_weight = 0x00032040 ;
    switch (flt_cfg->denoise_level) {
      case 0: {
        /* NoiseReductionLevel = 0 */
        flt_result->thresh_sh0 = 0;
        flt_result->thresh_sh1 = 0;
        flt_result->thresh_bl0 = 0;
        flt_result->thresh_bl1 = 0;
        flt_result->chr_h_mode = 0;
        flt_result->chr_v_mode = 1;
        flt_result->grn_stage1 = 6;
        break;
      }

      case 1: {
        /* NoiseReductionLevel = 1 */
        flt_result->thresh_sh0 = 18;
        flt_result->thresh_sh1 = 33;
        flt_result->thresh_bl0 = 8;
        flt_result->thresh_bl1 = 2;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 6;
        break;
      }

      case 2: {
        /* NoiseReductionLevel = 2 */
        flt_result->thresh_sh0 = 26;
        flt_result->thresh_sh1 = 44;
        flt_result->thresh_bl0 = 13;
        flt_result->thresh_bl1 = 5;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 4;
        break;
      }

      case 3: {
        /* NoiseReductionLevel = 3; */
        flt_result->thresh_sh0 = 36;
        flt_result->thresh_sh1 = 51;
        flt_result->thresh_bl0 = 23;
        flt_result->thresh_bl1 = 10;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 2;
        break;
      }

      case 4: {
        /* NoiseReductionLevel = 4; */
        flt_result->thresh_sh0 = 41;
        flt_result->thresh_sh1 = 67;
        flt_result->thresh_bl0 = 26;
        flt_result->thresh_bl1 = 15;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 3;
        break;
      }

      case 5: {
        /* NoiseReductionLevel = 5; */
        flt_result->thresh_sh0 = 75;
        flt_result->thresh_sh1 = 100;
        flt_result->thresh_bl0 = 50;
        flt_result->thresh_bl1 = 20;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 3;
        break;
      }

      case 6: {
        /* NoiseReductionLevel = 6; */
        flt_result->thresh_sh0 = 90;
        flt_result->thresh_sh1 = 120;
        flt_result->thresh_bl0 = 60;
        flt_result->thresh_bl1 = 26;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 2;
        break;
      }

      case 7: {
        /* NoiseReductionLevel = 7; */
        flt_result->thresh_sh0 = 120;
        flt_result->thresh_sh1 = 150;
        flt_result->thresh_bl0 = 80;
        flt_result->thresh_bl1 = 51;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 2;
        break;
      }

      case 8: {
        /* NoiseReductionLevel = 8; */
        flt_result->thresh_sh0 = 170;
        flt_result->thresh_sh1 = 200;
        flt_result->thresh_bl0 = 140;
        flt_result->thresh_bl1 = 100;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = 2;
        break;
      }

      case 9: {
        /* NoiseReductionLevel = 9; */
        flt_result->thresh_sh0 = 250;
        flt_result->thresh_sh1 = 300;
        flt_result->thresh_bl0 = 180;
        flt_result->thresh_bl1 = 150;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = (flt_cfg->sharp_level > 3) ? 2 : 1  ;
        break;
      }

      case 10: {
        /* NoiseReductionLevel = 10; extrem noise */
        flt_result->thresh_sh0 = 1023;
        flt_result->thresh_sh1 = 1023;
        flt_result->thresh_bl0 = 1023;
        flt_result->thresh_bl1 = 1023;
        flt_result->chr_h_mode = 3;
        flt_result->chr_v_mode = 3;
        flt_result->grn_stage1 = (flt_cfg->sharp_level > 5) ? 2 : (flt_cfg->sharp_level > 3) ? 1 : 0 ;
        break;
      }

      default: {
        return (RET_OUTOFRANGE);
      }
    }

    switch (flt_cfg->sharp_level) {

      case 0: {
        /* SharpeningLevel = 0; no sharp enhancement */
        flt_result->fac_sh0 = 0x4;
        flt_result->fac_sh1 = 0x4;
        flt_result->fac_mid = 0x4;
        flt_result->fac_bl0 = 0x2;
        flt_result->fac_bl1 = 0x0;

        break;
      }

      case 1: {
        /* SharpeningLevel = 1; */
        flt_result->fac_sh0 = 0x7;
        flt_result->fac_sh1 = 0x8;
        flt_result->fac_mid = 0x6;
        flt_result->fac_bl0 = 0x2;
        flt_result->fac_bl1 = 0x0;
        break;
      }

      case 2: {
        /* SharpeningLevel = 2; */
        flt_result->fac_sh0 = 0xa;
        flt_result->fac_sh1 = 0xc;
        flt_result->fac_mid = 0x8;
        flt_result->fac_bl0 = 0x4;
        flt_result->fac_bl1 = 0x0;
        break;
      }

      case 3: {
        /* SharpeningLevel = 3; */
        flt_result->fac_sh0 = 0xc;
        flt_result->fac_sh1 = 0x10;
        flt_result->fac_mid = 0xa;
        flt_result->fac_bl0 = 0x6;
        flt_result->fac_bl1 = 0x2;
        break;
      }

      case 4: {
        /* SharpeningLevel = 4; */
        flt_result->fac_sh0 = 0x10;
        flt_result->fac_sh1 = 0x16;
        flt_result->fac_mid = 0xc;
        flt_result->fac_bl0 = 0x8;
        flt_result->fac_bl1 = 0x4;
        break;
      }

      case 5: {
        /* SharpeningLevel = 5; */
        flt_result->fac_sh0 = 0x14;
        flt_result->fac_sh1 = 0x1b;
        flt_result->fac_mid = 0x10;
        flt_result->fac_bl0 = 0xa;
        flt_result->fac_bl1 = 0x4;
        break;
      }

      case 6: {
        /* SharpeningLevel = 6; */
        flt_result->fac_sh0 = 0x1a;
        flt_result->fac_sh1 = 0x20;
        flt_result->fac_mid = 0x13;
        flt_result->fac_bl0 = 0xc;
        flt_result->fac_bl1 = 0x6;
        break;
      }

      case 7: {
        /* SharpeningLevel = 7; */
        flt_result->fac_sh0 = 0x1e;
        flt_result->fac_sh1 = 0x26;
        flt_result->fac_mid = 0x17;
        flt_result->fac_bl0 = 0x10;
        flt_result->fac_bl1 = 0x8;
        break;
      }

      case 8: {
        /* SharpeningLevel = 8; */
        flt_result->thresh_sh0 = 0x13;
        if (flt_result->thresh_sh1 > 0x8a)
          flt_result->thresh_sh1 = 0x8a;
        flt_result->fac_sh0 = 0x24;
        flt_result->fac_sh1 = 0x2c;
        flt_result->fac_mid = 0x1d;
        flt_result->fac_bl0 = 0x15;
        flt_result->fac_bl1 = 0xd;
        break;
      }

      case 9: {
        /* SharpeningLevel = 9; */
        flt_result->thresh_sh0 = 0x13;
        if (flt_result->thresh_sh1 > 0x8a)
          flt_result->thresh_sh1 = 0x8a;
        flt_result->fac_sh0 = 0x2a;
        flt_result->fac_sh1 = 0x30;
        flt_result->fac_mid = 0x22;
        flt_result->fac_bl0 = 0x1a;
        flt_result->fac_bl1 = 0x14;
        break;
      }

      case 10: {
        /* SharpeningLevel = 10; */
        flt_result->fac_sh0 = 0x30;
        flt_result->fac_sh1 = 0x3f;
        flt_result->fac_mid = 0x28;
        flt_result->fac_bl0 = 0x24;
        flt_result->fac_bl1 = 0x20;
        break;
      }

      default: {
        return (RET_OUTOFRANGE);
      }
    }

    if (flt_cfg->denoise_level > 7) {
      if (flt_cfg->sharp_level > 7) {
        flt_result->fac_bl0 = flt_result->fac_bl0 >> 1;// * 0.50
        flt_result->fac_bl1 = flt_result->fac_bl1 >> 2;// * 0.25
      } else if (flt_cfg->sharp_level > 4) {
        flt_result->fac_bl0 = (flt_result->fac_bl0 * 3) >> 2;// * 0.75
        flt_result->fac_bl1 = flt_result->fac_bl1 >> 2;//  * 0.50
      }
    }
    flt_result->denoise_level = flt_cfg->denoise_level;
    flt_result->sharp_level = flt_cfg->sharp_level;

	//oyyf change filter level reg value by xml setting
	CamFilterProfile_t* pFilterProfile = NULL;
	if(flt_cfg != NULL){
	    enum LIGHT_MODE LightMode = (enum LIGHT_MODE)flt_cfg->light_mode;
		int no_filter;
		result = CamCalibDbGetNoOfFilterProfile(hCamCalibDb, pDpfProfile_t, &no_filter);
		if(result != RET_SUCCESS){
			LOGD("fail to get no FilterProfile, ret: %d", result);
		}else{
			if(no_filter > 0 && pDpfProfile_t != NULL){
				if(LightMode <= LIGHT_MODE_MIN ||  LightMode >= LIGHT_MODE_MAX || LightMode > no_filter)
					LightMode = LIGHT_MODE_DAY;
			    result = CamCalibDbGetFilterProfileByIdx(hCamCalibDb, pDpfProfile_t, LightMode, &pFilterProfile);
			    if (result != RET_SUCCESS) {
			      LOGD("fail to get pGocGlobal, ret: %d", result);
			    }
			}
		}
	}
	
	if(pFilterProfile != NULL && pFilterProfile->FiltLevelRegConf.FiltLevelRegConfEnable == 1) {
	  for(int i=0; i < pFilterProfile->FiltLevelRegConf.ArraySize; i++) {
	  	#if 0
	  	 LOGD("%s:111 oyyf filter[%d] %d %d %d %d !", __func__, 
		  	i, pFilterProfile->FiltLevelRegConf.p_fac_sh0[i], pFilterProfile->FiltLevelRegConf.p_fac_sh1[i],
		  	pFilterProfile->FiltLevelRegConf.p_fac_mid[i]);
		#endif
		
		if(flt_cfg->denoise_level == pFilterProfile->FiltLevelRegConf.p_FiltLevel[i]){
          flt_result->chr_h_mode = pFilterProfile->FiltLevelRegConf.p_chr_h_mode[i];
          flt_result->chr_v_mode = pFilterProfile->FiltLevelRegConf.p_chr_v_mode[i];
          flt_result->grn_stage1 = pFilterProfile->FiltLevelRegConf.p_grn_stage1[i];
		}

		if(flt_cfg->sharp_level == pFilterProfile->FiltLevelRegConf.p_FiltLevel[i]){
		  flt_result->fac_bl0 = pFilterProfile->FiltLevelRegConf.p_fac_bl0[i];
          flt_result->fac_bl1 = pFilterProfile->FiltLevelRegConf.p_fac_bl1[i];
          flt_result->thresh_bl0 = pFilterProfile->FiltLevelRegConf.p_thresh_bl0[i];
          flt_result->thresh_bl1 = pFilterProfile->FiltLevelRegConf.p_thresh_bl1[i];
		  flt_result->thresh_sh0 = pFilterProfile->FiltLevelRegConf.p_thresh_sh0[i];
          flt_result->thresh_sh1 = pFilterProfile->FiltLevelRegConf.p_thresh_sh1[i];
          flt_result->fac_sh0 = pFilterProfile->FiltLevelRegConf.p_fac_sh0[i];
          flt_result->fac_sh1 = pFilterProfile->FiltLevelRegConf.p_fac_sh1[i];
          flt_result->fac_mid = pFilterProfile->FiltLevelRegConf.p_fac_mid[i]; 
		}	
	  }
   }
	#if 0
	LOGD("%s: flt_cfg->sharp_level %d %d %d %d %p!", __func__, 
		  	flt_cfg->sharp_level, flt_result->fac_sh0, flt_result->fac_sh1,
		  	flt_result->fac_mid, pFilterProfile);
	#endif
	 
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //flt_result->enabled = BOOL_FALSE;
    struct HAL_ISP_flt_cfg_s flt_default_cfg = {2, 5, 0};
    cam_ia10_isp_flt_config(hCamCalibDb, HAL_ISP_ACTIVE_SETTING, &flt_default_cfg, drv_width, drv_height, flt_result);
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;

}
#if 0
static uint16_t cam_ia_goc_def_gamma_y[CAMERIC_ISP_GAMMA_CURVE_SIZE] = {
  0x000, 0x049, 0x089, 0x0B7,
  0x0DF, 0x11F, 0x154, 0x183,
  0x1AD, 0x1F6, 0x235, 0x26F,
  0x2D3, 0x32A, 0x378, 0x3BF,
  0x3FF,
};
//[0 73 137 183 223 287 340 387 429 502 565 623 723 810 888 959 1023]
#endif
// only define the 34 segments goc, if other number of segments is
// required, we could map from this.
static uint16_t cam_ia_goc_def_gamma_y[CAMERIC_ISP_GAMMA_CURVE_SIZE] = {
  0x000, 0x092, 0x124, 0x1a4,
  0x224, 0x280, 0x2dc, 0x32c,
  0x37c, 0x3fc, 0x47c, 0x4e6,
  0x550, 0x5ae, 0x60c, 0x660,
  0x6b4, 0x746, 0x7d8, 0x856,
  0x8d4, 0x948, 0x9bc, 0xa84,
  0xb4c, 0xbfa, 0xca8, 0xd44,
  0xde0, 0xe6e, 0xefc, 0xf7c,
  0xfff, 0x000
};

void cam_ia10_isp_goc_map_34_to_17
(
     uint16_t* goc_in,
     uint16_t* goc_out
) {
    int i;

    for (i = 0; i < 17; i++) {
        goc_out[i] = goc_in[i * 2] / 4;
    }
}

RESULT cam_ia10_isp_goc_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_goc_cfg_s* goc_cfg,
    CamerIcIspGocConfig_t* goc_result,
    bool_t WDR_enable_mode,
    int isp_ver
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(goc_result);

  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    int ind = 0;
    ISP_CHECK_NULL(goc_cfg);
    goc_result->enabled = BOOL_FALSE;
    goc_result->mode = (CamerIcIspGammaSegmentationMode_t)(goc_cfg->mode);
    if (isp_ver > 0) {
        if (goc_cfg->used_cnt != 34)
            LOGE("goc segment count %d is error for isp ver %d",
                 goc_cfg->used_cnt, isp_ver);
        for (ind = 0; (ind < goc_cfg->used_cnt); ind++)
          goc_result->gamma_y.GammaY[ind] = goc_cfg->gamma_y[ind];

    } else {
        cam_ia10_isp_goc_map_34_to_17(goc_cfg->gamma_y,
                                      goc_result->gamma_y.GammaY);
    }
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    int ind = 0;
    ISP_CHECK_NULL(goc_cfg);
    goc_result->enabled = BOOL_TRUE;
    goc_result->mode = (CamerIcIspGammaSegmentationMode_t)(goc_cfg->mode);
    if (isp_ver > 0) {
        if (goc_cfg->used_cnt != 34)
            LOGE("goc segment count %d is error for isp ver %d",
                 goc_cfg->used_cnt, isp_ver);
        for (ind = 0; (ind < goc_cfg->used_cnt); ind++)
          goc_result->gamma_y.GammaY[ind] = goc_cfg->gamma_y[ind];

    } else {
        cam_ia10_isp_goc_map_34_to_17(goc_cfg->gamma_y,
                                      goc_result->gamma_y.GammaY);
    }
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //default gamma 2.2
    //goc_result->enabled = BOOL_TRUE;

    CamCalibGocProfile_t* pGocProfile = NULL;
	if(goc_cfg != NULL){
	    enum LIGHT_MODE LightMode = (enum LIGHT_MODE)goc_cfg->light_mode;
		int no_goc;
		result = CamCalibDbGetNoOfGocProfile(hCamCalibDb, &no_goc);
		if(result != RET_SUCCESS){
			LOGD("fail to get no GocProfile, ret: %d", result);
		}else{
			if(no_goc > 0){
				if(LightMode <= LIGHT_MODE_MIN ||  LightMode >= LIGHT_MODE_MAX || LightMode > no_goc)
					LightMode = LIGHT_MODE_DAY;
			    result = CamCalibDbGetGocProfileByIdx(hCamCalibDb, LightMode, &pGocProfile);
			    if (result != RET_SUCCESS) {
			      LOGD("fail to get pGocGlobal, ret: %d", result);
			    }
			}
		}
	}
	
    HAL_ISP_ACTIVE_MODE goc_enable_mode = HAL_ISP_ACTIVE_SETTING;
    struct HAL_ISP_goc_cfg_s  goc_def_cfg;
    goc_def_cfg.mode = HAL_ISP_GAMMA_SEG_MODE_LOGARITHMIC;
    goc_def_cfg.used_cnt =  CAMERIC_ISP_GAMMA_CURVE_SIZE;

    if (pGocProfile != NULL) {
      goc_enable_mode = pGocProfile->enable_mode < 0 ?
                        HAL_ISP_ACTIVE_SETTING : HAL_ISP_ACTIVE_MODE(pGocProfile->enable_mode);
      goc_def_cfg.mode = pGocProfile->def_cfg_mode < 0 ?
                         HAL_ISP_GAMMA_SEG_MODE_LOGARITHMIC : HAL_ISP_GAMMA_SEG_MODE_e(pGocProfile->def_cfg_mode);
    }

    if (WDR_enable_mode == BOOL_FALSE && pGocProfile != NULL && pGocProfile->GammaY[CAMERIC_ISP_GAMMA_CURVE_SIZE / 2 - 1] > 0) {
      for (int index = 0; index < (CAMERIC_ISP_GAMMA_CURVE_SIZE); index++) {
        goc_def_cfg.gamma_y[index] = pGocProfile->GammaY[index];
      }
    } else {
      for (int index = 0; index < (CAMERIC_ISP_GAMMA_CURVE_SIZE); index++) {
        goc_def_cfg.gamma_y[index] = cam_ia_goc_def_gamma_y[index];
      }
    }

    result = cam_ia10_isp_goc_config(hCamCalibDb, goc_enable_mode,
                                     &goc_def_cfg, goc_result,
                                     WDR_enable_mode, isp_ver);
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_cproc_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_cproc_cfg_s* cproc_cfg,
    CamerIcCprocConfig_t* cproc_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(cproc_result);
  
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    cproc_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(cproc_cfg);
    cproc_result->enabled = BOOL_TRUE;
    cproc_result->LumaIn = (CamerIcCprocLuminanceRangeIn_t)(cproc_cfg->range);
    cproc_result->LumaOut = (CamerIcCprocLuminanceRangeOut_t)(cproc_cfg->range);
    cproc_result->ChromaOut = (CamerIcCprocChrominaceRangeOut_t)(cproc_cfg->range);
    cproc_result->brightness = UtlFloatToFix_S0800((float)(cproc_cfg->cproc.brightness));
    cproc_result->contrast = UtlFloatToFix_U0107(cproc_cfg->cproc.contrast);
    cproc_result->saturation = UtlFloatToFix_U0107((float)(cproc_cfg->cproc.saturation));
    if (cproc_cfg->cproc.hue < 0.0f)
      cproc_result->hue = (uint8_t)((cproc_cfg->cproc.hue) * -128.0f / 90.0f);
    else
      cproc_result->hue = (uint8_t)((cproc_cfg->cproc.hue) * 128.0f / 90.0f);
#if 0
    //ignore sharpness,sharpness was set in flt module
    ALOGE("%s:set  cproc use_case %d, ct=%f,hue=%f,bt=%d,st=%f", __func__,
          cproc_cfg->use_case,
          cproc_cfg->cproc.contrast,
          cproc_cfg->cproc.hue,
          cproc_cfg->cproc.brightness,
          cproc_cfg->cproc.saturation);
#endif
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    CamCprocProfile_t* cproc_profile;
    CamCprocData_t*  cproc_data = NULL;
    HAL_ISP_COLOR_RANGE_e range;

    result = CamCalibDbGetCproc(hCamCalibDb, &cproc_profile);
    if (result != RET_SUCCESS) {
      ALOGE("fail to get cproc_profile, ret: %d", result);
    }
    if (cproc_profile != NULL) {
      if (cproc_cfg) {
        if (cproc_cfg->use_case == UC_PREVIEW) {
          cproc_data = &cproc_profile->cproc[CAM_CPROC_USECASE_PREVIEW];
          range = HAL_ISP_COLOR_RANGE_OUT_BT601;
        } else if (cproc_cfg->use_case == UC_CAPTURE) {
          cproc_data = &cproc_profile->cproc[CAM_CPROC_USECASE_CAPTURE];
          range = HAL_ISP_COLOR_RANGE_OUT_FULL_RANGE;
        } else if (cproc_cfg->use_case == UC_RECORDING) {
          cproc_data = &cproc_profile->cproc[CAM_CPROC_USECASE_VIDEO];
          range = HAL_ISP_COLOR_RANGE_OUT_BT601;
        } else
          ALOGE("%s:error uscase %d !", __func__, cproc_cfg->use_case);
        if (cproc_data) {
          cproc_result->enabled = BOOL_TRUE;
          cproc_result->LumaIn = (CamerIcCprocLuminanceRangeIn_t)(range);
          cproc_result->LumaOut = (CamerIcCprocLuminanceRangeOut_t)(range);
          cproc_result->ChromaOut = (CamerIcCprocChrominaceRangeOut_t)(range);
          cproc_result->brightness = UtlFloatToFix_S0800((float)(cproc_data->cproc_brightness));
          cproc_result->contrast = UtlFloatToFix_U0107(cproc_data->cproc_contrast);
          cproc_result->saturation = UtlFloatToFix_U0107((float)(cproc_data->cproc_saturation));
		  if (cproc_cfg->cproc.hue < 0.0f)
			cproc_result->hue = (uint8_t)((cproc_cfg->cproc.hue) * -128.0f / 90.0f);
		  else
			cproc_result->hue = (uint8_t)((cproc_cfg->cproc.hue) * 128.0f / 90.0f);
#if 0
          ALOGE("%s:set xml cproc use_case %d, ct=%f,hue=%f,bt=%d,st=%f", __func__,
                cproc_cfg->use_case,
                cproc_data->cproc_contrast,
                cproc_data->cproc_hue,
                cproc_data->cproc_brightness,
                cproc_data->cproc_saturation);
#endif
        }
      } else {
        ALOGE("%s:cproc_cfg is NULL!", __func__);
        cproc_result->enabled = BOOL_FALSE;
      }

    } else {
      ALOGE("%s:cproc profile doesn't exist!", __func__);
      cproc_result->enabled = BOOL_FALSE;
    }

  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_ie_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_ie_cfg_s* ie_cfg,
    CamerIcIeConfig_t* ie_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(ie_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    ie_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(ie_cfg);
    ie_result->range = (CamerIcIeRange_t)(ie_cfg->range);
    switch (ie_cfg->mode) {
      case HAL_EFFECT_NONE:
        ie_result->enabled = BOOL_FALSE;
        break;
      case HAL_EFFECT_MONO:
        ie_result->mode = CAMERIC_IE_MODE_GRAYSCALE;
        ie_result->enabled = BOOL_TRUE;
        break;
      case HAL_EFFECT_NEGATIVE:
        ie_result->mode = CAMERIC_IE_MODE_NEGATIVE;
        ie_result->enabled = BOOL_TRUE;
        break;
      case HAL_EFFECT_EMBOSS:
        ie_result->mode = CAMERIC_IE_MODE_EMBOSS;
        ie_result->ModeConfig.Emboss.coeff[0] = 0x9;//  2
        ie_result->ModeConfig.Emboss.coeff[1] = 0x0;// 0
        ie_result->ModeConfig.Emboss.coeff[2] = 0x0;// 0
        ie_result->ModeConfig.Emboss.coeff[3] = 0x8;// 1
        ie_result->ModeConfig.Emboss.coeff[4] = 0x0;// 0
        ie_result->ModeConfig.Emboss.coeff[5] = 0xc;// -1
        ie_result->ModeConfig.Emboss.coeff[6] = 0x0;// 0x0
        ie_result->ModeConfig.Emboss.coeff[7] = 0xc;// -1
        ie_result->ModeConfig.Emboss.coeff[8] = 0x9;// 2
        ie_result->enabled = BOOL_TRUE;
        break;
      case HAL_EFFECT_SKETCH:
        ie_result->mode = CAMERIC_IE_MODE_SKETCH;
        ie_result->ModeConfig.Sketch.coeff[0] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[1] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[2] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[3] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[4] = 0xb;// 0x8
        ie_result->ModeConfig.Sketch.coeff[5] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[6] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[7] = 0xc;//-1
        ie_result->ModeConfig.Sketch.coeff[8] = 0xc;//-1
        ie_result->enabled = BOOL_TRUE;
        break;
      case HAL_EFFECT_SHARPEN:
        ie_result->mode = CAMERIC_IE_MODE_SHARPEN;
        ie_result->ModeConfig.Sharpen.coeff[0] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[1] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[2] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[3] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[4] = 0xb;// 0x8
        ie_result->ModeConfig.Sharpen.coeff[5] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[6] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[7] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.coeff[8] = 0xc;//-1
        ie_result->ModeConfig.Sharpen.factor = UtlFloatToFix_U0402(ie_cfg->sharp_fact);
        ie_result->ModeConfig.Sharpen.threshold = ie_cfg->sharp_thd;
        ie_result->enabled = BOOL_TRUE;
        break;
      default :
        result = RET_FAILURE;
    }
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    ie_result->enabled = BOOL_FALSE;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

/******************************************************************************
 * UpdateStepSize()
 *****************************************************************************/
RESULT cam_ia10_isp_hst_update_stepSize
(
    const CamerIcIspHistMode_t  mode,
    const CamerIcHistWeights_t  weights,
    const uint16_t              width,
    const uint16_t              height,
    const int                   isp_ver,
    uint8_t*                     StepSize
) {
  uint32_t i;
  uint32_t square;

  uint32_t MaxNumOfPixel = 0U;
  uint32_t hist_grid_items, hist_grid_w;
  //TRACE( CAMERIC_ISP_HIST_DRV_INFO, "%s: (enter)\n", __FUNCTION__ );
  if (isp_ver == 0) {
      hist_grid_items = 25;
      hist_grid_w = 5;
  } else {
      hist_grid_items = 81;
      hist_grid_w = 9;
  }
  for (i = 0; i < hist_grid_items; i++) {
    MaxNumOfPixel += weights[i];
  }
  MaxNumOfPixel = MaxNumOfPixel * \
                  (((uint32_t)height / hist_grid_w) * ((uint32_t)width)
                   / hist_grid_w);

  switch (mode) {
    case CAMERIC_ISP_HIST_MODE_RGB_COMBINED: {
      if (isp_ver > 0)
          square = ((3 * MaxNumOfPixel) / 0x003FFFFF + 1);
      else
          square = ((3 * MaxNumOfPixel) / 0x000FFFFF + 1);
      break;
    }

    case CAMERIC_ISP_HIST_MODE_R:
    case CAMERIC_ISP_HIST_MODE_G:
    case CAMERIC_ISP_HIST_MODE_B:
    case CAMERIC_ISP_HIST_MODE_Y: {
      if (isp_ver > 0)
          square = (MaxNumOfPixel / 0x003FFFFF + 1);
      else
          square = (MaxNumOfPixel / 0x000FFFFF + 1);
      break;
    }

    default: {
      ALOGE("%s: Invalid histogram mode (%d) selected\n", __func__, mode);
      return (RET_OUTOFRANGE);
    }
  }

  /* avoid fsqrt */
  for (i = 3; i < 127; i++) {
    if (square <= (i * i)) {
      *StepSize = (uint8_t)i;
      break;
    }
  }

  //ALOGE("%s: (exit) StepSize=%d\n", __func__, *StepSize);

  return (RET_SUCCESS);
}

void cam_ia10_isp_map_hstw_9x9_to_5x5
(
    uint8_t* histw_9x9_in,
    uint8_t* histw_5x5_out
) {
    // 9x9->5x5 can be split to two step:
    // 1. 9x9 -> 9x5
    //    do the horizon map firstly, and each element in first line can be
    //    calculated like this:
    //      w00_5x5 = w00 * 1.0 + w01 * 0.8 + w02 * 0.0
    //      w01_5x5 = w01 * 0.2 + w02 * 1.0 + w03 * 0.6
    //      w02_5x5 = w03 * 0.4 + w04 * 1.0 + w05 * 0.4
    //      w03_5x5 = w05 * 0.6 + w06 * 1.0 + w07 * 0.2
    //      w04_5x5 = w07 * 0.8 + w08 * 1.0
    //    so does other horizon lines.
    // 2. 9x5 -> 5x5
    //    do the vertical map, map fomular is the same as above
    int i, j;
    // isp v12 weights limit is 0x3f
    float max_val_9x9 = 1.8*0x3f*2;
    // isp v10 weights limit is 0x1f
    float max_val_5x5 = 0x1f;
    float weights[5][3] = {
        {1.0 , 0.8 , 0.0},
        {0.2 , 1.0 , 0.6},
        {0.4 , 1.0 , 0.4},
        {0.6 , 1.0 , 0.2},
        {0.8 , 1.0 , 0.0}
    };
    float histw_9x5[45];
    // 9x9 -> 9x5
    for (j = 0; j < 9; j++) {
        for (i = 0; i < 5; i++) {
           int base_h = i*1.8;
           histw_9x5[5*j+i] =
               histw_9x9_in[9*j+base_h]   * weights[i][0] +
               histw_9x9_in[9*j+base_h+1] * weights[i][1] +
               ((i == 4) ? 0 : histw_9x9_in[9*j+base_h+2] * weights[i][2]);
        }
    }
    // 9x5 -> 5x5
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 5; i++) {
           int base_v = j*1.8;
           histw_5x5_out[5*j+i] = ceilf(
               (histw_9x5[base_v*5+i]     * weights[j][0] +
               histw_9x5[(base_v+1)*5+i] * weights[j][1] +
               ((j==4) ? 0 : histw_9x5[(base_v+2)*5+i] * weights[j][2])) *
               max_val_5x5 / max_val_9x9);
        }
    }
}

RESULT cam_ia10_isp_hst_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_hst_cfg_s* hst_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    int      isp_ver,
    CamerIcIspHistConfig_t* hst_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(hst_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    hst_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    int ind = 0;
    uint8_t step_size = 0;
    ISP_CHECK_NULL(hst_cfg);
    hst_result->enabled = BOOL_TRUE;
    hst_result->mode = (CamerIcIspHistMode_t)(hst_cfg->mode);
    hst_result->Window.hOffset = hst_cfg->win.left_hoff;
    hst_result->Window.vOffset = hst_cfg->win.top_voff;
    /* notice: driver will translate to grid (means divided 5)*/
    hst_result->Window.width = hst_cfg->win.right_width;
    hst_result->Window.height = hst_cfg->win.bottom_height;
    if (isp_ver > 0)
        cam_ia10_isp_map_hstw_9x9_to_5x5(hst_cfg->weight, hst_result->Weights);
    else
        for (; ind < CAMERIC_ISP_HIST_GRID_ITEMS; ind++)
          hst_result->Weights[ind] = hst_cfg->weight[ind];
    cam_ia10_isp_hst_update_stepSize(hst_result->mode,
                                     hst_result->Weights,
                                     hst_cfg->win.right_width,
                                     hst_cfg->win.bottom_height,
                                     isp_ver,
                                     &step_size);
    hst_result->StepSize = step_size;
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by aec algorithm
    int ind = 0;
    hst_result->enabled = BOOL_TRUE;
    hst_result->mode = CAMERIC_ISP_HIST_MODE_RGB_COMBINED;
    memset(&hst_result->Window, 0, sizeof(CamerIcWindow_t));
    hst_result->StepSize = 0;
    for (; ind < CAMERIC_ISP_HIST_GRID_ITEMS; ind++)
      hst_result->Weights[ind] = 0x10;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_lsc_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_lsc_cfg_s* lsc_cfg,
    int width,
    int height,
    int isp_ver,
    CamerIcLscConfig_t* lsc_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(lsc_result);
  ISP_CHECK_NULL(lsc_result->lsc_result);
  ISP_CHECK_NULL(lsc_result->lsc_seg_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    lsc_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(lsc_cfg);
    int ind = 0;
    memcpy(lsc_result->lsc_result->LscMatrix[ind++].uCoeff, lsc_cfg->LscRDataTbl,
           sizeof(uint16_t) * 17 * 17);
    memcpy(lsc_result->lsc_result->LscMatrix[ind++].uCoeff, lsc_cfg->LscGBDataTbl,
           sizeof(uint16_t) * 17 * 17);
    memcpy(lsc_result->lsc_result->LscMatrix[ind++].uCoeff, lsc_cfg->LscGRDataTbl,
           sizeof(uint16_t) * 17 * 17);
    memcpy(lsc_result->lsc_result->LscMatrix[ind++].uCoeff, lsc_cfg->LscBDataTbl,
           sizeof(uint16_t) * 17 * 17);

    memcpy(lsc_result->lsc_seg_result->LscXGradTbl, lsc_cfg->LscXGradTbl,
           sizeof(uint16_t) * 8);
    memcpy(lsc_result->lsc_seg_result->LscXSizeTbl, lsc_cfg->LscXSizeTbl,
           sizeof(uint16_t) * 8);
    memcpy(lsc_result->lsc_seg_result->LscYGradTbl, lsc_cfg->LscYGradTbl,
           sizeof(uint16_t) * 8);
    memcpy(lsc_result->lsc_seg_result->LscYSizeTbl, lsc_cfg->LscYSizeTbl,
           sizeof(uint16_t) * 8);
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by awb algorithm
    lsc_result->enabled = BOOL_FALSE;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_awb_gain_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_awb_gain_cfg_s* awb_gain_cfg,
    CameraIcAwbGainConfig_t* awb_gain_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(awb_gain_result);
  ISP_CHECK_NULL(awb_gain_result->awb_gain_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    awb_gain_result->enabled = BOOL_FALSE;
#if 0 //driver will do this
    //controlled by awb algorithm
    awb_gain_result->awb_gain_result->Red   = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->GreenR  = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->GreenB  = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->Blue    = UtlFloatToFix_U0208(1.0f);
#endif
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(awb_gain_cfg);
    awb_gain_result->enabled = BOOL_TRUE;
    awb_gain_result->awb_gain_result->Red   = UtlFloatToFix_U0208((float)(awb_gain_cfg->gain_red));
    awb_gain_result->awb_gain_result->GreenR  = UtlFloatToFix_U0208((float)(awb_gain_cfg->gain_green_b));
    awb_gain_result->awb_gain_result->GreenB  = UtlFloatToFix_U0208((float)(awb_gain_cfg->gain_green_r));
    awb_gain_result->awb_gain_result->Blue    = UtlFloatToFix_U0208((float)(awb_gain_cfg->gain_blue));
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by awb algorithm
    awb_gain_result->enabled = BOOL_FALSE;
#if 0 //driver will do this
    awb_gain_result->enabled = BOOL_TRUE;
    awb_gain_result->awb_gain_result->Red   = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->GreenR  = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->GreenB  = UtlFloatToFix_U0208(1.0f);
    awb_gain_result->awb_gain_result->Blue    = UtlFloatToFix_U0208(1.0f);
#endif
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_ctk_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_ctk_cfg_s* ctk_cfg,
    CameraIcCtkConfig_t* ctk_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(ctk_result);
  ISP_CHECK_NULL(ctk_result->ctk_matrix_result);
  ISP_CHECK_NULL(ctk_result->ctk_offset_result);
  if ((enable_mode == HAL_ISP_ACTIVE_FALSE)  ||
      (enable_mode == HAL_ISP_ACTIVE_DEFAULT)) {
    //controlled by awb algorithm
    //int ind = 0;
    ctk_result->enabled = BOOL_FALSE;
#if 0 // driver will do this
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(1.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(1.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(0.0f);
    ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407(1.0f);

    ctk_result->ctk_offset_result->Blue = UtlFloatToFix_S1200(0.0f);
    ctk_result->ctk_offset_result->Green = UtlFloatToFix_S1200(0.0f);
    ctk_result->ctk_offset_result->Red = UtlFloatToFix_S1200(0.0f);
#endif
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(ctk_cfg);
    int ind = 0;
    ctk_result->enabled = BOOL_TRUE;
    if (ctk_cfg->update_mask & HAL_ISP_CTK_UPDATE_CC_MATRIX) {
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff0));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff1));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff2));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff3));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff4));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff5));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff6));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff7));
      ctk_result->ctk_matrix_result->Coeff[ind++] = UtlFloatToFix_S0407((float)(ctk_cfg->coeff8));
    }

    if (ctk_cfg->update_mask & HAL_ISP_CTK_UPDATE_CC_OFFSET) {
      ctk_result->ctk_offset_result->Blue = UtlFloatToFix_S1200((float)(ctk_cfg->ct_offset_b));
      ctk_result->ctk_offset_result->Green = UtlFloatToFix_S1200((float)(ctk_cfg->ct_offset_g));
      ctk_result->ctk_offset_result->Red = UtlFloatToFix_S1200((float)(ctk_cfg->ct_offset_r));
    }
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_awb_meas_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_awb_meas_cfg_s* awb_meas_cfg,
    CamerIcAwbMeasConfig_t* awb_meas_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(awb_meas_result);
  ISP_CHECK_NULL(awb_meas_result->awb_meas_mode_result);
  ISP_CHECK_NULL(awb_meas_result->awb_meas_result);
  ISP_CHECK_NULL(awb_meas_result->awb_win);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    awb_meas_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(awb_meas_cfg);
    awb_meas_result->enabled = BOOL_TRUE;
    *(awb_meas_result->awb_meas_mode_result) =
        (CamerIcIspAwbMeasuringMode_t)(awb_meas_cfg->mode);
    awb_meas_result->awb_meas_result->MaxCSum =
        awb_meas_cfg->MaxCSum;
    awb_meas_result->awb_meas_result->MaxY =
        awb_meas_cfg->MaxY;
    awb_meas_result->awb_meas_result->MinC =
        awb_meas_cfg->MinC;
    awb_meas_result->awb_meas_result->MinY_MaxG =
        awb_meas_cfg->MinY_MaxG;
    awb_meas_result->awb_meas_result->RefCr_MaxR =
        awb_meas_cfg->RefCr_MaxR ;
    awb_meas_result->awb_meas_result->RefCb_MaxB =
        awb_meas_cfg->RefCb_MaxB ;
    awb_meas_result->awb_win->h_offs =
        awb_meas_cfg->win.left_hoff;
    awb_meas_result->awb_win->v_offs =
        awb_meas_cfg->win.top_voff;
    awb_meas_result->awb_win->h_size =
        awb_meas_cfg->win.right_width;
    awb_meas_result->awb_win->v_size =
        awb_meas_cfg->win.bottom_height;
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by awb algorithm
    awb_meas_result->enabled = BOOL_FALSE;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_aec_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_aec_cfg_s* aec_cfg,
    CameraIcAecConfig_t*  aec_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(aec_result);
  ISP_CHECK_NULL(aec_result->aec_meas_mode);
  ISP_CHECK_NULL(aec_result->meas_win);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    aec_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(aec_cfg);
    aec_result->enabled = BOOL_TRUE;
    *(aec_result->aec_meas_mode) =
        aec_cfg->mode;
    aec_result->meas_win->h_offs =
        aec_cfg->win.left_hoff;
    aec_result->meas_win->v_offs =
        aec_cfg->win.top_voff;
    /* notice: driver will translate to grid (means divided 5)*/
    aec_result->meas_win->v_size =
        aec_cfg->win.bottom_height ;
    aec_result->meas_win->h_size =
        aec_cfg->win.right_width;
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by aec algorithm
    aec_result->enabled = BOOL_TRUE;
    *(aec_result->aec_meas_mode) = 1;
    memset(aec_result->meas_win, 0, sizeof(struct Cam_Win));
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}


RESULT cam_ia10_isp_bdm_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_bdm_cfg_s*   bdm_cfg,
    CameraIcBdmConfig_t* bdm_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(bdm_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    bdm_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(bdm_cfg);
    bdm_result->enabled = BOOL_TRUE;
    bdm_result->demosaic_th = bdm_cfg->demosaic_th;
	//LOGD("%s: oyyf bdm th %d!", __func__, bdm_cfg->demosaic_th);
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    //controlled by aec algorithm
    bdm_result->enabled = BOOL_TRUE;
    bdm_result->demosaic_th = 4;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

/*used by gloabl mode*/
static uint32_t cam_ia_wdr_def_global_y[CAMERIC_WDR_CURVE_SIZE] = {
  0x0000, 0x00a2, 0x00a2, 0x016a,
  0x01e6, 0x029c, 0x02e6, 0x0368,
  0x03d9, 0x049b, 0x049b, 0x058d,
  0x0619, 0x069a, 0x0712, 0x0783,
  0x07ed, 0x0852, 0x090e, 0x0967,
  0x0a0f, 0x0a5f, 0x0af9, 0x0af9,
  0x0af9, 0x0c9a, 0x0d1e, 0x0d69,
  0x0e21, 0x0e9f, 0x0ef4, 0x0f74,
  0x1000,

};

/*used by block mode*/
static uint16_t cam_ia_wdr_def_block_y[CAMERIC_WDR_CURVE_SIZE] = {
  0x0000, 0x011c, 0x011c, 0x02d8,
  0x0375, 0x0478, 0x054f, 0x0609,
  0x06b0, 0x07d6, 0x07d6, 0x09b9,
  0x0b49, 0x0ca5, 0x0ddc, 0x0ef8,
  0x0fff, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000,
  0x0000,
};

//each value means 2^(val+3)
static uint8_t  cam_ia_wdr_def_segment[CAMERIC_WDR_CURVE_SIZE - 1] = {
  0x0, 0x1, 0x1, 0x2, 0x3, 0x2, 0x3, 0x3, //0x33232110
  0x4, 0x3, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, //0x44444434
  0x4, 0x5, 0x4, 0x5, 0x4, 0x5, 0x5, 0x5, //0x55545454
  0x5, 0x5, 0x4, 0x5, 0x4, 0x3, 0x3, 0x2, //0x23345455
};


RESULT cam_ia10_isp_wdr_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_wdr_cfg_s* wdr_cfg,
    CameraIcWdrConfig_t* wdr_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(wdr_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    wdr_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(wdr_cfg);
    unsigned int index = 0;
    wdr_result->enabled = BOOL_TRUE;
    wdr_result->mode = (CameraIcWdrMode_t)(wdr_cfg->mode);
    wdr_result->wdr_bavg_clip = wdr_cfg->wdr_bavg_clip;
    wdr_result->wdr_bestlight = wdr_cfg->wdr_bestlight;
    wdr_result->wdr_epsilon = wdr_cfg->wdr_epsilon;
    wdr_result->wdr_flt_sel = wdr_cfg->wdr_flt_sel ? 1 : 0;
    wdr_result->wdr_gain_max_clip_enable =
        wdr_cfg->wdr_gain_max_clip_enable ? 1 : 0;
    wdr_result->wdr_gain_max_value = wdr_cfg->wdr_gain_max_value;
    wdr_result->wdr_gain_off1 = wdr_cfg->wdr_gain_off1;
    wdr_result->wdr_lvl_en = wdr_cfg->wdr_lvl_en;
    wdr_result->wdr_noiseratio = wdr_cfg->wdr_noiseratio;
    wdr_result->wdr_nonl_mode1 = wdr_cfg->wdr_nonl_mode1 ? 1 : 0;
    wdr_result->wdr_nonl_open = wdr_cfg->wdr_nonl_open ? 1 : 0;
    wdr_result->wdr_nonl_segm = wdr_cfg->wdr_nonl_segm ? 1 : 0;
    wdr_result->wdr_pym_cc = wdr_cfg->wdr_pym_cc;
    wdr_result->wdr_coe0 = wdr_cfg->wdr_coe0;
    wdr_result->wdr_coe1 = wdr_cfg->wdr_coe1;
    wdr_result->wdr_coe2 = wdr_cfg->wdr_coe2;
    wdr_result->wdr_coe_off = wdr_cfg->wdr_coe_off;
    for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE - 1); index++) {
      int j = 10;
      for (j = 10; j >= 4 ; j--)
        if (wdr_cfg->wdr_dx[index] >= (2 << j))
          break;
      wdr_result->segment[index] = j - 3;
    }

    if (wdr_result->mode == CAMERIC_WDR_MODE_BLOCK)
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_block_y[index] = wdr_cfg->wdr_dy.wdr_block_dy[index];
    else
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_block_y[index] = wdr_cfg->wdr_dy.wdr_global_dy[index];
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    CamCalibWdrGlobal_t* pWdrGlobal;
    result = CamCalibDbGetWdrGlobal(hCamCalibDb, &pWdrGlobal);
    if (result != RET_SUCCESS) {
      LOGD("fail to get pWdrGlobal, ret: %d", result);
    }

    unsigned int index = 0;
    wdr_result->enabled = BOOL_TRUE;
    wdr_result->mode = CAMERIC_WDR_MODE_BLOCK;
    if (pWdrGlobal != NULL) {
      wdr_result->enabled = pWdrGlobal->Enabled ? BOOL_TRUE : BOOL_FALSE;
      wdr_result->mode = CameraIcWdrMode_t(pWdrGlobal->Mode);
    }

    for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE - 1); index++)
      wdr_result->segment[index] = cam_ia_wdr_def_segment[index];

    if (pWdrGlobal != NULL && pWdrGlobal->LocalCurve[CAMERIC_WDR_CURVE_SIZE / 2] > 0) {
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_block_y[index] = pWdrGlobal->LocalCurve[index];
    } else {
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_block_y[index] = cam_ia_wdr_def_block_y[index];
    }

    if (pWdrGlobal != NULL && pWdrGlobal->GlobalCurve[CAMERIC_WDR_CURVE_SIZE / 2] > 0) {
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_global_y[index] = pWdrGlobal->GlobalCurve[index];
    } else {
      for (index = 0; index < (CAMERIC_WDR_CURVE_SIZE); index++)
        wdr_result->wdr_global_y[index] = cam_ia_wdr_def_global_y[index];
    }

    //now value means as ISP register
    wdr_result->wdr_pym_cc = 0x3;
    wdr_result->wdr_epsilon = 0xc;
    wdr_result->wdr_lvl_en = 0xf;

    wdr_result->wdr_gain_max_clip_enable = 0x1;
    wdr_result->wdr_gain_max_value = 0x40;
    wdr_result->wdr_bavg_clip = 0x3;
    wdr_result->wdr_nonl_segm = 0x0;
    wdr_result->wdr_nonl_open = 0x1;
    wdr_result->wdr_nonl_mode1 = 0x0;
    wdr_result->wdr_flt_sel = 0x1;

    wdr_result->wdr_gain_off1 = 0x000000cd;

    wdr_result->wdr_bestlight = 0x0ccc;
    wdr_result->wdr_noiseratio = 0x00ee;
    wdr_result->wdr_coe0 = 0x00000036;
    wdr_result->wdr_coe1 = 0x000000b7;
    wdr_result->wdr_coe2 = 0x00000012;
    wdr_result->wdr_coe_off = 0x0;

    if (pWdrGlobal != NULL) {
      wdr_result->wdr_pym_cc = pWdrGlobal->wdr_pym_cc;
      wdr_result->wdr_epsilon = pWdrGlobal->wdr_epsilon;
      wdr_result->wdr_lvl_en = pWdrGlobal->wdr_lvl_en;
      wdr_result->wdr_gain_max_clip_enable = pWdrGlobal->wdr_gain_max_clip_enable;
      wdr_result->wdr_gain_max_value = pWdrGlobal->wdr_gain_max_value;
      wdr_result->wdr_bavg_clip = pWdrGlobal->wdr_bavg_clip;
      wdr_result->wdr_nonl_segm = pWdrGlobal->wdr_nonl_segm;
      wdr_result->wdr_nonl_open = pWdrGlobal->wdr_nonl_open;
      wdr_result->wdr_nonl_mode1 = pWdrGlobal->wdr_nonl_mode1;

      wdr_result->wdr_flt_sel = pWdrGlobal->wdr_flt_sel;
      wdr_result->wdr_gain_off1 = pWdrGlobal->wdr_gain_off1;
      wdr_result->wdr_bestlight = pWdrGlobal->wdr_bestlight;
      wdr_result->wdr_noiseratio = pWdrGlobal->wdr_noiseratio;
      wdr_result->wdr_coe0 = pWdrGlobal->wdr_coe0;
      wdr_result->wdr_coe1 = pWdrGlobal->wdr_coe1;
      wdr_result->wdr_coe2 = pWdrGlobal->wdr_coe2;
      wdr_result->wdr_coe_off = pWdrGlobal->wdr_coe_off;
    }
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}


RESULT cam_ia10_isp_dpf_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpf_cfg_s* dpf_cfg,
    CameraIcDpfConfig_t* dpf_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(dpf_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    dpf_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(dpf_cfg);
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    dpf_result->enabled = BOOL_FALSE;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}


RESULT cam_ia10_isp_dpf_strength_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpf_strength_cfg_s* dpf_streng_cfg,
    CameraIcDpfStrengthConfig_t* dpf_streng_result
) {
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(dpf_streng_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    dpf_streng_result->enabled = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
    ISP_CHECK_NULL(dpf_streng_cfg);
    dpf_streng_result->enabled = BOOL_TRUE;
    dpf_streng_result->b = (uint8_t)UtlFloatToFix_U0800(dpf_streng_cfg->b);
    dpf_streng_result->g = (uint8_t)UtlFloatToFix_U0800(dpf_streng_cfg->g);
    dpf_streng_result->r = (uint8_t)UtlFloatToFix_U0800(dpf_streng_cfg->r);
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
    dpf_streng_result->enabled = BOOL_FALSE;
  } else {
    ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  return result;
}

RESULT cam_ia10_isp_demosaicLp_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_demosaiclp_cfg_s* demosaicLP_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcRKDemosaicLP_t* demosaicLP_result
) 
{
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(demosaicLP_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    demosaicLP_result->lp_en = BOOL_FALSE;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
	ISP_CHECK_NULL(demosaicLP_cfg);
	demosaicLP_result->lp_en = demosaicLP_cfg->lp_en;
	demosaicLP_result->use_old_lp = demosaicLP_cfg->use_old_lp;
	demosaicLP_result->rb_filter_en = demosaicLP_cfg->rb_filter_en;
	demosaicLP_result->hp_filter_en = demosaicLP_cfg->hp_filter_en;
	demosaicLP_result->edge_level_sel = demosaicLP_cfg->edge_level_sel;
	demosaicLP_result->flat_level_sel = demosaicLP_cfg->flat_level_sel;
	demosaicLP_result->pattern_level_sel = demosaicLP_cfg->pattern_level_sel;
	demosaicLP_result->th_var_en = demosaicLP_cfg->th_var_en;
	demosaicLP_result->th_csc_en = demosaicLP_cfg->th_csc_en;
	demosaicLP_result->th_diff_en = demosaicLP_cfg->th_diff_en;
	demosaicLP_result->th_grad_en = demosaicLP_cfg->th_grad_en;
	demosaicLP_result->thgrad_r_fct = demosaicLP_cfg->thgrad_r_fct;
	demosaicLP_result->thdiff_r_fct = demosaicLP_cfg->thdiff_r_fct;
	demosaicLP_result->thvar_r_fct = demosaicLP_cfg->thvar_r_fct;
	demosaicLP_result->thgrad_b_fct = demosaicLP_cfg->thgrad_b_fct;
	demosaicLP_result->thdiff_b_fct = demosaicLP_cfg->thdiff_b_fct;
	demosaicLP_result->thvar_b_fct = demosaicLP_cfg->thvar_b_fct;
	demosaicLP_result->similarity_th = demosaicLP_cfg->similarity_th;
	demosaicLP_result->th_var = demosaicLP_cfg->th_var;
	demosaicLP_result->th_csc = demosaicLP_cfg->th_csc;
	demosaicLP_result->th_diff = demosaicLP_cfg->th_diff;
	demosaicLP_result->th_grad = demosaicLP_cfg->th_grad;

	memcpy(demosaicLP_result->lu_divided, demosaicLP_cfg->lu_divided, sizeof(demosaicLP_result->lu_divided));
	memcpy(demosaicLP_result->thgrad_divided, demosaicLP_cfg->thgrad_divided, sizeof(demosaicLP_result->thgrad_divided));
	memcpy(demosaicLP_result->thcsc_divided, demosaicLP_cfg->thcsc_divided, sizeof(demosaicLP_result->thcsc_divided));
	memcpy(demosaicLP_result->thdiff_divided, demosaicLP_cfg->thdiff_divided, sizeof(demosaicLP_result->thdiff_divided));
	memcpy(demosaicLP_result->thvar_divided, demosaicLP_cfg->thvar_divided, sizeof(demosaicLP_result->thvar_divided));
	
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
	CamDpfProfile_t* pDpfProfile_t = NULL;
	CamResolutionName_t ResName;   
	result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, drv_width, drv_height, &ResName);
	if (RET_SUCCESS != result) {
	  LOGD("%s: resolution (%dx%d) not found in database\n", drv_width, drv_height);
	}else{
	  result = CamCalibDbGetDpfProfileByResolution(hCamCalibDb, ResName, &pDpfProfile_t);
	  if (RET_SUCCESS != result) {
	    LOGD("%s: get dpf fail (%dx%d) (%s) not found in database\n", drv_width, drv_height, ResName);
	  } 
	}

	int no_filter;
	CamFilterProfile_t* pFilterProfile = NULL;
	CamDemosaicLpProfile_t *pDemosaicLpConf = NULL;
	enum LIGHT_MODE LightMode = (enum LIGHT_MODE)demosaicLP_cfg->light_mode;
	
	result = CamCalibDbGetNoOfFilterProfile(hCamCalibDb, pDpfProfile_t, &no_filter);
	
	if(result != RET_SUCCESS){
		LOGD("fail to get no FilterProfile, ret: %d", result);
	}else{
		if(no_filter > 0 && pDpfProfile_t != NULL){
			if(LightMode <= LIGHT_MODE_MIN ||  LightMode >= LIGHT_MODE_MAX || LightMode > no_filter)
				LightMode = LIGHT_MODE_DAY;
		    result = CamCalibDbGetFilterProfileByIdx(hCamCalibDb, pDpfProfile_t, LightMode, &pFilterProfile);
		    if (result != RET_SUCCESS) {
		      LOGD("fail to get filter profile fail ret: %d", result);
		    }else{
				pDemosaicLpConf = &pFilterProfile->DemosaicLpConf;
		    }
		}
	}

	if(pDemosaicLpConf != NULL){
		demosaicLP_result->lp_en = pDemosaicLpConf->lp_en;
		demosaicLP_result->use_old_lp = pDemosaicLpConf->use_old_lp;
		demosaicLP_result->rb_filter_en = pDemosaicLpConf->rb_filter_en;
		demosaicLP_result->hp_filter_en = pDemosaicLpConf->hp_filter_en;
		demosaicLP_result->edge_level_sel = pDemosaicLpConf->edge_level_sel[0];
		demosaicLP_result->flat_level_sel = pDemosaicLpConf->flat_level_sel[0];
		demosaicLP_result->pattern_level_sel = pDemosaicLpConf->pattern_level_sel[0];
		demosaicLP_result->th_var_en = pDemosaicLpConf->th_var_en;
		demosaicLP_result->th_csc_en = pDemosaicLpConf->th_csc_en;
		demosaicLP_result->th_diff_en = pDemosaicLpConf->th_diff_en;
		demosaicLP_result->th_grad_en = pDemosaicLpConf->th_grad_en;
		demosaicLP_result->thgrad_r_fct = pDemosaicLpConf->thgrad_r_fct[0];
		demosaicLP_result->thdiff_r_fct = pDemosaicLpConf->thdiff_r_fct[0];
		demosaicLP_result->thvar_r_fct = pDemosaicLpConf->thvar_r_fct[0];
		demosaicLP_result->thgrad_b_fct = pDemosaicLpConf->thgrad_b_fct[0];
		demosaicLP_result->thdiff_b_fct = pDemosaicLpConf->thdiff_b_fct[0];
		demosaicLP_result->thvar_b_fct = pDemosaicLpConf->thvar_b_fct[0];
		demosaicLP_result->similarity_th = pDemosaicLpConf->similarity_th[0];
		demosaicLP_result->th_var = pDemosaicLpConf->th_var[0];
		demosaicLP_result->th_csc = pDemosaicLpConf->th_csc[0];
		demosaicLP_result->th_diff = pDemosaicLpConf->th_diff[0];
		demosaicLP_result->th_grad = pDemosaicLpConf->th_grad[0];
		
		demosaicLP_result->thgrad_divided[0] = pDemosaicLpConf->thH_divided0[0];
		demosaicLP_result->thgrad_divided[1] = pDemosaicLpConf->thH_divided1[0];
		demosaicLP_result->thgrad_divided[2] = pDemosaicLpConf->thH_divided2[0];
		demosaicLP_result->thgrad_divided[3] = pDemosaicLpConf->thH_divided3[0];
		demosaicLP_result->thgrad_divided[4] = pDemosaicLpConf->thH_divided4[0];

		demosaicLP_result->thcsc_divided[0] = pDemosaicLpConf->thCSC_divided0[0];
		demosaicLP_result->thcsc_divided[1] = pDemosaicLpConf->thCSC_divided1[0];
		demosaicLP_result->thcsc_divided[2] = pDemosaicLpConf->thCSC_divided2[0];
		demosaicLP_result->thcsc_divided[3] = pDemosaicLpConf->thCSC_divided3[0];
		demosaicLP_result->thcsc_divided[4] = pDemosaicLpConf->thCSC_divided4[0];

		demosaicLP_result->thvar_divided[0] = pDemosaicLpConf->varTh_divided0[0];
		demosaicLP_result->thvar_divided[1] = pDemosaicLpConf->varTh_divided1[0];
		demosaicLP_result->thvar_divided[2] = pDemosaicLpConf->varTh_divided2[0];
		demosaicLP_result->thvar_divided[3] = pDemosaicLpConf->varTh_divided3[0];
		demosaicLP_result->thvar_divided[4] = pDemosaicLpConf->varTh_divided4[0];

		demosaicLP_result->thdiff_divided[0] = pDemosaicLpConf->diff_divided0[0];
		demosaicLP_result->thdiff_divided[1] = pDemosaicLpConf->diff_divided1[0];
		demosaicLP_result->thdiff_divided[2] = pDemosaicLpConf->diff_divided2[0];
		demosaicLP_result->thdiff_divided[3] = pDemosaicLpConf->diff_divided3[0];
		demosaicLP_result->thdiff_divided[4] = pDemosaicLpConf->diff_divided4[0];
		
		demosaicLP_result->lu_divided[0] = pDemosaicLpConf->lu_divided[0];
		demosaicLP_result->lu_divided[1] = pDemosaicLpConf->lu_divided[1];
		demosaicLP_result->lu_divided[2] = pDemosaicLpConf->lu_divided[2];
		demosaicLP_result->lu_divided[3] = pDemosaicLpConf->lu_divided[3];
	}else{
		demosaicLP_result->lp_en = 0;
		demosaicLP_result->use_old_lp = 0;
		demosaicLP_result->rb_filter_en = 0;
		demosaicLP_result->hp_filter_en = 0;
	}	
	
  } else{
	ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }

  return result;
}

RESULT cam_ia10_isp_rkIEsharp_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_RKIEsharp_cfg_s* rkIEsharp_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcRKIeSharpConfig_t* rkIEsharp_result
) 
{
  RESULT result = RET_SUCCESS;
  ISP_CHECK_NULL(rkIEsharp_result);
  if (enable_mode == HAL_ISP_ACTIVE_FALSE) {
    rkIEsharp_result->iesharpen_en = 0;
  } else if (enable_mode == HAL_ISP_ACTIVE_SETTING) {
	ISP_CHECK_NULL(rkIEsharp_cfg);
	rkIEsharp_result->iesharpen_en = rkIEsharp_cfg->iesharpen_en;
	rkIEsharp_result->coring_thr = rkIEsharp_cfg->coring_thr;
	rkIEsharp_result->full_range = rkIEsharp_cfg->full_range;
	rkIEsharp_result->switch_avg = rkIEsharp_cfg->switch_avg;
	memcpy(rkIEsharp_result->maxnumber, rkIEsharp_cfg->maxnumber, sizeof(rkIEsharp_result->maxnumber));
	memcpy(rkIEsharp_result->minnumber, rkIEsharp_cfg->minnumber, sizeof(rkIEsharp_result->minnumber));
	memcpy(rkIEsharp_result->yavg_thr, rkIEsharp_cfg->yavg_thr, sizeof(rkIEsharp_result->yavg_thr));
	memcpy(rkIEsharp_result->delta1, rkIEsharp_cfg->delta1, sizeof(rkIEsharp_result->delta1));
	memcpy(rkIEsharp_result->delta2, rkIEsharp_cfg->delta2, sizeof(rkIEsharp_result->delta2));
	memcpy(rkIEsharp_result->gauss_flat_coe, rkIEsharp_cfg->gauss_flat_coe, sizeof(rkIEsharp_result->gauss_flat_coe));
	memcpy(rkIEsharp_result->gauss_noise_coe, rkIEsharp_cfg->gauss_noise_coe, sizeof(rkIEsharp_result->gauss_noise_coe));
	memcpy(rkIEsharp_result->gauss_other_coe, rkIEsharp_cfg->gauss_other_coe, sizeof(rkIEsharp_result->gauss_other_coe));
	memcpy(rkIEsharp_result->uv_gauss_flat_coe, rkIEsharp_cfg->uv_gauss_flat_coe, sizeof(rkIEsharp_result->uv_gauss_flat_coe));
	memcpy(rkIEsharp_result->uv_gauss_noise_coe, rkIEsharp_cfg->uv_gauss_noise_coe, sizeof(rkIEsharp_result->uv_gauss_noise_coe));
	memcpy(rkIEsharp_result->uv_gauss_other_coe, rkIEsharp_cfg->uv_gauss_other_coe, sizeof(rkIEsharp_result->uv_gauss_other_coe));
	memcpy(rkIEsharp_result->p_grad, rkIEsharp_cfg->p_grad, sizeof(rkIEsharp_result->p_grad));
	memcpy(rkIEsharp_result->sharp_factor, rkIEsharp_cfg->sharp_factor, sizeof(rkIEsharp_result->sharp_factor));
	memcpy(rkIEsharp_result->line1_filter_coe, rkIEsharp_cfg->line1_filter_coe, sizeof(rkIEsharp_result->line1_filter_coe));
	memcpy(rkIEsharp_result->line2_filter_coe, rkIEsharp_cfg->line2_filter_coe, sizeof(rkIEsharp_result->line2_filter_coe));
	memcpy(rkIEsharp_result->line3_filter_coe, rkIEsharp_cfg->line3_filter_coe, sizeof(rkIEsharp_result->line3_filter_coe));
	
  } else if (enable_mode == HAL_ISP_ACTIVE_DEFAULT) {
	CamResolutionName_t res_name = {0};
    CamIesharpenProfile_t*        pIEsharpProfile = NULL;
    //get configs from xml
	
    result = CamCalibDbGetResolutionNameByWidthHeight(hCamCalibDb, drv_width, drv_height,  &res_name);
    if (RET_SUCCESS != result) {
      ALOGE("%s: resolution (%dx%d) not found in database\n", __FUNCTION__, drv_width, drv_height);
      return (result);
    }
    // get dpf-profile from calibration database
    CamIesharpenProfile_t *pIesharpenProfile = NULL;
    result = CamCalibDbGetRKsharpenProfileByResolution(hCamCalibDb, res_name, &pIesharpenProfile);
    if (result != RET_SUCCESS) {
      ALOGE("%s: Getting rk ie sharp profile for resolution %s from calibration database failed (%d)\n",
            __FUNCTION__, res_name, result);
    }

	if(pIesharpenProfile != NULL){
		rkIEsharp_result->iesharpen_en = pIesharpenProfile->iesharpen_en;
		rkIEsharp_result->coring_thr = pIesharpenProfile->coring_thr;
		rkIEsharp_result->full_range = pIesharpenProfile->full_range;
		rkIEsharp_result->switch_avg = pIesharpenProfile->switch_avg;
		memcpy(rkIEsharp_result->maxnumber, pIesharpenProfile->pmaxnumber, sizeof(rkIEsharp_result->maxnumber));
		memcpy(rkIEsharp_result->minnumber, pIesharpenProfile->pminnumber, sizeof(rkIEsharp_result->minnumber));
		memcpy(rkIEsharp_result->yavg_thr, pIesharpenProfile->yavg_thr, sizeof(rkIEsharp_result->yavg_thr));
		memcpy(rkIEsharp_result->delta1, pIesharpenProfile->P_delta1, sizeof(rkIEsharp_result->delta1));
		memcpy(rkIEsharp_result->delta2, pIesharpenProfile->P_delta2, sizeof(rkIEsharp_result->delta2));
		memcpy(rkIEsharp_result->gauss_flat_coe, pIesharpenProfile->gauss_flat_coe, sizeof(rkIEsharp_result->gauss_flat_coe));
		memcpy(rkIEsharp_result->gauss_noise_coe, pIesharpenProfile->gauss_noise_coe, sizeof(rkIEsharp_result->gauss_noise_coe));
		memcpy(rkIEsharp_result->gauss_other_coe, pIesharpenProfile->gauss_other_coe, sizeof(rkIEsharp_result->gauss_other_coe));
		memcpy(rkIEsharp_result->uv_gauss_flat_coe, pIesharpenProfile->uv_gauss_flat_coe, sizeof(rkIEsharp_result->uv_gauss_flat_coe));
		memcpy(rkIEsharp_result->uv_gauss_noise_coe, pIesharpenProfile->uv_gauss_noise_coe, sizeof(rkIEsharp_result->uv_gauss_noise_coe));
		memcpy(rkIEsharp_result->uv_gauss_other_coe, pIesharpenProfile->uv_gauss_other_coe, sizeof(rkIEsharp_result->uv_gauss_other_coe));
		memcpy(rkIEsharp_result->p_grad, pIesharpenProfile->lgridconf.p_grad, sizeof(rkIEsharp_result->p_grad));
		memcpy(rkIEsharp_result->sharp_factor, pIesharpenProfile->lgridconf.sharp_factor, sizeof(rkIEsharp_result->sharp_factor));
		memcpy(rkIEsharp_result->line1_filter_coe, pIesharpenProfile->lgridconf.line1_filter_coe, sizeof(rkIEsharp_result->line1_filter_coe));
		memcpy(rkIEsharp_result->line2_filter_coe, pIesharpenProfile->lgridconf.line2_filter_coe, sizeof(rkIEsharp_result->line2_filter_coe));
		memcpy(rkIEsharp_result->line3_filter_coe, pIesharpenProfile->lgridconf.line3_filter_coe, sizeof(rkIEsharp_result->line3_filter_coe));
	}else{
		rkIEsharp_result->iesharpen_en = 0;
	}
  } else{
	ALOGE("%s:error enable mode %d!", __func__, enable_mode);
    result = RET_FAILURE;
  }
  
  return result;
}

/*

RESULT cam_ia10_isp_afc_config
(
struct HAL_ISP_wdr_cfg_s* wdr_cfg,
cifisp_wdr_config* wdr_result
)
{
}
*/






