#ifndef __CAM_IA10_ENGINE_ISP_MODULES_H__
#define __CAM_IA10_ENGINE_ISP_MODULES_H__
#include <ebase/types.h>
#include <common/return_codes.h>
#include <common/cam_types.h>
#include <cameric.h>
#include <cam_calibdb/cam_calibdb_api.h>
#include <cam_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

RESULT cam_ia10_isp_bls_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    CamCalibDbHandle_t        hCamCalibDb,
    uint16_t  width,
    uint16_t height,
    struct HAL_ISP_bls_cfg_s* bls_cfg,
    CamerIcIspBlsConfig_t* bls_result
);

RESULT cam_ia10_isp_dpcc_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpcc_cfg_s* dpcc_cfg,
    CamCalibDbHandle_t        hCamCalibDb,
    uint16_t  width,
    uint16_t height,
    CamerIcDpccConfig_t* dpcc_result
);


RESULT cam_ia10_isp_sdg_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_sdg_cfg_s* sdg_cfg,
    CamerIcIspDegammaCurve_t* sdg_result
);

RESULT cam_ia10_isp_flt_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_flt_cfg_s* flt_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcIspFltConfig_t* flt_result
);


RESULT cam_ia10_isp_goc_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_goc_cfg_s* goc_cfg,
    CamerIcIspGocConfig_t* goc_result,
    bool_t WDR_enable_mohde,
    int isp_ver
) ;

void cam_ia10_isp_goc_map_34_to_17
(
     uint16_t* goc_in,
     uint16_t* goc_out
);

RESULT cam_ia10_isp_cproc_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_cproc_cfg_s* cproc_cfg,
    CamerIcCprocConfig_t* cproc_result
);


RESULT cam_ia10_isp_ie_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_ie_cfg_s* ie_cfg,
    CamerIcIeConfig_t* ie_result
);


RESULT cam_ia10_isp_hst_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_hst_cfg_s* hst_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    int isp_ver,
    CamerIcIspHistConfig_t* hst_result
);

void cam_ia10_isp_map_hstw_9x9_to_5x5
(
    uint8_t* histw_9x9_in,
    uint8_t* histw_5x5_out
);

RESULT cam_ia10_isp_lsc_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_lsc_cfg_s* lsc_cfg,
    int width,
    int height,
    int isp_ver,
    CamerIcLscConfig_t* lsc_result
);


RESULT cam_ia10_isp_awb_gain_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_awb_gain_cfg_s* awb_gain_cfg,
    CameraIcAwbGainConfig_t* awb_gain_result
);


RESULT cam_ia10_isp_ctk_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_ctk_cfg_s* ctk_cfg,
    CameraIcCtkConfig_t* ctk_result
);

RESULT cam_ia10_isp_awb_meas_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_awb_meas_cfg_s* awb_meas_cfg,
    CamerIcAwbMeasConfig_t* awb_meas_result
);


RESULT cam_ia10_isp_aec_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_aec_cfg_s* aec_cfg,
    CameraIcAecConfig_t*  aec_result
);

RESULT cam_ia10_isp_bdm_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_bdm_cfg_s* bdm_cfg,
    CameraIcBdmConfig_t* bdm_result
);

RESULT cam_ia10_isp_hst_update_stepSize
(
    const CamerIcIspHistMode_t  mode,
    const CamerIcHistWeights_t  weights,
    const uint16_t              width,
    const uint16_t              height,
    int                         isp_ver,
    uint8_t*                     StepSize
);

RESULT cam_ia10_isp_wdr_config
(
    CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_wdr_cfg_s* wdr_cfg,
    CameraIcWdrConfig_t* wdr_result
);

RESULT cam_ia10_isp_dpf_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpf_cfg_s* dpf_cfg,
    CameraIcDpfConfig_t* dpf_result
);

RESULT cam_ia10_isp_dpf_strength_config
(
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_dpf_strength_cfg_s* dpf_streng_cfg,
    CameraIcDpfStrengthConfig_t* dpf_streng_result
);

RESULT cam_ia10_isp_demosaicLp_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_demosaiclp_cfg_s* demosaicLP_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcRKDemosaicLP_t* demosaicLP_result
) ;

RESULT cam_ia10_isp_rkIEsharp_config
(
	CamCalibDbHandle_t hCamCalibDb,
    enum HAL_ISP_ACTIVE_MODE enable_mode,
    struct HAL_ISP_RKIEsharp_cfg_s* rkIEsharp_cfg,
    uint16_t drv_width,
    uint16_t drv_height,
    CamerIcRKIeSharpConfig_t* rkIEsharp_result
) ;



#ifdef __cplusplus
}
#endif

#endif


