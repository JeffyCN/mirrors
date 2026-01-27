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

#include "algos/abayertnrV30/rk_aiq_types_abayertnr_algo_v30.h"
#include "algos/acnrV31/rk_aiq_types_acnr_algo_v31.h"
#include "algos/adebayer/rk_aiq_types_adebayer_algo.h"
#include "algos/aldc/rk_aiq_types_aldc_algo.h"
#include "algos/argbir/rk_aiq_types_argbir_algo.h"
#include "algos/asharpV34/rk_aiq_types_asharp_algo_v34.h"
#include "algos/aynrV24/rk_aiq_types_aynr_algo_v24.h"
#include "algos/ayuvmeV1/rk_aiq_types_ayuvme_algo_v1.h"
#include "common/rk_aiq_comm.h"
#include "rk_aiq_mems_sensor.h"
//#include "isp/rk_aiq_stats_awb39.h"
#ifndef ISP39_LDCH_BIC_NUM
#define ISP39_LDCH_BIC_NUM 36
#endif

typedef AdebayerHwConfigV3_t rk_aiq_isp_debayer_v39_t;

typedef rk_aiq_af_algo_meas_v33_t rk_aiq_isp_af_v33_t;
typedef RK_Bayertnr_Fix_V30_t rk_aiq_isp_tnr_v39_t;
typedef RK_YUVME_Fix_V1_t rk_aiq_isp_yuvme_v39_t;
typedef RK_CNR_Fix_V31_t rk_aiq_isp_cnr_v39_t;
#ifndef USE_NEWSTRUCT
typedef RkAiqAdrcProcResult_t rk_aiq_isp_drc_v39_t;
typedef RkAiqAdehazeProcResult_t rk_aiq_isp_dehaze_v39_t;
#endif
typedef RkAiqArgbirProcResult_t rk_aiq_isp_rgbir_v39_t;
typedef RK_YNR_Fix_V24_t rk_aiq_isp_ynr_v39_t;

typedef RK_SHARP_Fix_V34_t rk_aiq_isp_sharp_v39_t;

typedef struct rk_aiq_isp_ldc_s {
    bool ldch_en;
    bool ldcv_en;
    rkaiq_ldc_hw_param_t cfg;
} rk_aiq_isp_ldc_t;




#endif
