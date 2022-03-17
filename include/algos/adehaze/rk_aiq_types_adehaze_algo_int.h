/*
 *rk_aiq_types_adehaze_algo_int.h
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

#ifndef _RK_AIQ_TYPE_ADEHAZE_ALGO_INT_H_
#define _RK_AIQ_TYPE_ADEHAZE_ALGO_INT_H_
#include "RkAiqCalibDbTypes.h"
#include "adehaze_head.h"
#include "adehaze/rk_aiq_types_adehaze_algo.h"

typedef enum AdehazeVersion_e {
    ADEHAZE_ISP20      = 0,
    ADEHAZE_ISP21      = 1,
    ADEHAZE_ISP30      = 2,
    ADEHAZE_VERSION_MAX
} AdehazeVersion_t;

typedef enum dehaze_api_mode_s {
    DEHAZE_API_BYPASS                  = 0,        /**< api bypass */
    DEHAZE_API_MANUAL                  = 1,        /**< run manual mode*/
    DEHAZE_API_DEHAZE_AUTO             = 2,        /**< run auto dehaze */
    DEHAZE_API_DEHAZE_MANUAL           = 3,        /**< run manual dehaze */
    DEHAZE_API_DEHAZE_OFF              = 4,        /**< dehaze off, enhance follow IQ setting*/
    DEHAZE_API_ENHANCE_MANUAL          = 5,        /**< dehaze enhance Manual*/
    DEHAZE_API_ENHANCE_AUTO            = 6,        /**< dehaze enhance follow IQ setting*/
    DEHAZE_API_ENHANCE_OFF             = 7,        /**< dehaze enhance off, dehaze enable fellow IQ setting*/
} dehaze_api_mode_t;

typedef struct mDehazeDataV21_s {
    float dc_min_th;
    float dc_max_th;
    float yhist_th;
    float yblk_th;
    float dark_th;
    float bright_min;
    float bright_max;
    float wt_max;
    float air_min;
    float air_max;
    float tmax_base;
    float tmax_off;
    float tmax_max;
    float cfg_wt;
    float cfg_air;
    float cfg_tmax;
    float dc_weitcur;
    float bf_weight;
    float range_sigma;
    float space_sigma_pre;
    float space_sigma_cur;
} mDehazeDataV21_t;

typedef struct mDehaze_Setting_V21_s {
    bool en;
    bool air_lc_en;
    float stab_fnum;
    float sigma;
    float wt_sigma;
    float air_sigma;
    float tmax_sigma;
    float pre_wet;
    mDehazeDataV21_t DehazeData;
} mDehaze_Setting_V21_t;

typedef struct mEnhanceDataV21_s {
    float enhance_value;
    float enhance_chroma;
} mEnhanceDataV21_t;

typedef struct mEnhance_Setting_V21_s {
    bool en;
    float enhance_curve[CALIBDB_ADEHAZE_ENHANCE_CURVE_KNOTS_NUM];
    mEnhanceDataV21_t EnhanceData;
} mEnhance_Setting_V21_t;

typedef struct mHistDataV21_s {
    float hist_gratio;
    float hist_th_off;
    float hist_k;
    float hist_min;
    float hist_scale;
    float cfg_gratio;
} mHistDataV21_t;

typedef struct mHist_setting_V21_s {
    bool en;
    bool hist_para_en;
    mHistDataV21_t HistData;
} mHist_setting_V21_t;

typedef struct mDehazeAttr_s {
    bool Enable;
    float cfg_alpha;
    mDehaze_Setting_V21_t dehaze_setting;
    mEnhance_Setting_V21_t enhance_setting;
    mHist_setting_V21_t hist_setting;
} mDehazeAttr_t;

typedef struct DehazeManuAttr_s {
    int level;//0~100
} DehazeManuAttr_t;

typedef struct EnhanceManuAttr_s {
    int level;//0~100
} EnhanceManuAttr_t;

typedef struct adehaze_sw_V2_s {
    rk_aiq_uapi_sync_t sync;

    dehaze_api_mode_t mode;
    mDehazeAttr_t stManual;
    DehazeManuAttr_t stDehazeManu;
    EnhanceManuAttr_t stEnhanceManu;
} adehaze_sw_V2_t;

typedef struct AdehazeExpInfo_s {
    int hdr_mode;
    float arTime[3];
    float arAGain[3];
    float arDGain[3];
    int   arIso[3];
} AdehazeExpInfo_t;

typedef struct AdehazeV20ProcResult_s
{
    bool enable;
    bool update;
    int enhance_en;
    int hist_chn;
    int hpara_en;
    int hist_en;
    int dc_en;
    int big_en;
    int nobig_en;
    int yblk_th;
    int yhist_th;
    int dc_max_th;
    int dc_min_th;
    int wt_max;
    int bright_max;
    int bright_min;
    int tmax_base;
    int dark_th;
    int air_max;
    int air_min;
    int tmax_max;
    int tmax_off;
    int hist_th_off;
    int hist_gratio;
    int hist_min;
    int hist_k;
    int enhance_value;
    int hist_scale;
    int iir_wt_sigma;
    int iir_sigma;
    int stab_fnum;
    int iir_tmax_sigma;
    int iir_air_sigma;
    int cfg_wt;
    int cfg_air;
    int cfg_alpha;
    int cfg_gratio;
    int cfg_tmax;
    int dc_weitcur;
    int dc_thed;
    int sw_dhaz_dc_bf_h3;
    int sw_dhaz_dc_bf_h2;
    int sw_dhaz_dc_bf_h1;
    int sw_dhaz_dc_bf_h0;
    int sw_dhaz_dc_bf_h5;
    int sw_dhaz_dc_bf_h4;
    int air_weitcur;
    int air_thed;
    int air_bf_h2;
    int air_bf_h1;
    int air_bf_h0;
    int gaus_h2;
    int gaus_h1;
    int gaus_h0;
    int conv_t0[6];
    int conv_t1[6];
    int conv_t2[6];
} AdehazeV20ProcResult_t;

typedef struct AdehazeV21ProcResult_s
{
    bool enable;
    bool update;
    int enhance_en;
    int air_lc_en;
    int hpara_en;
    int hist_en;
    int dc_en;
    int yblk_th;
    int yhist_th;
    int dc_max_th;
    int dc_min_th;
    int wt_max;
    int bright_max;
    int bright_min;
    int tmax_base;
    int dark_th;
    int air_max;
    int air_min;
    int tmax_max;
    int tmax_off;
    int hist_k;
    int hist_th_off;
    int hist_min;
    int hist_gratio;
    int hist_scale;
    int enhance_value;
    int enhance_chroma;
    int iir_wt_sigma;
    int iir_sigma;
    int stab_fnum;
    int iir_tmax_sigma;
    int iir_air_sigma;
    int iir_pre_wet;
    int cfg_wt;
    int cfg_air;
    int cfg_alpha;
    int cfg_gratio;
    int cfg_tmax;
    int range_sima;
    int space_sigma_pre;
    int space_sigma_cur;
    int dc_weitcur;
    int bf_weight;
    int enh_curve[17];
    int gaus_h2;
    int gaus_h1;
    int gaus_h0;
} AdehazeV21ProcResult_t;

typedef struct AdehazeV30ProcResult_s
{
    bool enable;
    bool update;
    int round_en;
    int soft_wr_en;
    int enhance_en;
    int air_lc_en;
    int hpara_en;
    int hist_en;
    int dc_en;
    int yblk_th;
    int yhist_th;
    int dc_max_th;
    int dc_min_th;
    int wt_max;
    int bright_max;
    int bright_min;
    int tmax_base;
    int dark_th;
    int air_max;
    int air_min;
    int tmax_max;
    int tmax_off;
    int hist_k;
    int hist_th_off;
    int hist_min;
    int hist_gratio;
    int hist_scale;
    int enhance_value;
    int enhance_chroma;
    int iir_wt_sigma;
    int iir_sigma;
    int stab_fnum;
    int iir_tmax_sigma;
    int iir_air_sigma;
    int iir_pre_wet;
    int cfg_wt;
    int cfg_air;
    int cfg_alpha;
    int cfg_gratio;
    int cfg_tmax;
    int range_sima;
    int space_sigma_pre;
    int space_sigma_cur;
    int dc_weitcur;
    int bf_weight;
    int enh_curve[17];
    int gaus_h2;
    int gaus_h1;
    int gaus_h0;
    int sigma_idx[ISP3X_DHAZ_SIGMA_IDX_NUM];
    int sigma_lut[ISP3X_DHAZ_SIGMA_LUT_NUM];
    int adp_wt_wr;//calc in kernel
    int adp_air_wr;//calc in kernel
    int adp_tmax_wr;//calc in kernel
    int adp_gratio_wr;//calc in kernel
    int hist_wr[64];//calc in kernel
} AdehazeV30ProcResult_t;

typedef struct RkAiqAdehazeProcResult_s
{
    union {
        AdehazeV20ProcResult_t ProcResV20;
        AdehazeV21ProcResult_t ProcResV21;
        AdehazeV30ProcResult_t ProcResV30;
    };

} RkAiqAdehazeProcResult_t;

#endif

