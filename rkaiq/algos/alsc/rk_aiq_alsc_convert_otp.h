/*
 * rk_aiq_algo_alsc_itf.h
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

#ifndef _RK_AIQ_ALSC_CONVERT_OTP_H_
#define _RK_AIQ_ALSC_CONVERT_OTP_H_

#include "stdint.h"
#include "alsc/rk_aiq_types_alsc_algo_int.h"
#include "alsc/rk_aiq_types_alsc_algo_prvt.h"

#define c_lsc_ram_ad_bw    9                  // bit width for the RAM address
#define c_lsc_ram_d_bw     26                 // double correction factor, must be even numbers

#define c_lsc_size_bw      10                 // bit width for xsize and ysize values
#define c_lsc_grad_bw      12                 // bit width of the factor for x and y gradients calculation
#define c_lsc_size_bw_2x   (2*c_lsc_size_bw)
#define c_lsc_grad_bw_2x   (2*c_lsc_grad_bw)

#define c_lsc_sample_bw    (c_lsc_ram_d_bw/2) // bit width of the correction factor values stored in RAM
#define c_lsc_sample_bw_2x c_lsc_ram_d_bw

#define c_lsc_corr_bw       15     // bit width of the correction factor values used internal.
#define c_lsc_corr_frac_bw  12     // bit width of the fractional part of correction factor values used internal

#define c_lsc_grad_exp      15     // fixed exponent for the x and y gradients
#define c_lsc_corr_extend   10     // extended fractal part of dx,dy of internal correction factor
// constraint : c_lsc_corr_extend <= c_lsc_grad_exp
#define c_extend_round      (1 << (c_lsc_corr_extend - 1))
#define c_frac_round        (1 << (c_lsc_corr_frac_bw-1))

// bit width difference of correction factor values between used internal and stored in RAM
#define c_corr_diff  (c_lsc_corr_bw - c_lsc_sample_bw)

#define c_dx_shift   (c_lsc_grad_exp - c_lsc_corr_extend)
#define c_dx_round   (1 << (c_dx_shift - 1))
#define c_dy_shift   (c_lsc_grad_exp - c_lsc_corr_extend - c_corr_diff)
#define c_dy_round   (1 << (c_dy_shift - 1))

#define c_dx_bw      (c_lsc_corr_bw + c_lsc_grad_bw - c_dx_shift)
#define c_dy_bw      (c_lsc_sample_bw + c_lsc_grad_bw - c_dy_shift)

#define LSCDATA_LEN     289

typedef enum
{
    BAYER_BGGR = 0,
    BAYER_GBRG = 1,
    BAYER_GRBG = 2,
    BAYER_RGGB = 3,
} BayerPattern;

typedef enum
{
    R_INDEX = 0,
    GR_INDEX = 1,
    GB_INDEX = 2,
    B_INDEX = 3,
} BayerIndex;

void convertLscTableParameter(resolution_t *cur_res, alsc_otp_grad_t *otpGrad, RkAiqBayerPattern_t bayerPattern);
void computeSamplingPoint(int size, int *pos);
void computeSamplingInterval(int width, int height, int *xInterval, int *yInterval);
void getLscParameter(uint16_t *r, uint16_t *gr, uint16_t *gb, uint16_t *b, uint16_t *table, int width, int height, int bayer);
void calculateCorrectFactor(uint16_t *table, uint16_t *correctTable, int width, int height, int *xInterval, int *yInterval);
void computeGradient(int *xInterval, int *yInterval, int *xGradient, int *yGradient);
int getBayerIndex(int pattern, int row, int col);
void separateBayerChannel(uint16_t* src, uint16_t* disR, uint16_t* disGr, uint16_t* disGb, uint16_t* disB, int width, int height, int bayer);
void writeFile(char *fileName, uint16_t *buf);
void adjustVignettingForLscOTP(uint16_t *tableB, uint16_t *tableGb, uint16_t *tableGr, uint16_t *tableR, float newVig, int width, int height);
#endif
