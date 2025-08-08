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
#include "aldch/rk_aiq_uapi_aldch_v21_int.h"
#include "aldch/rk_aiq_types_aldch_algo_prvt.h"

#define DISABLE_HANDLE_ATTRIB

XCamReturn
rk_aiq_uapi_aldch_v21_SetAttrib(RkAiqAlgoContext *ctx,
                           rk_aiq_ldch_v21_attrib_t attr,
                           bool /* need_sync */)
{
    LDCHHandle_t ldch_contex = (LDCHHandle_t)ctx->hLDCH;

    LOGD_ALDCH("%s, attr en:%d, level:%d, bic_en:%d, zero_interp_en:%d, sample_avr_en:%d",
            __FUNCTION__,
            attr.en,
            attr.correct_level,
            attr.bic_mode_en,
            attr.zero_interp_en,
            attr.sample_avr_en);

    LOGD_ALDCH("%s, attr custom lut: mode %d, flag %d",
            __FUNCTION__,
            attr.update_lut_mode,
            attr.lut.update_flag);

    if (attr.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_FILE) {
        LOGD_ALDCH("%s, attr custom lut file name: %s/%s",
                __FUNCTION__,
                attr.lut.u.file.config_file_dir,
                attr.lut.u.file.mesh_file_name);
    } else if (attr.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER) {
        LOGD_ALDCH("%s, attr custom lut buffer %p, size %zu",
                __FUNCTION__,
                attr.lut.u.buffer.addr,
                attr.lut.u.buffer.size);
        if (attr.lut.u.buffer.size == 0) {
            LOGE_ALDCH("Invalid lut buffer size %zu", attr.lut.u.buffer.size);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }

    if (!ldch_contex->ldch_en && !attr.en) {
        LOGE_ALDCH("uapi want to disalbe ldch, but ldch has been disalbed!");
        return XCAM_RETURN_NO_ERROR;
    }

    if (0 != memcmp(&ldch_contex->user_config, &attr, sizeof(rk_aiq_ldch_v21_attrib_t))) {
#ifdef DISABLE_HANDLE_ATTRIB
        if (attr.update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER && \
            attr.lut.update_flag) {
            if (!ldch_contex->_lutCache.ptr()) {
                ldch_contex->_lutCache = new LutCache(attr.lut.u.buffer.size);
            } else if (attr.lut.u.buffer.size != ldch_contex->_lutCache->GetSize()) {
                ldch_contex->_lutCache.release();
                ldch_contex->_lutCache = new LutCache(attr.lut.u.buffer.size);
            }

            if (ldch_contex->_lutCache.ptr()) {
                if (ldch_contex->_lutCache->GetBuffer()) {
                    memcpy(ldch_contex->_lutCache->GetBuffer(), attr.lut.u.buffer.addr, attr.lut.u.buffer.size);
                }
            } else {
                LOGE_ALDCH("Failed to malloc ldch cache!");
                return XCAM_RETURN_ERROR_MEM;
            }
        }
#endif

        memcpy(&ldch_contex->user_config, &attr, sizeof(attr));
        ldch_contex->isAttribUpdated = true;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_aldch_v21_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ldch_v21_attrib_t *attr)
{
    LDCHHandle_t ldch_contex = (LDCHHandle_t)ctx->hLDCH;

    memcpy(attr, &ldch_contex->user_config, sizeof(*attr));

    LOGD_ALDCH("%s, attr en:%d, level:%d, bic_en:%d, zero_interp_en:%d, sample_avr_en:%d",
            __FUNCTION__,
            attr->en,
            attr->correct_level,
            attr->bic_mode_en,
            attr->zero_interp_en,
            attr->sample_avr_en);

    LOGD_ALDCH("%s, attr custom lut: mode %d, flag %d",
            __FUNCTION__,
            attr->update_lut_mode,
            attr->lut.update_flag);

    if (attr->update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_FILE) {
        LOGD_ALDCH("%s, attr custom lut file name: %s/%s",
                __FUNCTION__,
                attr->lut.u.file.config_file_dir,
                attr->lut.u.file.mesh_file_name);
    } else if (attr->update_lut_mode == RK_AIQ_LDCH_UPDATE_LUT_FROM_EXTERNAL_BUFFER) {
        LOGD_ALDCH("%s, attr custom lut buffer %p, size %zu",
                __FUNCTION__,
                attr->lut.u.buffer.addr,
                attr->lut.u.buffer.size);
    }

    return XCAM_RETURN_NO_ERROR;
}
