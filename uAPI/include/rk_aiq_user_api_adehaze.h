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

#ifndef _RK_AIQ_USER_API_ADEHAZE_H_
#define _RK_AIQ_USER_API_ADEHAZE_H_

#include "adehaze/rk_aiq_uapi_adehaze_int.h"

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;


//typedef enum rk_aiq_dehaze_op_mode_s {
//    RK_AIQ_DEHAZE_MODE_INVALID                     = 0,        /**< invalid mode */
//    RK_AIQ_DEHAZE_MODE_MANUAL                      = 1,        /**< run manual dehaze */
//    RK_AIQ_DEHAZE_MODE_AUTO                        = 2,        /**< run auto dehaze */
//    RK_AIQ_DEHAZE_MODE_OFF                         = 3,        /**< dehaze off, enhance follow IQ setting*/
//    RK_AIQ_DEHAZE_MODE_TOOL                        = 4,        /**< dehaze off, enhance follow IQ setting*/
//} rk_aiq_dehaze_op_mode_t;
/*
struct CalibDb_IIR_setting_s {
    float stab_fnum;
    float sigma;
    float wt_sigma;
    float air_sigma;
    float tmax_sigma;
};

struct CalibDb_Dehaze_Setting_s {
    char name[CALIBDB_MAX_MODE_NAME_LENGTH];
    float en;
    float iso[9];
    float dc_min_th[9];
    float dc_max_th[9];
    float yhist_th[9];
    float yblk_th[9];
    float dark_th[9];
    float bright_min[9];
    float bright_max[9];
    float wt_max[9];
    float air_min[9];
    float air_max[9];
    float tmax_base[9];
    float tmax_off[9];
    float tmax_max[9];
    float cfg_wt[9];
    float cfg_air[9];
    float cfg_tmax[9];
    float dc_thed[9];
    float dc_weitcur[9];
    float air_thed[9];
    float air_weitcur[9];
    struct CalibDb_IIR_setting_s IIR_setting;
};

struct CalibDb_Enhance_Setting_s {
    char name[CALIBDB_MAX_MODE_NAME_LENGTH];
    float en;
    float iso[9];
    float enhance_value[9];
};

struct CalibDb_Hist_setting_s {
    char name[CALIBDB_MAX_MODE_NAME_LENGTH];
    float en;
    float iso[9];
    unsigned char  hist_channel[9];
    unsigned char  hist_para_en[9];
    float hist_gratio[9];
    float hist_th_off[9];
    float hist_k[9];
    float hist_min[9];
    float hist_scale[9];
    float cfg_gratio[9];
};

typedef struct CalibDbDehaze_s {
    float en;
    float cfg_alpha_normal;
    float cfg_alpha_hdr;
    float cfg_alpha_night;
    struct CalibDb_Dehaze_Setting_s dehaze_setting[CALIBDB_MAX_MODE_NUM];
    struct CalibDb_Enhance_Setting_s enhance_setting[CALIBDB_MAX_MODE_NUM];
    struct CalibDb_Hist_setting_s hist_setting[CALIBDB_MAX_MODE_NUM];
} CalibDbDehaze_t;

typedef struct rk_adehaze_sw_s {
    bool byPass;
    rk_aiq_dehaze_op_mode_t mode;
    rk_aiq_dehaze_M_attrib_t stManual;
    CalibDbDehaze_t stAuto;
    rk_aiq_dehaze_enhance_t stEnhance;
    CalibDbDehaze_t  stTool;
} adehaze_sw_t;
*/

RKAIQ_BEGIN_DECLARE
/*
XCamReturn  rk_aiq_user_api_adehaze_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t attr);
XCamReturn  rk_aiq_user_api_adehaze_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t *attr);
*/
RKAIQ_END_DECLARE

#endif

