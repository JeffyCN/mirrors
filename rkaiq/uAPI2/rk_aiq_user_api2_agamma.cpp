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
#include "uAPI2/rk_aiq_user_api2_agamma.h"
#include "RkAiqCamGroupHandleInt.h"
#include "algo_handlers/RkAiqAgammaHandle.h"

RKAIQ_BEGIN_DECLARE


#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn rk_aiq_user_api2_agamma_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                             rk_aiq_gamma_attrib_V2_t attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef USE_NEWSTRUCT
#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t attr_v10;
    memset(&attr_v10, 0x0, sizeof(rk_aiq_gamma_v10_attr_t));

    attr_v10.sync.done      = attr.sync.done;
    attr_v10.sync.sync_mode = attr.sync.sync_mode;
    if (attr.atrrV21.mode == GAMMA_MODE_OFF)
        attr_v10.mode = RK_AIQ_GAMMA_MODE_AUTO;
    else if (attr.atrrV21.mode == GAMMA_MODE_MANUAL || attr.atrrV21.mode == GAMMA_MODE_FAST)
        attr_v10.mode = RK_AIQ_GAMMA_MODE_MANUAL;

    attr_v10.stManual.Gamma_en         = attr.atrrV21.stManual.Gamma_en;
    attr_v10.stManual.Gamma_out_offset = attr.atrrV21.stManual.Gamma_out_offset;
    attr_v10.stManual.Gamma_out_segnum = attr.atrrV21.stManual.Gamma_out_segnum;
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++)
        attr_v10.stManual.Gamma_curve[i] = attr.atrrV21.stManual.Gamma_curve[i];

    if (attr.atrrV21.mode == GAMMA_MODE_FAST) {
        attr_v10.stManual.Gamma_en                      = attr.atrrV21.stFast.en;
        attr_v10.stManual.Gamma_out_offset              = 0;
        attr_v10.stManual.Gamma_out_segnum              = GAMMATYPE_LOG;
        float gamma_X_v10[CALIBDB_AGAMMA_KNOTS_NUM_V10] = {
            0,   1,   2,   3,   4,   5,   6,    7,    8,    10,   12,   14,   16,   20,   24,
            28,  32,  40,  48,  56,  64,  80,   96,   112,  128,  160,  192,  224,  256,  320,
            384, 448, 512, 640, 768, 896, 1024, 1280, 1536, 1792, 2048, 2560, 3072, 3584, 4095};
        float gamma_Y_v10[CALIBDB_AGAMMA_KNOTS_NUM_V10];
        for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++) {
            gamma_Y_v10[i] = 4095 * pow(gamma_X_v10[i] / 4095, 1 / attr.atrrV21.stFast.GammaCoef +
                                                                   attr.atrrV21.stFast.SlopeAtZero);
            gamma_Y_v10[i] = gamma_Y_v10[i] > 4095 ? 4095 : gamma_Y_v10[i] < 0 ? 0 : gamma_Y_v10[i];
            attr_v10.stManual.Gamma_curve[i] = (int)(gamma_Y_v10[i] + 0.5);
        }
    }
    ret = rk_aiq_user_api2_agamma_v10_SetAttrib(sys_ctx, &attr_v10);
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(rk_aiq_gamma_v11_attr_t));

    attr_v11.sync.done      = attr.sync.done;
    attr_v11.sync.sync_mode = attr.sync.sync_mode;
    if (attr.atrrV30.mode == GAMMA_MODE_OFF)
        attr_v11.mode = RK_AIQ_GAMMA_MODE_AUTO;
    else if (attr.atrrV30.mode == GAMMA_MODE_MANUAL || attr.atrrV30.mode == GAMMA_MODE_FAST)
        attr_v11.mode = RK_AIQ_GAMMA_MODE_MANUAL;

    attr_v11.stManual.Gamma_en         = attr.atrrV30.stManual.Gamma_en;
    attr_v11.stManual.Gamma_out_offset = attr.atrrV30.stManual.Gamma_out_offset;
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V11; i++)
        attr_v11.stManual.Gamma_curve[i] = attr.atrrV30.stManual.Gamma_curve[i];

    if (attr.atrrV30.mode == GAMMA_MODE_FAST) {
        attr_v11.stManual.Gamma_en                      = attr.atrrV30.stFast.en;
        attr_v11.stManual.Gamma_out_offset              = 0;
        float gamma_X_v11[CALIBDB_AGAMMA_KNOTS_NUM_V11] = {
            0,    1,    2,    3,    4,    5,    6,    7,    8,    10,  12,   14,   16,
            20,   24,   28,   32,   40,   48,   56,   64,   80,   96,  112,  128,  160,
            192,  224,  256,  320,  384,  448,  512,  640,  768,  896, 1024, 1280, 1536,
            1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840, 4095};
        float gamma_Y_v11[CALIBDB_AGAMMA_KNOTS_NUM_V11];
        for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V11; i++) {
            gamma_Y_v11[i] = 4095 * pow(gamma_X_v11[i] / 4095, 1 / attr.atrrV30.stFast.GammaCoef +
                                                                   attr.atrrV30.stFast.SlopeAtZero);
            gamma_Y_v11[i] = gamma_Y_v11[i] > 4095 ? 4095 : gamma_Y_v11[i] < 0 ? 0 : gamma_Y_v11[i];
            attr_v11.stManual.Gamma_curve[i] = (int)(gamma_Y_v11[i] + 0.5);
        }
    }
    ret = rk_aiq_user_api2_agamma_v11_SetAttrib(sys_ctx, &attr_v11);
#endif
#endif
    return ret;
}
XCamReturn rk_aiq_user_api2_agamma_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                             rk_aiq_gamma_attrib_V2_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#ifndef USE_NEWSTRUCT
#if RKAIQ_HAVE_GAMMA_V10
    rk_aiq_gamma_v10_attr_t attr_v10;
    memset(&attr_v10, 0x0, sizeof(rk_aiq_gamma_v10_attr_t));
    ret = rk_aiq_user_api2_agamma_v10_GetAttrib(sys_ctx, &attr_v10);

    attr->sync.done      = attr_v10.sync.done;
    attr->sync.sync_mode = attr_v10.sync.sync_mode;
    if (attr_v10.mode == RK_AIQ_GAMMA_MODE_AUTO)
        attr->atrrV21.mode = GAMMA_MODE_OFF;
    else if (attr_v10.mode == RK_AIQ_GAMMA_MODE_MANUAL)
        attr->atrrV21.mode = GAMMA_MODE_MANUAL;

    attr->atrrV21.stManual.Gamma_en         = attr_v10.stManual.Gamma_en;
    attr->atrrV21.stManual.Gamma_out_offset = attr_v10.stManual.Gamma_out_offset;
    attr->atrrV21.stManual.Gamma_out_segnum = attr_v10.stManual.Gamma_out_segnum;
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V10; i++)
        attr->atrrV21.stManual.Gamma_curve[i] = attr_v10.stManual.Gamma_curve[i];
#endif
#if RKAIQ_HAVE_GAMMA_V11
    rk_aiq_gamma_v11_attr_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(rk_aiq_gamma_v11_attr_t));
    ret = rk_aiq_user_api2_agamma_v11_GetAttrib(sys_ctx, &attr_v11);

    attr->sync.done      = attr_v11.sync.done;
    attr->sync.sync_mode = attr_v11.sync.sync_mode;
    if (attr_v11.mode == RK_AIQ_GAMMA_MODE_AUTO)
        attr->atrrV30.mode = GAMMA_MODE_OFF;
    else if (attr_v11.mode == RK_AIQ_GAMMA_MODE_MANUAL)
        attr->atrrV30.mode = GAMMA_MODE_MANUAL;

    attr->atrrV30.stManual.Gamma_en         = attr_v11.stManual.Gamma_en;
    attr->atrrV30.stManual.Gamma_out_offset = attr_v11.stManual.Gamma_out_offset;
    for (int i = 0; i < CALIBDB_AGAMMA_KNOTS_NUM_V11; i++)
        attr->atrrV30.stManual.Gamma_curve[i] = attr_v11.stManual.Gamma_curve[i];
#endif
#endif
    return ret;
}

#if RKAIQ_HAVE_GAMMA_V10
XCamReturn rk_aiq_user_api2_agamma_v10_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const rk_aiq_gamma_v10_attr_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGAMMA);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgammaHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        } else {
            XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAgammaHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgammaHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGAMMA);
                if (singleCam_algo_handle) {
                    ret = singleCam_algo_handle->setAttribV10(attr);
                    if (ret != XCAM_RETURN_NO_ERROR) LOGE("%s returned: %d", __FUNCTION__, ret);
                }
            }
            return ret;
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgammaHandleInt* algo_handle =
            algoHandle<RkAiqAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api2_agamma_v10_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 rk_aiq_gamma_v10_attr_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgammaHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAgammaHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgammaHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGAMMA);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV10(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgammaHandleInt* algo_handle =
            algoHandle<RkAiqAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_agamma_v10_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const rk_aiq_gamma_v10_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_agamma_v10_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 rk_aiq_gamma_v10_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

#if RKAIQ_HAVE_GAMMA_V11
XCamReturn rk_aiq_user_api2_agamma_v11_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const rk_aiq_gamma_v11_attr_t* attr) {
#ifndef USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AGAMMA);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgammaHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgammaHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgammaHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGAMMA);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgammaHandleInt* algo_handle =
            algoHandle<RkAiqAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        }
    }

    return ret;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
}

XCamReturn rk_aiq_user_api2_agamma_v11_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 rk_aiq_gamma_v11_attr_t* attr) {
#ifndef USE_NEWSTRUCT
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAgammaHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAgammaHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAgammaHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_AGAMMA);
                if (singleCam_algo_handle) return singleCam_algo_handle->getAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAgammaHandleInt* algo_handle =
            algoHandle<RkAiqAgammaHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AGAMMA);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        }
    }

    return ret;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
}
#else
XCamReturn rk_aiq_user_api2_agamma_v11_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 const rk_aiq_gamma_v11_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_agamma_v11_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                 rk_aiq_gamma_v11_attr_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

RKAIQ_END_DECLARE
