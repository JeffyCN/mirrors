/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef _RK_AIQ_TYPES_V39_H_
#define _RK_AIQ_TYPES_V39_H_

#include "abayertnrV30/rk_aiq_types_abayertnr_algo_v30.h"
#include "acnrV31/rk_aiq_types_acnr_algo_v31.h"
#include "adebayer/rk_aiq_types_adebayer_algo.h"
#include "argbir/rk_aiq_types_argbir_algo.h"
#include "asharpV34/rk_aiq_types_asharp_algo_v34.h"
#include "aynrV24/rk_aiq_types_aynr_algo_v24.h"
#include "ayuvmeV1/rk_aiq_types_ayuvme_algo_v1.h"
#include "isp/rk_aiq_isp_trans10.h"
#include "rk_aiq_api_types_trans.h"
#if USE_NEWSTRUCT
#include "isp/rk_aiq_isp_drc40.h"
#endif
#include "rk_aiq_comm.h"
#include "rk_aiq_mems_sensor.h"

#ifndef ISP32_LDCH_BIC_NUM
#define ISP32_LDCH_BIC_NUM      36
#endif

typedef AdebayerHwConfigV3_t rk_aiq_isp_debayer_v39_t;

typedef rk_aiq_af_algo_meas_v33_t rk_aiq_isp_af_v33_t;
typedef RK_Bayertnr_Fix_V30_t rk_aiq_isp_tnr_v39_t;
typedef RK_YUVME_Fix_V1_t rk_aiq_isp_yuvme_v39_t;
typedef RK_CNR_Fix_V31_t rk_aiq_isp_cnr_v39_t;
#ifndef USE_NEWSTRUCT
typedef RkAiqAdrcProcResult_t rk_aiq_isp_drc_v39_t;
#endif
typedef RkAiqArgbirProcResult_t rk_aiq_isp_rgbir_v39_t;
typedef RK_YNR_Fix_V24_t rk_aiq_isp_ynr_v39_t;

typedef RK_SHARP_Fix_V34_t rk_aiq_isp_sharp_v39_t;

typedef RkAiqAdehazeProcResult_t rk_aiq_isp_dehaze_v39_t;

#if USE_NEWSTRUCT
typedef struct rk_aiq_isp_drc_v39_s {
    drc_param_t drc_param;
    trans_api_attrib_t trans_attr;
} rk_aiq_isp_drc_v39_t;
#endif

#endif
