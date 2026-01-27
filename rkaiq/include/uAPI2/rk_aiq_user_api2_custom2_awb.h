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

#ifndef _RK_AIQ_USER_API_CUSTOM_AWB_H_
#define _RK_AIQ_USER_API_CUSTOM_AWB_H_

#include "common/rk_aiq_types.h" /* common structs */
#include "uAPI2/rk_aiq_user_api2_sysctl.h" /* rk_aiq_sys_ctx_t */
#include "uAPI2/rk_aiq_user_api2_custom2_awb_type.h"
RKAIQ_BEGIN_DECLARE



/*!
 * \brief register custom Awb algo
 *
 * \param[in] ctx             context
 * \param[in] cbs             custom Awb callbacks
 * \note should be called after rk_aiq_uapi_sysctl_init
 */
XCamReturn
rk_aiq_uapi2_awb_register(const rk_aiq_sys_ctx_t* ctx, rk_aiq_customeAwb_cbs_t* cbs);

/*!
 * \brief enable/disable custom Awb algo
 *
 * \param[in] ctx             context
 * \param[in] enable          enable/diable custom Awb
 * \note should be called after rk_aiq_uapi_customAWB_register. If custom Awb was enabled,
 *       Rk awb will be stopped, vice versa.
 */
XCamReturn
rk_aiq_uapi2_customAWB_enable(const rk_aiq_sys_ctx_t* ctx, bool enable);

/*!
 * \brief unregister custom Awb algo
 *
 * \param[in] ctx             context
 * \note should be called after rk_aiq_uapi_customAWB_register.
 */
XCamReturn
rk_aiq_uapi2_awb_unRegister(const rk_aiq_sys_ctx_t* ctx);

RKAIQ_END_DECLARE

#endif
