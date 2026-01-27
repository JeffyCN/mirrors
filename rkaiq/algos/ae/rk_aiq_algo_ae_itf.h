/*
 * rk_aiq_algo_ae_itf.h
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


#ifndef _RK_AIQ_ALGO_AE_ITF_H_
#define _RK_AIQ_ALGO_AE_ITF_H_


#include "rk_aiq_algo_des.h"

/*
 ***************** AE LIB VERSION NOTE *****************
 * v0.0.9:
 * basic version, support ae smooth & ae speed up
 * v0.1.0:
 * optimize ae smooth, add delay trigger
 * v0.1.1-20200521:
 * add dcg switch function in ae, adapt for Hdr/Normal
 * add Antiflicker mode:normal & auto
 * v0.1.2-20200620
 * add Longframe Mode in Hdr
 * including Normal/LongFrame/Auto_Longframe
 * v0.1.3-20200623
 * add auto fps function
 * Attention: for sony sensor, vts will be effective more quickly than exposure,
 *                whitch will cause flicker.
 * v0.1.4-20200722
 * for LinearAe: aestats can use rawae/yuvae
 * DCG-setting is divided into 2 parts: Hdr/Normal
 * GainRange is divided into 2 parts: Linear/NonLinear(for sony sensor, dB-Mode)
 * LCG=>LSNR, HCG=>HSNR
 * v0.1.5-20200814
 * In LinAe, Backlit support designated DarkROI
 *              add OverExpCtrl
 * delete some useless modules,including:AOE,Hist2Hal,IntervalAdjust
 * v0.1.6-20201211
 * support P-iris/DC-iris
 * support SyncTest
 * v0.1.7-20210127
 * support R/G/B/Y rawstats
 * support dynamic AecSpeed
 */


#define RKISP_ALGO_AE_VERSION     "v0.1.7"
#define RKISP_ALGO_AE_VENDOR      "Rockchip"
#define RKISP_ALGO_AE_DESCRIPTION "Rockchip Ae algo for ISP2.0"

XCAM_BEGIN_DECLARE

extern RkAiqAlgoDescription g_RkIspAlgoDescAe;

XCAM_END_DECLARE

#endif //_RK_AIQ_ALGO_AE_ITF_H_
