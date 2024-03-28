/*
 *rk_aiq_types_alsc_hw.h
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#ifndef _RK_AIQ_TYPE_ACNR_HW_V31_H_
#define _RK_AIQ_TYPE_ACNR_HW_V31_H_
#include "rk_aiq_comm.h"

typedef struct RK_CNR_Fix_V31_s {
   
   /* CNR_CTRL */
   uint8_t cnr_en;
   uint8_t exgain_bypass;
   uint8_t yuv422_mode;
   uint8_t thumb_mode;
   uint8_t hiflt_wgt0_mode;
   uint8_t loflt_coeff;
   /* CNR_EXGAIN */
   uint16_t global_gain;
   uint8_t global_gain_alpha;
   uint8_t local_gain_scale;
   /* CNR_THUMB1 */
   uint16_t lobfflt_vsigma_uv;
   uint16_t lobfflt_vsigma_y;
   /* CNR_THUMB_BF_RATIO */
   uint16_t lobfflt_alpha;
   /* CNR_LBF_WEITD */
   uint8_t thumb_bf_coeff[4];
   /* CNR_IIR_PARA1 */
   uint8_t loflt_uv_gain;
   uint8_t loflt_vsigma;
   uint8_t exp_x_shift_bit;
   uint16_t loflt_wgt_slope;
   /* CNR_IIR_PARA2 */
   uint8_t loflt_wgt_min_thred;
   uint8_t loflt_wgt_max_limit;
   /* CNR_GAUS_COE */
   uint8_t gaus_flt_coeff[6];
   /* CNR_GAUS_RATIO */
   uint16_t gaus_flt_alpha;
   uint8_t hiflt_wgt_min_limit;
   uint16_t hiflt_alpha;
   /* CNR_BF_PARA1 */
   uint8_t hiflt_uv_gain;
   uint16_t hiflt_global_vsigma;
   uint8_t hiflt_cur_wgt;
   /* CNR_BF_PARA2 */
   uint16_t adj_offset;
   uint16_t adj_scale;
   /* CNR_SIGMA */
   uint8_t sgm_ratio[13];
   /* CNR_IIR_GLOBAL_GAIN */
   uint8_t loflt_global_sgm_ratio;
   uint8_t loflt_global_sgm_ratio_alpha;
   /* CNR_WGT_SIGMA */
   uint8_t cur_wgt[13];
   /* GAUS_X_SIGMAR */
   uint16_t hiflt_vsigma_idx[8];
   /* GAUS_Y_SIGMAR */
   uint16_t hiflt_vsigma[8];


} RK_CNR_Fix_V31_t;

#endif



