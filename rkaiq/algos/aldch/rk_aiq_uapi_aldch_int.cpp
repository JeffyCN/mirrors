/*
 * rk_aiq_uapi_aldch_int.cpp
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

#include "xcam_log.h"
#include "aldch/rk_aiq_uapi_aldch_int.h"
#include "aldch/rk_aiq_types_aldch_algo_prvt.h"

XCamReturn
rk_aiq_uapi_aldch_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_ldch_attrib_t attr,
                           bool need_sync)
{
    LDCHHandle_t ldch_contex = (LDCHHandle_t)ctx->hLDCH;

    if (!ldch_contex->ldch_en && !attr.en) {
        LOGE_AFEC("failed, ldch is disalbed!");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (0 != memcmp(&ldch_contex->user_config, &attr, sizeof(rk_aiq_ldch_attrib_t))) {
        memcpy(&ldch_contex->user_config, &attr, sizeof(rk_aiq_ldch_attrib_t));

        SmartPtr<rk_aiq_ldch_attrib_t> attrPtr = new rk_aiq_ldch_attrib_t;

        attrPtr->en = ldch_contex->user_config.en;
        attrPtr->correct_level = ldch_contex->user_config.correct_level;
        ldch_contex->aldchReadMeshThread->clear_attr();
        ldch_contex->aldchReadMeshThread->push_attr(attrPtr);
        LOGV_ALDCH("ldch en(%d-%d), level(%d)\n",
                  ldch_contex->ldch_en, attrPtr->en, attrPtr->correct_level);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aldch_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ldch_attrib_t *attr)
{
    LDCHHandle_t ldch_contex = (LDCHHandle_t)ctx->hLDCH;

    memcpy(attr, &ldch_contex->user_config, sizeof(rk_aiq_ldch_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}
