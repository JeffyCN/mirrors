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
#include "common/list.h"


RKAIQ_BEGIN_DECLARE
#define RKAIQ_ACCM_ILLU_VOTE 0
#define CCM_CURVE_DOT_NUM 17

typedef struct ccm_3ares_info_s{
    float sensorGain;
    float awbGain[2];
} ccm_3ares_info_t;

typedef struct {
    unsigned int name_len;
    int matused_len;
    unsigned int matused_str_len[CCM_PROFILES_NUM_MAX];
} accmCof_initlen_info_t;

typedef struct {
    float scl;
    uint8_t tbl_idx;
} yalp_tbl_info_t;

typedef struct {
    unsigned int name_len;
    unsigned int illu_len;
} matrixall_initlen_info_t;
typedef struct {
    int accmCof_initlen;
    accmCof_initlen_info_t accmCof_initlen_info[CCM_ILLUMINATION_MAX];
    int matrixall_initlen;
    matrixall_initlen_info_t matrixall_initlen_info[CCM_ILLUMINATION_MAX*CCM_PROFILES_NUM_MAX];
} ccm_calib_initlen_info_t;

typedef struct accm_rest_s {
    float fSaturation;
#if RKAIQ_ACCM_ILLU_VOTE
    struct list_head dominateIlluList;//to record domain illuminant
#endif
    struct list_head problist;
    int dominateIlluProfileIdx;
    char CcmProf1Name[25];
    char CcmProf2Name[25];
    float undampedCcmMatrix[9];
    float undampedCcOffset[3];
    float fScale;
    float color_inhibition_level;
    float color_saturation_level;
    ccm_3ares_info_t res3a_info;
    int illuNum;
    ccm_calib_initlen_info_t ccm_calib_initlen_info;
    yalp_tbl_info_t yalp_tbl_info;
} accm_rest_t;

typedef struct illu_node_s {
    list_head node;       /**< for adding to a list */
    unsigned int value;
} illu_node_t;

typedef struct prob_node_s {
    list_head node;       /**< for adding to a list */
    unsigned int value;
    float prob;
} prob_node_t;

typedef struct accm_context_s {
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* ccm_v1;
    rk_aiq_ccm_cfg_t ccmHwConf;
    rk_aiq_ccm_attrib_t mCurAtt;
#endif
#if RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* ccm_v2;
    rk_aiq_ccm_cfg_v2_t ccmHwConf_v2;
    rk_aiq_ccm_v2_attrib_t mCurAttV2;
#endif
#if RKAIQ_HAVE_CCM_V3
    CalibDbV2_Ccm_Para_V39_t* ccm_v3;
    rk_aiq_ccm_cfg_v2_t ccmHwConf_v2;
    rk_aiq_ccm_v3_attrib_t mCurAttV3;
#endif
    const CalibDbV2_Ccm_Matrix_Para_t *pCcmMatrixAll[CCM_ILLUMINATION_MAX][CCM_PROFILES_NUM_MAX];// reorder para //to do, change to pointer
    accm_sw_info_t accmSwInfo;
    accm_rest_t accmRest;
    unsigned int count;
    //ctrl & api
    uint8_t invarMode; // 0- mode change 1- mode unchange
    bool updateAtt;
    bool update;
    bool calib_update;
    bool isReCal_;
} accm_context_t ;

typedef accm_context_t* accm_handle_t ;

typedef struct _RkAiqAlgoContext {
    accm_handle_t accm_para;
} RkAiqAlgoContext;

RKAIQ_END_DECLARE

#endif

