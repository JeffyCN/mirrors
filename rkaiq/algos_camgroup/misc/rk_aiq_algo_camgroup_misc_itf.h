/*
 * rk_aiq_algo_camgroup_misc_itf.h
 *
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

#ifndef _RK_AIQ_ALGO_CAMGROUP_MISC_ITF_H_
#define _RK_AIQ_ALGO_CAMGROUP_MISC_ITF_H_

#include "rk_aiq_algo_des.h"
#include "RkAiqCalibDbV2Helper.h"

XCAM_BEGIN_DECLARE

extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdpcc;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAlsc;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAccm;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupA3dlut;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdhaz;
extern RkAiqAlgoDescription g_RkIspAlgoDescamgroupAgamma;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAdrc;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupArgbir;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAmerge;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAblc;
extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAblcV32;

XCAM_END_DECLARE

#endif //_RK_AIQ_ALGO_CAMGROUP_MISC_ITF_H_
