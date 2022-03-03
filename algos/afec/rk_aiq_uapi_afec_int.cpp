/*
 * rk_aiq_uapi_afec_int.cpp
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
#include "afec/rk_aiq_uapi_afec_int.h"
#include "afec/rk_aiq_types_afec_algo_prvt.h"

XCamReturn
rk_aiq_uapi_afec_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_fec_attrib_t attr,
                           bool need_sync)
{
    FECHandle_t fec_contex = (FECHandle_t)ctx->hFEC;;

    LOGD_AFEC("Fec setAttr en(%d), bypass(%d), correct_level(%d), direction(%d)\n",
            attr.en, attr.bypass, attr.correct_level, attr.direction);

    if (fec_contex->fec_en != attr.en && \
        (fec_contex->eState == FEC_STATE_INITIALIZED || \
         fec_contex->eState == FEC_STATE_RUNNING)) {
        LOGE_AFEC("failed, Fec en(%d-%d) don't support switch at running time!\n",
                  fec_contex->fec_en, attr.en);
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (fec_contex->user_config.bypass && attr.bypass) {
        LOGE_AFEC("failed, bypass fec!\n");
        return XCAM_RETURN_ERROR_FAILED;
    }

    if (0 != memcmp(&fec_contex->user_config, &attr, sizeof(rk_aiq_fec_attrib_t)) ||\
        fec_contex->eState == FEC_STATE_INVALID) {
        memcpy(&fec_contex->user_config, &attr, sizeof(rk_aiq_fec_attrib_t));

        SmartPtr<rk_aiq_fec_attrib_t> attrPtr = new rk_aiq_fec_attrib_t;

        attrPtr->en = fec_contex->user_config.en;
        attrPtr->mode = fec_contex->user_config.mode;
        attrPtr->bypass = fec_contex->user_config.bypass;
        attrPtr->correct_level = fec_contex->user_config.correct_level;
        fec_contex->afecReadMeshThread->clear_attr();
        fec_contex->afecReadMeshThread->push_attr(attrPtr);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_afec_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_fec_attrib_t *attr)
{
    FECHandle_t fec_contex = (FECHandle_t)ctx->hFEC;;

    memcpy(attr, &fec_contex->user_config, sizeof(rk_aiq_fec_attrib_t));

    LOGD_AFEC("Fec getAttr en(%d), bypass(%d), correct_level(%d), direction(%d)\n",
            attr->en, attr->bypass, attr->correct_level, attr->direction);

    return XCAM_RETURN_NO_ERROR;
}
