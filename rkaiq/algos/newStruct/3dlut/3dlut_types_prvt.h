/*
* lut3d_types_prvt.h

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

#ifndef __RKAIQ_TYPES_3DLUT_ALGO_PRVT_H__
#define __RKAIQ_TYPES_3DLUT_ALGO_PRVT_H__

/**
 * @file lut3d_types_prvt.h
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
 * @defgroup Dm Auto debayer
 * @{
 *
 */
#include "include/3dlut_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"
#include "algo_types_priv.h"
#include "newStruct/algo_common.h"

#define RK_DEBAYER_ISO_STEP_MAX 13
#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

#define ILLU_ESTM_RES_LEN (15)
#define INVALID_ILLU_IDX (0xff)

typedef struct {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    lut3d_calib_attrib_t* lut3d_attrib;
    //lut3d_meshGain_t lut0;

    uint8_t pre_illu_idx;
    float pre_alpha;
    uint32_t pre_lutSum[3];

    lut3d_meshGain_t *calib_matrix;

    bool damp_converged;
    lut3d_meshGain_t undamped_matrix;
    lut3d_meshGain_t damped_matrix;

    bool is_calib_update;
} Lut3dContext_t;

XCamReturn A3dlut_prepare(RkAiqAlgoCom* params);
XCamReturn A3dlut_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t *swinfo);

#endif//__RKAIQ_TYPES_3DLUT_ALGO_PRVT_H__
