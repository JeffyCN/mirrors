/*
 * Copyright (C) 2017 Intel Corporation.
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __RKAIQ_H
#define __RKAIQ_H

#include <xcam_std.h>
#ifdef ANDROID_VERSION_ABOVE_8_X
#include <CameraMetadata.h>
using ::android::hardware::camera::common::V1_0::helper::CameraMetadata;
#else
#include <camera/CameraMetadata.h>
#endif
#include <base/xcam_params.h>

using namespace android;
namespace XCam {
typedef struct _AiqInputParams AiqInputParams; 

/**
 * \struct AeControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AeControls {
    uint8_t aeMode;                 /**< AE_MODE */
    uint8_t aeLock;                 /**< AE_LOCK */
    uint8_t aePreCaptureTrigger;    /**< PRECAPTURE_TRIGGER */
    uint8_t aeAntibanding;          /**< AE_ANTIBANDING */
    int32_t evCompensation;         /**< AE_EXPOSURE_COMPENSATION */
    int32_t aeTargetFpsRange[2];    /**< AE_TARGET_FPS_RANGE */
} AeControls;

/**
 * \struct AwbControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AwbControls {
    uint8_t awbMode;                        /**< AWB_MODE */
    uint8_t awbLock;                        /**< AWB_LOCK */
    uint8_t colorCorrectionMode;            /**< COLOR_CORRECTION_MODE */
    uint8_t colorCorrectionAberrationMode;  /**< COLOR_CORRECTION_ABERRATION_MODE */
} AwbControls;

/**
 * \struct AfControls
 *
 * Control Modes saved and passed back to control unit after reading
 *
 */
typedef struct _AfControls {
    uint8_t afMode; /**< AF_MODE */
    uint8_t afTrigger; /**< AF_TRIGGER */
} AfControls;

typedef struct _AAAControls {
    uint8_t controlMode;    /**< MODE */
    AeControls  ae;
    AwbControls awb;
    AfControls  af;
} AAAControls;

typedef struct _AeInputParams {
    XCamAeParam                         aeParams;
    int32_t                             aeRegion[5];
    AiqInputParams*                     aiqParams;
    /* AeInputParams()                     { CLEAR(*this); } */
} AeInputParams;

typedef struct _AwbInputParams {
    XCamAwbParam                        awbParams;
    int32_t                             awbRegion[5];
    AiqInputParams*                     aiqParams;
    /* AwbInputParams()                    { CLEAR(*this); } */
} AwbInputParams;

typedef struct _AfInputParams {
    XCamAfParam                         afParams;
    int32_t                             afRegion[5];
    AiqInputParams*                     aiqParams;
    /* AfInputParams()                    { CLEAR(*this); } */
} AfInputParams;

typedef enum {
   AIQ_FRAME_USECASE_PREVIEW,
   AIQ_FRAME_USECASE_STILL_CAPTURE,
   AIQ_FRAME_USECASE_VIDEO_RECORDING,
} AiqFrameUseCase;

typedef struct _BlsInputParams {
    uint8_t    updateFlag;
    uint8_t    enable;
    uint8_t    mode;
    uint8_t    winEnable;
    struct Win_s{
       uint16_t hOffset;
       uint16_t vOffset;
       uint16_t width;
       uint16_t height;
    } win[2];
    uint8_t     samples;
    struct FixedVal_s{
        uint16_t    fixed_r;
        uint16_t    fixed_gr;
        uint16_t    fixed_gb;
        uint16_t    fixed_b;
    }fixedVal;
} __attribute__((packed)) BlsInputParams;

typedef struct _LscInputParams {
    #define HAL_ISP_LSC_NAME_LEN         25
    #define HAL_ISP_LSC_SIZE_TBL_LEN     8
    #define HAL_ISP_LSC_MATRIX_COLOR_NUM 4
    #define HAL_ISP_LSC_MATRIX_TBL_LEN   289
    uint8_t   updateFlag;
    uint8_t   on;
    int8_t    LscName[HAL_ISP_LSC_NAME_LEN];
    uint16_t  LscSectors;
    uint16_t  LscNo;
    uint16_t  LscXo;
    uint16_t  LscYo;
    uint16_t  LscXSizeTbl[HAL_ISP_LSC_SIZE_TBL_LEN];
    uint16_t  LscYSizeTbl[HAL_ISP_LSC_SIZE_TBL_LEN];
    uint16_t  LscMatrix[HAL_ISP_LSC_MATRIX_COLOR_NUM][HAL_ISP_LSC_MATRIX_TBL_LEN];
}__attribute__((packed)) LscInputParams;

typedef struct _CcmInputParams {
    uint8_t   updateFlag;
    int8_t   on;
    int8_t   name[20];
    float    matrix[9];
    float    offsets[3];
} __attribute__((packed)) CcmInputParams;

typedef struct _AwbToolInputParams {
  #define HAL_ISP_ILL_NAME_LEN    20
  uint8_t   updateFlag;
  uint8_t on;
  float r_gain;
  float gr_gain;
  float gb_gain;
  float b_gain;
  uint8_t lock_ill;
  char ill_name[HAL_ISP_ILL_NAME_LEN];
}__attribute__((packed)) AwbToolInputParams;

typedef struct _AwbWhitePointParams {
#define HAL_ISP_AWBFADE2PARM_LEN  6
  uint8_t updateFlag;
  uint16_t win_h_offs;
  uint16_t win_v_offs;
  uint16_t win_width;
  uint16_t win_height;
  uint8_t awb_mode;

  float afFade[HAL_ISP_AWBFADE2PARM_LEN];
  #if 1
    float afmaxCSum_br[HAL_ISP_AWBFADE2PARM_LEN];
    float afmaxCSum_sr[HAL_ISP_AWBFADE2PARM_LEN];
    float afminC_br[HAL_ISP_AWBFADE2PARM_LEN];
    float afMaxY_br[HAL_ISP_AWBFADE2PARM_LEN];
    float afMinY_br[HAL_ISP_AWBFADE2PARM_LEN];
    float afminC_sr[HAL_ISP_AWBFADE2PARM_LEN];
    float afMaxY_sr[HAL_ISP_AWBFADE2PARM_LEN];
    float afMinY_sr[HAL_ISP_AWBFADE2PARM_LEN];
  #else
  	float afCbMinRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afCrMinRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afMaxCSumRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afCbMinRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
	float afCrMinRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
	float afMaxCSumRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
	float afMinCRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afMinCRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
	float afMaxYRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afMaxYRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
	float afMinYMaxGRegionMax[HAL_ISP_AWBFADE2PARM_LEN];
	float afMinYMaxGRegionMin[HAL_ISP_AWBFADE2PARM_LEN];
  #endif
  float afRefCb[HAL_ISP_AWBFADE2PARM_LEN];
  float afRefCr[HAL_ISP_AWBFADE2PARM_LEN];
  float fRgProjIndoorMin;
  float fRgProjOutdoorMin;
  float fRgProjMax;
  float fRgProjMaxSky;
  float fRgProjALimit;
  float fRgProjAWeight;
  float fRgProjYellowLimitEnable;
  float fRgProjYellowLimit;
  float fRgProjIllToCwfEnable;
  float fRgProjIllToCwf;
  float fRgProjIllToCwfWeight;
  float fRegionSize;
  float fRegionSizeInc;
  float fRegionSizeDec;

  uint32_t cnt;
  uint8_t mean_y;
  uint8_t mean_cb;
  uint8_t mean_cr;
  uint16_t mean_r;
  uint16_t mean_b;
  uint16_t mean_g;
}__attribute__((packed)) AwbWhitePointParams;

typedef struct _AwbCurveInputParams {
#define HAL_ISP_CURVE_NAME_LEN    20
#define HAL_ISP_AWBCLIPPARM_LEN   16
  uint8_t updateFlag;
  float f_N0_Rg;
  float f_N0_Bg;
  float f_d;
  float Kfactor;

  float afRg1[HAL_ISP_AWBCLIPPARM_LEN];
  float afMaxDist1[HAL_ISP_AWBCLIPPARM_LEN];
  float afRg2[HAL_ISP_AWBCLIPPARM_LEN];
  float afMaxDist2[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalFade1[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalGainDistance1[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalFade2[HAL_ISP_AWBCLIPPARM_LEN];
  float afGlobalGainDistance2[HAL_ISP_AWBCLIPPARM_LEN];
}__attribute__((packed)) AwbCurveInputParams;

typedef struct _AwbRefGainInputParams {
  uint8_t updateFlag;
  int8_t ill_name[HAL_ISP_ILL_NAME_LEN];
  float refRGain;
  float refGrGain;
  float refGbGain;
  float refBGain;
}__attribute__((packed)) AwbRefGainInputParams;

typedef struct _GocInputParams {
  uint8_t updateFlag;
  uint8_t on;
  int8_t scene_name[20];
  uint8_t wdr_status;
  uint8_t cfg_mode;
  uint16_t gamma_y[34];
}__attribute__((packed)) GocInputParams;

typedef struct _CprocInputParams {
  uint8_t updateFlag;
  uint8_t on;
  uint8_t mode;
  float cproc_contrast;
  float cproc_hue;
  float cproc_saturation;
  int8_t cproc_brightness;
}__attribute__((packed)) CprocInputParams;

typedef struct _AdpfInputParams {
  uint8_t updateFlag;
  int8_t  dpf_name[20];
  uint8_t dpf_enable;
  uint8_t nll_segment;
  uint16_t nll_coeff[17];
  uint16_t sigma_green;
  uint16_t sigma_redblue;
  float gradient;
  float offset;
  float fRed;
  float fGreenR;
  float fGreenB;
  float fBlue;
}__attribute__((packed)) AdpfInputParams;

struct ISP_FLT_Level_Conf_s {
  uint8_t grn_stage1;
  uint8_t chr_h_mode;
  uint8_t chr_v_mode;
  uint32_t thresh_bl0;
  uint32_t thresh_bl1;
  uint32_t thresh_sh0;
  uint32_t thresh_sh1;
  uint32_t fac_sh1;
  uint32_t fac_sh0;
  uint32_t fac_mid;
  uint32_t fac_bl0;
  uint32_t fac_bl1;
}__attribute__((packed));

typedef struct _FltInputParams {
  uint8_t updateFlag;
  int8_t  filter_name[20];
  uint8_t scene_mode;
  uint8_t filter_enable;
  uint8_t denoise_gain[5];
  uint8_t denoise_level[5];
  uint8_t sharp_gain[5];
  uint8_t sharp_level[5];
  uint8_t level_conf_enable;
  uint8_t level;
  struct ISP_FLT_Level_Conf_s level_conf;
}__attribute__((packed)) FltInputParams;

typedef struct _RestartInputParams {
  uint8_t updateFlag;
  uint8_t on;
}__attribute__((packed)) RestartInputParams;

typedef struct _AiqInputParams {
    _AiqInputParams &operator=(const _AiqInputParams &other);
    unsigned int    reqId;
    AeInputParams   aeInputParams;
    AwbInputParams  awbInputParams;
    // for tuning tool START
    BlsInputParams  blsInputParams;
    LscInputParams  lscInputParams;
    CcmInputParams  ccmInputParams;
    AwbToolInputParams  awbToolInputParams;
    AwbWhitePointParams awbWpInputParams;
    AwbCurveInputParams awbCurveInputParams;
    AwbRefGainInputParams awbRefGainInputParams;
    GocInputParams   gocInputParams;
    CprocInputParams cprocInputParams;
    AdpfInputParams  adpfInputParams;
    FltInputParams   fltInputParams;
    RestartInputParams restartInputParams;
    bool                  tuningFlag;
    // for tuning tool END
    AfInputParams   afInputParams;
    AAAControls     aaaControls;
    CameraMetadata  settings;
    CameraMetadata* staticMeta;
    int sensorOutputWidth;
    int sensorOutputHeight;
    AiqFrameUseCase frameUseCase;
    uint8_t         stillCapSyncCmd;
    void init();
    _AiqInputParams();
    ~_AiqInputParams() {}
} AiqInputParams;

};
#endif //__RKAIQ_H
