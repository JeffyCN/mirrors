/*
 *rk_aiq_types_accm_algo_prvt.h
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

#ifndef _RK_AIQ_TYPES_ACCM_ALGO_PRVT_H_
#define _RK_AIQ_TYPES_ACCM_ALGO_PRVT_H_
#include "accm/rk_aiq_types_accm_algo_int.h"
#include "xcam_log.h"
#include "xcam_common.h"
#include "list.h"


RKAIQ_BEGIN_DECLARE
#define CCM_CURVE_DOT_NUM 17

typedef struct ccm_3ares_info_s{
    float sensorGain;
    float awbGain[2];
    bool wbgain_stable;
    bool gain_stable;
} ccm_3ares_info_t;


typedef struct accm_rest_s {
    float fSaturation;
    List dominateIlluList;//to record domain illuminant
    List problist;
    int dominateIlluProfileIdx;
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t *pCcmProfile1;
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t *pCcmProfile2;
    float undampedCcmMatrix[9];
    float dampedCcmMatrix[9];
    float undampedCcOffset[3];
    float dampedCcOffset[3];
    float color_inhibition_level;
    float color_saturation_level;
    ccm_3ares_info_t res3a_info;
} accm_rest_t;

typedef struct illu_node_s {
    void*        p_next;       /**< for adding to a list */
    unsigned int value;
} illu_node_t;

typedef struct prob_node_s {
    void*        p_next;       /**< for adding to a list */
    unsigned int value;
    float prob;
} prob_node_t;


typedef struct accm_context_s {
    const CalibDbV2_Ccm_Para_V2_t *calibV2Ccm; // json para
    const CalibDbV2_Ccm_Ccm_Matrix_Para_t *pCcmMatrixAll[CCM_ILLUMINATION_MAX][CCM_PROFILES_NUM_MAX];// reorder para //to do, change to pointer
    accm_sw_info_t accmSwInfo;
    accm_rest_t accmRest;
    rk_aiq_ccm_cfg_t ccmHwConf; //hw para
    unsigned int count;
    //ctrl & api
    rk_aiq_ccm_attrib_t mCurAtt;
    rk_aiq_ccm_attrib_t mNewAtt;
    bool updateAtt;
    bool update;
    bool calib_update;
} accm_context_t ;

typedef accm_context_t* accm_handle_t ;

typedef struct _RkAiqAlgoContext {
    accm_handle_t accm_para;
} RkAiqAlgoContext;

RKAIQ_END_DECLARE

#endif

