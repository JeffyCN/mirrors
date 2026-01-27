/*
* demosaic_types_prvt.h

* for rockchip v2.0.0
*
*  Copyright (c) 2019 Rockchip Corporation
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
*
*/
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_HSV_ALGO_PRVT_H__
#define __RKAIQ_TYPES_HSV_ALGO_PRVT_H__

#include "include/hsv_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define ISP_PREDGAIN_DEFAULT  (1.0f)
#define FUNCTION_ENABLE (1)
#define FUNCTION_DISABLE (0)

#define HSV_1DLUT_NUM      65
#define HSV_2DLUT_COL      17
#define HSV_2DLUT_ROW      17
#define HSV_2DLUT_NUM      (HSV_2DLUT_ROW*HSV_2DLUT_COL)
#define INVALID_ILLU_IDX (0xff)

typedef struct {
    int16_t lut0[HSV_1DLUT_NUM];
    int16_t lut1[HSV_1DLUT_NUM];
    int16_t lut2[HSV_2DLUT_NUM];
#if RKAIQ_HAVE_HSV_V11
    int16_t lut2d1[HSV_2DLUT_NUM];
    int16_t lut2d2[HSV_2DLUT_NUM];
#endif
} hsv_meshGain_t;

typedef struct {
    hsv_calib_attrib_t* hsv_calib;

    int pre_gain;
    int pre_wbgain[2];
    uint8_t pre_illu_idx;
#if RKAIQ_HAVE_HSV_V10
    uint8_t pre_mode[3];
    int32_t pre_lutSum[3];
#elif RKAIQ_HAVE_HSV_V11
    uint8_t pre_mode[5];
    int32_t pre_lutSum[5];
#endif
    float pre_alpha;

    hsv_param_dyn_t* calib_lut;

    bool damp_converged;
    hsv_meshGain_t undamped_lut;
    hsv_meshGain_t damped_lut;

    bool is_calib_update;

    ahsv_satStrg_t strg;
    ahsv_hueOffset_t hoffset;
    ahsv_valOffset_t voffset;
    bool isReCal_;
} HsvContext_t;

XCamReturn Ahsv_prepare(RkAiqAlgoCom* params);
XCamReturn Ahsv_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t *swinfo);

XCAM_BEGIN_DECLARE

XCAM_END_DECLARE

#endif
