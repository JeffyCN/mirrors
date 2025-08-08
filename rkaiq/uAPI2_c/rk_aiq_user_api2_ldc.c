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

#include "aiq_core_c/algo_handlers/RkAiqLdcHandler.h"
#include "newStruct/ldc/include/ldc_algo_api.h"
#include "uAPI2_c/rk_aiq_user_api2_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

static bool check_attrib(ldc_api_attrib_t* attr) {
#if RKAIQ_HAVE_LDCV
    if (attr->tunning.enMode == LDC_LDCH_LDCV_EN && !attr->en) {
        LOGE_ALDC("LDC mode is `LDC_LDCH_LDCV_EN`. Disabling is not supported");
        return false;
    }
#endif

    if (attr->opMode == RK_AIQ_OP_MODE_MANUAL) {
        char* path = attr->tunning.extMeshFile.sw_ldcT_extMeshFile_path;
        if (strlen(path) > 0) {
            char* name = attr->tunning.extMeshFile.sw_ldcT_ldchExtMeshFile_name;
            if (!strlen(name)) name = DEFAULT_LDCH_MESH;

            char file_name[800] = "\0";
            sprintf(file_name, "%s/%s", path, name);
            if (access(file_name, F_OK) < 0) {
                LOGE_ALDC("%s does not exist", file_name);
                return false;
            }

#if RKAIQ_HAVE_LDCV
            name = attr->tunning.extMeshFile.sw_ldcT_ldcvExtMeshFile_name;
            if (!strlen(name)) name = DEFAULT_LDCV_MESH;

            memset(file_name, 0, sizeof(file_name));
            sprintf(file_name, "%s/%s", path, name);
            if (access(file_name, F_OK) < 0) {
                LOGE_ALDC("%s does not exist", file_name);
                return false;
            }
#endif
        }
    }

    return true;
}

static bool check_manualAttrib(const rk_aiq_sys_ctx_t* ctx, ldc_param_t* attr) {
    if (attr->sta.ldchCfg.en) {
        uint32_t size = attr->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMap_size;
        void* vaddr   = attr->sta.ldchCfg.lutMapCfg.sw_ldcT_lutMapBuf_vaddr[0];
        if (size <= 0 || !vaddr) {
            LOGE_ALDC("LDCH map addr %p, size %d is error!", vaddr, size);
            return false;
        }
    }

#if RKAIQ_HAVE_LDCV
    int type           = RESULT_TYPE_LDC_PARAM;
    int aut_param_size = sizeof(ldc_param_auto_t);

    rk_aiq_op_mode_t opMode;
    bool en;
    bool bypass;
    ldc_param_auto_t aut_param;

    XCamReturn ret = rk_aiq_user_api2_common_processParams(ctx, false, &opMode, &en, &bypass, type,
                                                           0, NULL, aut_param_size, &aut_param);
    if (ret >= 0) {
        LOGD_ALDC("LDCV en %d, opMode %d, ", en, opMode, aut_param.enMode, aut_param);
        if (aut_param.enMode != LDC_LDCH_LDCV_EN && attr->sta.ldcvCfg.en) {
            LOGE_ALDC("LDC mode is %s. Pls change enMode to `LDC_LDCH_LDCV_EN`",
                      aut_param.enMode == LDC_LDCH_LDCV_EN ? "LDC_LDCH_LDCV_EN" : "LDC_LDCH_EN");
            return false;
        }

        if (aut_param.enMode == LDC_LDCH_LDCV_EN &&
            (!attr->sta.ldchCfg.en || !attr->sta.ldcvCfg.en)) {
            LOGE_ALDC("LDC mode is `LDC_LDCH_LDCV_EN`. Disabling is not supported");
            return false;
        }
    }

    if (attr->sta.ldcvCfg.en) {
        uint32_t size = attr->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMap_size;
        void* vaddr   = attr->sta.ldcvCfg.lutMapCfg.sw_ldcT_lutMapBuf_vaddr[0];
        if (size <= 0 || !vaddr) {
            LOGE_ALDC("LDCV map addr %p, size %d is error!", vaddr, size);
            return false;
        }
    }
#endif

    return true;
}

XCamReturn rk_aiq_user_api2_ldc_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldc_api_attrib_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

    int type           = RESULT_TYPE_LDC_PARAM;
    int man_param_size = sizeof(ldc_param_t);
    int aut_param_size = sizeof(ldc_param_auto_t);

    if (!check_attrib(attr)) return XCAM_RETURN_ERROR_FILE;

    for (int i = 0; i < ctx_array.num; i++) {
        ret = rk_aiq_user_api2_common_processParams(ctx_array.ctx[i], true, &attr->opMode,
                                                    &attr->en, &attr->bypass, type, man_param_size,
                                                    NULL, aut_param_size, &attr->tunning);
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_ldc_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldc_api_attrib_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

    int type           = RESULT_TYPE_LDC_PARAM;
    int man_param_size = sizeof(ldc_param_t);
    int aut_param_size = sizeof(ldc_param_auto_t);

    return rk_aiq_user_api2_common_processParams(ctx, false, &attr->opMode, &attr->en,
                                                 &attr->bypass, type, man_param_size, NULL,
                                                 aut_param_size, &attr->tunning);
}

XCamReturn rk_aiq_user_api2_ldc_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, ldc_status_t* status) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

    int type           = RESULT_TYPE_LDC_PARAM;
    int man_param_size = sizeof(ldc_param_t);
    int aut_param_size = sizeof(ldc_param_auto_t);

    return rk_aiq_user_api2_common_processParams(ctx, false, &status->opMode, &status->en,
                                                 &status->bypass, type, man_param_size, NULL,
                                                 aut_param_size, &status->tunning);

    return ret;
}

XCamReturn rk_aiq_user_api2_ldc_SetManualAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                ldc_param_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    rk_aiq_sys_ctx_array_t ctx_array = rk_aiq_user_api2_common_getSysCtxArray(sys_ctx);

    int type                = RESULT_TYPE_LDC_PARAM;
    int man_param_size      = sizeof(ldc_param_t);
    rk_aiq_op_mode_t opMode = RK_AIQ_OP_MODE_MANUAL;
    bool en                 = true;
    bool bypass             = 0;


    for (int i = 0; i < ctx_array.num; i++) {
        if (!check_manualAttrib(ctx_array.ctx[i], attr)) return XCAM_RETURN_ERROR_PARAM;

        ret = rk_aiq_user_api2_common_processParams(ctx_array.ctx[i], true, &opMode, &en, &bypass,
                                                    type, man_param_size, attr, 0, NULL);
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_ldc_GetManualAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                ldc_param_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDC);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    const rk_aiq_sys_ctx_t* ctx = rk_aiq_user_api2_common_getSysCtx(sys_ctx);

    int type           = RESULT_TYPE_LDC_PARAM;
    int man_param_size = sizeof(ldc_param_t);
    ldc_param_t man    = {0};

    ret = rk_aiq_user_api2_common_processParams(ctx, false, NULL, NULL, NULL, type, man_param_size,
                                                &man, 0, NULL);

    attr->sta.ldchCfg.en = man.sta.ldchCfg.en;
#if RKAIQ_HAVE_LDCV
    attr->sta.ldcvCfg.en = man.sta.ldcvCfg.en;
#endif

    return ret;
}

RKAIQ_END_DECLARE
