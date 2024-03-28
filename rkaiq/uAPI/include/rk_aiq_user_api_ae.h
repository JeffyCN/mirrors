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

#ifndef __RK_AIQ_USER_API_AE_H__
#define __RK_AIQ_USER_API_AE_H__

#include "ae/rk_aiq_uapi_ae_int.h"

typedef struct rk_aiq_sys_ctx_s rk_aiq_sys_ctx_t;

RKAIQ_BEGIN_DECLARE

XCamReturn rk_aiq_user_api_ae_setExpSwAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_ExpSwAttr_t expSwAttr);
XCamReturn rk_aiq_user_api_ae_getExpSwAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_ExpSwAttr_t* pExpSwAttr);
XCamReturn rk_aiq_user_api_ae_setLinExpAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_LinExpAttr_t linExpAttr);
XCamReturn rk_aiq_user_api_ae_getLinExpAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_LinExpAttr_t* pLinExpAttr);
XCamReturn rk_aiq_user_api_ae_setHdrExpAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_HdrExpAttr_t hdrExpAttr);
XCamReturn rk_aiq_user_api_ae_getHdrExpAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_HdrExpAttr_t* pHdrExpAttr);
XCamReturn rk_aiq_user_api_ae_setLinAeRouteAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_LinAeRouteAttr_t linAeRouteAttr);
XCamReturn rk_aiq_user_api_ae_getLinAeRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr);
XCamReturn rk_aiq_user_api_ae_setHdrAeRouteAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr);
XCamReturn rk_aiq_user_api_ae_getHdrAeRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr);

XCamReturn rk_aiq_user_api_ae_queryExpResInfo(const rk_aiq_sys_ctx_t* ctx, Uapi_ExpQueryInfo_t* pExpResInfo);
XCamReturn rk_aiq_user_api_ae_setExpWinAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_ExpWin_t ExpWinAttr);
XCamReturn rk_aiq_user_api_ae_getExpWinAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_ExpWin_t* pExpWinAttr);
XCamReturn rk_aiq_user_api_ae_setAecStatsCfg(const rk_aiq_sys_ctx_t* ctx, const Uapi_AecStatsCfg_t AecStatsCfg);
XCamReturn rk_aiq_user_api_ae_getAecStatsCfg(const rk_aiq_sys_ctx_t* ctx, Uapi_AecStatsCfg_t* pAecStatsCfg);

RKAIQ_END_DECLARE

#endif /*__RK_AIQ_USER_API_AE_H__*/
