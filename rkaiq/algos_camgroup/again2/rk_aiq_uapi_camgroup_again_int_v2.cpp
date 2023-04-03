/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
 */

#include "again2/rk_aiq_uapi_camgroup_again_int_v2.h"
#include "rk_aiq_types_camgroup_again_prvt_v2.h"

#if 1
#define ACNRV2_CHROMA_SF_STRENGTH_MAX_PERCENT (100.0)


XCamReturn
rk_aiq_uapi_camgroup_againV2_SetAttrib(RkAiqAlgoContext *ctx,
                                       const rk_aiq_gain_attrib_v2_t *attr,
                                       bool need_sync)
{
    CamGroup_AgainV2_Contex_t *pGroupCtx = (CamGroup_AgainV2_Contex_t *)ctx;
    Again_Context_V2_t* pCtx = pGroupCtx->again_contex_v2;

    pCtx->eMode = attr->eMode;
    if(pCtx->eMode == AGAINV2_OP_MODE_AUTO) {
        pCtx->stAuto = attr->stAuto;
    } else if(pCtx->eMode == AGAINV2_OP_MODE_MANUAL) {
        pCtx->stManual.stSelect = attr->stManual.stSelect;
    }
    pCtx->isReCalculate |= 1;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_againV2_GetAttrib(const RkAiqAlgoContext *ctx,
                                       rk_aiq_gain_attrib_v2_t *attr)
{

    CamGroup_AgainV2_Contex_t *pGroupCtx = (CamGroup_AgainV2_Contex_t *)ctx;
    Again_Context_V2_t* pCtx = pGroupCtx->again_contex_v2;

    attr->eMode = pCtx->eMode;
    memcpy(&attr->stAuto, &pCtx->stAuto, sizeof(attr->stAuto));
    memcpy(&attr->stManual, &pCtx->stManual, sizeof(attr->stManual));

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_camgroup_againV2_GetInfo(const RkAiqAlgoContext *ctx,
                                     rk_aiq_gain_info_v2_t *pInfo)
{
    CamGroup_AgainV2_Contex_t *pGroupCtx = (CamGroup_AgainV2_Contex_t *)ctx;
    Again_Context_V2_t* pCtx = pGroupCtx->again_contex_v2;


    pInfo->iso = pCtx->stExpInfo.arIso[pCtx->stExpInfo.hdr_mode];
    pInfo->expo_info = pCtx->stExpInfo;

    return XCAM_RETURN_NO_ERROR;
}


#endif

