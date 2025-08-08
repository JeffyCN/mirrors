/*
 * rk_aiq_algo_camgroup_ae_itf.h
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


#ifndef _RK_AIQ_ALGO_CAMGROUP_AE_ITF_H_
#define _RK_AIQ_ALGO_CAMGROUP_AE_ITF_H_


#include "rk_aiq_algo_des.h"

/*
 ***************** GROUP AE LIB VERSION NOTE *****************
 * v0.0.1:
 * basic version, support Group ae
 */

#define RKISP_ALGO_GROUP_AE_VERSION     "v0.0.1"
#define RKISP_ALGO_GROUP_AE_VENDOR      "Rockchip"
#define RKISP_ALGO_GROUP_AE_DESCRIPTION "Rockchip Ae camgroup algo for ISP3.0"

XCAM_BEGIN_DECLARE

extern RkAiqAlgoDescription g_RkIspAlgoDescCamgroupAe;

XCAM_END_DECLARE

#endif //_RK_AIQ_ALGO_CAMGROUP_AE_ITF_H_
