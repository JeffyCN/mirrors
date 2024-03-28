/*
 * rk_aiq_a3dlut_algo_prvt.h
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

#ifndef _RK_AIQ_A3DLUT_ALGO_PRVT_H_
#define _RK_AIQ_A3DLUT_ALGO_PRVT_H_

#include "RkAiqCalibDbTypes.h"
#include "a3dlut/rk_aiq_types_a3dlut_algo_int.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "RkAiqCalibDbTypesV2.h"
#include "common/list.h"



RKAIQ_BEGIN_DECLARE

#define RKAIQ_A3DLUT_ILLU_VOTE 0

typedef struct lut3d_3ares_info_s{
    float sensorGain;
    bool gain_stable;
} lut3d_3ares_info_t;

typedef struct alut3d_rest_info_s {
    float alpha;
#if RKAIQ_A3DLUT_ILLU_VOTE
    struct list_head dominateIdxList;//to record domain lutIdx
#endif
    int dominateIdx;
    const CalibDbV2_Lut3D_LutPara_t *pLutProfile;
    lut3d_3ares_info_t res3a_info;
    int lutSum_last[3];
    int lutSum[3];
} alut3d_rest_info_t;

typedef struct idx_node_s {
    list_head        p_next;       /**< for adding to a list */
    unsigned int value;
} idx_node_t;

typedef struct alut3d_context_s {
    const CalibDb_Lut3d_t *calib_lut3d;
    const CalibDbV2_Lut3D_Para_V2_t  *calibV2_lut3d;
    rk_aiq_lut3d_cfg_t lut3d_hw_conf;
    rk_aiq_lut3d_hw_tbl_t lut0;
    // info
    alut3d_rest_info_t restinfo;
    alut3d_sw_info_t swinfo;
    //ctrl & api
    rk_aiq_lut3d_attrib_t mCurAtt;
    //rk_aiq_lut3d_attrib_t mNewAtt;
    bool updateAtt;
    bool update;
    bool calib_update;
    int prepare_type;
} alut3d_context_t ;

typedef alut3d_context_t* alut3d_handle_t ;

typedef struct _RkAiqAlgoContext {
    alut3d_handle_t a3dlut_para;
} RkAiqAlgoContext;


RKAIQ_END_DECLARE

#endif

