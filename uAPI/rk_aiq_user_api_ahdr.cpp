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

#include "rk_aiq_user_api_ahdr.h"
#include "RkAiqHandleInt.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

/*
XCamReturn
rk_aiq_user_api_ahdr_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t attr)
{
    amerge_attrib_t mergeAttr;
    memset(&mergeAttr, 0, sizeof(amerge_attrib_t));
    atmo_attrib_t tmoAttr;
    memset(&tmoAttr, 0, sizeof(atmo_attrib_t));

    TransferSetData(&mergeAttr, &tmoAttr, &attr);

    XCamReturn ret_amerge = rk_aiq_user_api2_amerge_SetAttrib(sys_ctx, mergeAttr);
    XCamReturn ret_atmo = rk_aiq_user_api2_atmo_SetAttrib(sys_ctx, tmoAttr);


    if (ret_amerge != XCAM_RETURN_NO_ERROR)
        return ret_amerge;

    if (ret_atmo != XCAM_RETURN_NO_ERROR)
        return ret_atmo;

    return  XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api_ahdr_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t* attr)
{
    amerge_attrib_t mergeAttr;
    memset(&mergeAttr, 0, sizeof(amerge_attrib_t));
    atmo_attrib_t tmoAttr;
    memset(&tmoAttr, 0, sizeof(atmo_attrib_t));

    TransferGetData(&mergeAttr, &tmoAttr, attr);

    XCamReturn ret_amerge = rk_aiq_user_api2_amerge_GetAttrib(sys_ctx, &mergeAttr);
    XCamReturn ret_atmo = rk_aiq_user_api2_atmo_GetAttrib(sys_ctx, &tmoAttr);

    if (ret_amerge != XCAM_RETURN_NO_ERROR)
        return ret_amerge;

    if (ret_atmo != XCAM_RETURN_NO_ERROR)
        return ret_atmo;

    return XCAM_RETURN_NO_ERROR;
}
*/
RKAIQ_END_DECLARE
