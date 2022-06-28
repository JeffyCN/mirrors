/*
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

#ifndef _ISP20_HW_SIMULATOR_H_
#define _ISP20_HW_SIMULATOR_H_

#include "awb/rk_aiq_types_awb_algo_int.h"
#include "ae/rk_aiq_types_ae_algo.h"
#include "af/rk_aiq_af_hw_v200.h"
#include "adehaze/rk_aiq_types_adehaze_algo_prvt.h"
#include "anr/rk_aiq_types_anr_algo_int.h"
#include "asharp/rk_aiq_types_asharp_algo_int.h"
#include "adebayer/rk_aiq_types_algo_adebayer.h"
#include "atmo/rk_aiq_types_atmo_algo_int.h"
#include "atmo/rk_aiq_types_atmo_algo.h"
#include "amerge/rk_aiq_types_amerge_algo_int.h"
#include "amerge/rk_aiq_types_amerge_algo.h"
#include "agamma/rk_aiq_types_agamma_algo_prvt.h"
#include "aorb/rk_aiq_orb_hw.h"
#include "adpcc/rk_aiq_types_adpcc_algo_int.h"
#include "ablc/rk_aiq_types_ablc_algo_int.h"
#include "alsc/rk_aiq_types_alsc_algo.h"
#include "accm/rk_aiq_types_accm_algo.h"
#include "agic/rk_aiq_types_algo_agic.h"
#include "rk_aiq_luma.h"
#include "a3dlut/rk_aiq_types_a3dlut_algo_int.h"
typedef struct rk_sim_isp_v200_luma_s {
    //luma
    bool valid_luma;
    isp_luma_stat_t image_luma_result;
} rk_sim_isp_v200_luma_t;

typedef struct rk_sim_isp_v200_stats_s {
    //awb
    bool valid_awb;
    rk_aiq_awb_stat_res_v200_t awb;
    rk_aiq_awb_stat_res_v201_t awb_v201;
    //ae
    bool valid_ae;
    RKAiqAecStats_t ae;
    //af
    bool valid_af;
    rawaf_isp_af_stat_t af;
    //anr html params
    int iso;
    //orb
    bool valid_orb;
    sim_orb_stat_t orb;
    //atmo
    unsigned short  rawWidth;               // rawÍ¼¿í
    unsigned short  rawHeight;
    bool valid_atmo;
    rkisp_atmo_stats_t atmo;
} rk_sim_isp_v200_stats_t;

typedef struct rk_sim_isp_v200_params_s {
    float awb_smooth_factor;
    rk_aiq_wb_gain_t awb_gain_algo;
    rk_aiq_awb_stat_cfg_v200_t awb_hw0_para;
    rk_aiq_awb_stat_cfg_v201_t awb_hw1_para;
    //ae
    RkAiqAecHwConfig_t ae_hw_config;
    //anr
    ANRProcResult_t rkaiq_anr_proc_res;
    AsharpProcResult_t rkaiq_asharp_proc_res;
    //adhaz
    rk_aiq_dehaze_cfg_t adhaz_config;
    //agamma
    AgammaProcRes_t agamma_config;
    //amerge
    RkAiqAmergeProcResult_t amerge_proc_res;
    //atmo
    RkAiqAtmoProcResult_t atmo_proc_res;
    //adpcc
    AdpccProcResult_t   dpcc;
    //adebayer
    AdebayerConfig_t adebayer_config;
    //ablc
    AblcProc_t blc;
    //agic
    AgicConfig_t agic_config;
    rk_aiq_lsc_cfg_t lscHwConf;
    rk_aiq_ccm_cfg_t ccmHwConf;

    rk_aiq_lut3d_cfg_t lut3d_hw_conf;
    unsigned short  rawWidth;               // rawÍ¼¿í
    unsigned short  rawHeight;
} rk_sim_isp_v200_params_t;

#endif

