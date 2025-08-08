/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPES_V3X_H_
#define _RK_AIQ_TYPES_V3X_H_

/*
 * @file
 * @brief this file describle the isp v3 structs, will be included by rk_aiq_types.h
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "algos/a3dlut/rk_aiq_types_a3dlut_algo.h"
#include "algos/abayer2dnr2/rk_aiq_types_abayer2dnr_algo_v2.h"
#include "algos/abayertnr2/rk_aiq_types_abayertnr_algo_v2.h"
#include "algos/ablc/rk_aiq_types_ablc_algo.h"
#include "algos/ablc/rk_aiq_types_ablc_algo_int.h"
#include "algos/acac/rk_aiq_types_acac_algo.h"
#include "algos/accm/rk_aiq_types_accm_algo.h"
#include "algos/acnr/rk_aiq_types_acnr_algo_v1.h"
#include "algos/acnr2/rk_aiq_types_acnr_algo_v2.h"
#include "algos/acp/rk_aiq_types_acp_algo.h"
#include "algos/adebayer/rk_aiq_types_adebayer_algo.h"
#include "algos/adegamma/rk_aiq_types_adegamma_algo_int.h"
#include "algos/adehaze/rk_aiq_types_adehaze_algo.h"
#include "algos/adehaze/rk_aiq_types_adehaze_algo_int.h"
#include "algos/adpcc/rk_aiq_types_adpcc_algo.h"
#include "algos/adpcc/rk_aiq_types_adpcc_algo_int.h"
#include "algos/adrc/rk_aiq_types_adrc_algo.h"
#include "algos/adrc/rk_aiq_types_adrc_algo_int.h"
#include "algos/ae/rk_aiq_types_ae_algo.h"
#include "algos/af/rk_aiq_types_af_algo.h"
#include "algos/afec/rk_aiq_types_afec_algo.h"
#include "algos/again/rk_aiq_types_again_algo.h"
#include "algos/again2/rk_aiq_types_again_algo_v2.h"
#include "algos/agamma/rk_aiq_types_agamma_algo.h"
#include "algos/agamma/rk_aiq_types_agamma_algo_int.h"
#include "algos/agic/rk_aiq_types_algo_agic_int.h"
#include "algos/aie/rk_aiq_types_aie_algo_int.h"
#include "algos/aldch/rk_aiq_types_aldch_algo.h"
#include "algos/alsc/rk_aiq_types_alsc_algo.h"
#include "algos/amd/rk_aiq_types_amd_algo.h"
#include "algos/amerge/rk_aiq_types_amerge_algo.h"
#include "algos/amerge/rk_aiq_types_amerge_algo_int.h"
#include "algos/amfnr/rk_aiq_types_amfnr_algo_v1.h"
#include "algos/anr/rk_aiq_types_anr_algo.h"
#include "algos/anr/rk_aiq_types_anr_algo_int.h"
#include "algos/aorb/rk_aiq_types_orb_algo.h"
#include "algos/arawnr/rk_aiq_types_abayernr_algo_v1.h"
#include "algos/arawnr2/rk_aiq_types_abayernr_algo_v2.h"
#include "algos/asd/rk_aiq_types_asd_algo.h"
#include "algos/asharp/rk_aiq_types_asharp_algo.h"
#include "algos/asharp/rk_aiq_types_asharp_algo_int.h"
#include "algos/asharp3/rk_aiq_types_asharp_algo_v3.h"
#include "algos/asharp4/rk_aiq_types_asharp_algo_v4.h"
#include "algos/atmo/rk_aiq_types_atmo_algo.h"
#include "algos/atmo/rk_aiq_types_atmo_algo_int.h"
#include "algos/auvnr/rk_aiq_types_auvnr_algo_v1.h"
#include "algos/awb/rk_aiq_types_awb_algo.h"
#include "algos/aynr/rk_aiq_types_aynr_algo_v1.h"
#include "algos/aynr2/rk_aiq_types_aynr_algo_v2.h"
#include "algos/aynr3/rk_aiq_types_aynr_algo_v3.h"
#include "common/rk_aiq_comm.h"
#include "rk_aiq_mems_sensor.h"

//typedef struct rk_aiq_isp_blc_v3x_s {
//} rk_aiq_isp_blc_v3x_t;

//typedef rk_aiq_isp_blc_v21_t rk_aiq_isp_blc_v3x_t;

typedef AgammaProcRes_t rk_aiq_isp_goc_v3x_t;

typedef RkAiqAdrcProcResult_t rk_aiq_isp_drc_v3x_t;

typedef RkAiqAmergeProcResult_t rk_aiq_isp_merge_v3x_t;

typedef RkAiqAdehazeProcResult_t rk_aiq_isp_dehaze_v3x_t;


typedef RK_Bayer2dnr_Fix_V2_t rk_aiq_isp_baynr_v3x_t ;

typedef RK_Bayertnr_Fix_V2_t rk_aiq_isp_tnr_v3x_t ;

typedef RK_YNR_Fix_V3_t rk_aiq_isp_ynr_v3x_t ;

typedef RK_CNR_Fix_V2_t rk_aiq_isp_cnr_v3x_t ;

typedef RK_SHARP_Fix_V4_t rk_aiq_isp_sharp_v3x_t ;

typedef struct rk_aiq_isp_bay3d_v3x_s {
} rk_aiq_isp_bay3d_v3x_t;


typedef struct rk_aiq_isp_cac_v3x_s {
    bool enable;
    rkaiq_cac_v10_hw_param_t cfg[2];
} rk_aiq_isp_cac_v3x_t;

typedef RK_GAIN_Fix_V2_t rk_aiq_isp_gain_v3x_t;


typedef rk_aiq_awb_stat_cfg_v201_t rk_aiq_isp_awb_meas_cfg_v3x_t;
typedef rk_aiq_awb_stat_res_v201_t rk_aiq_isp_awb_stats_v3x_t;
typedef rk_aiq_awb_stat_res2_v30_t rk_aiq_isp_awb_stats2_v3x_t;

typedef rk_aiq_af_algo_stat_v30_t rk_aiq_isp_af_stats_v3x_t;
typedef rk_aiq_af_algo_meas_v30_t rk_aiq_isp_af_meas_v3x_t;

typedef struct rk_aiq_isp_dehaze_stats_v3x_s {
} rk_aiq_isp_dehaze_stats_v3x_t;

#endif
