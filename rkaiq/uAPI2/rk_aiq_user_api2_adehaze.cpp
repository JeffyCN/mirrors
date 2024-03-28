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
#include "uAPI2/rk_aiq_user_api2_adehaze.h"

#include "algo_handlers/RkAiqAdehazeHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

XCamReturn rk_aiq_user_api2_adehaze_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                adehaze_sw_V2_t attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO

    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(adehaze_sw_v11_t));

    // set sync
    attr_v11.sync.sync_mode = attr.sync.sync_mode;
    attr_v11.sync.done      = attr.sync.done;
    // set mode
    attr_v11.mode = attr.mode;
    // set stAuto
    attr_v11.stAuto.DehazeTuningPara.Enable    = attr.stAuto.DehazeTuningPara.Enable;
    attr_v11.stAuto.DehazeTuningPara.cfg_alpha = attr.stAuto.DehazeTuningPara.cfg_alpha;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.en =
        attr.stAuto.DehazeTuningPara.dehaze_setting.en;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.air_lc_en =
        attr.stAuto.DehazeTuningPara.dehaze_setting.air_lc_en;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.stab_fnum =
        attr.stAuto.DehazeTuningPara.dehaze_setting.stab_fnum;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.sigma =
        attr.stAuto.DehazeTuningPara.dehaze_setting.sigma;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.wt_sigma =
        attr.stAuto.DehazeTuningPara.dehaze_setting.wt_sigma;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.air_sigma =
        attr.stAuto.DehazeTuningPara.dehaze_setting.air_sigma;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.tmax_sigma =
        attr.stAuto.DehazeTuningPara.dehaze_setting.tmax_sigma;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.pre_wet =
        attr.stAuto.DehazeTuningPara.dehaze_setting.pre_wet;
    attr_v11.stAuto.DehazeTuningPara.dehaze_setting.DehazeData =
        attr.stAuto.DehazeTuningPara.dehaze_setting.DehazeData;
    attr_v11.stAuto.DehazeTuningPara.enhance_setting.en =
        attr.stAuto.DehazeTuningPara.enhance_setting.en;
    attr_v11.stAuto.DehazeTuningPara.enhance_setting.EnhanceData =
        attr.stAuto.DehazeTuningPara.enhance_setting.EnhanceData;
    for (int i = 0; i < DHAZ_ENHANCE_CURVE_KNOTS_NUM; i++)
        attr_v11.stAuto.DehazeTuningPara.enhance_setting.enhance_curve[i] =
            attr.stAuto.DehazeTuningPara.enhance_setting.enhance_curve[i];
    attr_v11.stAuto.DehazeTuningPara.hist_setting.en = attr.stAuto.DehazeTuningPara.hist_setting.en;
    attr_v11.stAuto.DehazeTuningPara.hist_setting.hist_para_en =
        attr.stAuto.DehazeTuningPara.hist_setting.hist_para_en;
    attr_v11.stAuto.DehazeTuningPara.hist_setting.HistData =
        attr.stAuto.DehazeTuningPara.hist_setting.HistData;
    // set stManual
    attr_v11.stManual.Enable                    = attr.stManual.Enable;
    attr_v11.stManual.cfg_alpha                 = attr.stManual.cfg_alpha;
    attr_v11.stManual.dehaze_setting.en         = attr.stManual.dehaze_setting.en;
    attr_v11.stManual.dehaze_setting.air_lc_en  = attr.stManual.dehaze_setting.air_lc_en;
    attr_v11.stManual.dehaze_setting.stab_fnum  = attr.stManual.dehaze_setting.stab_fnum;
    attr_v11.stManual.dehaze_setting.sigma      = attr.stManual.dehaze_setting.sigma;
    attr_v11.stManual.dehaze_setting.wt_sigma   = attr.stManual.dehaze_setting.wt_sigma;
    attr_v11.stManual.dehaze_setting.air_sigma  = attr.stManual.dehaze_setting.air_sigma;
    attr_v11.stManual.dehaze_setting.tmax_sigma = attr.stManual.dehaze_setting.tmax_sigma;
    attr_v11.stManual.dehaze_setting.pre_wet    = attr.stManual.dehaze_setting.pre_wet;
    attr_v11.stManual.dehaze_setting.DehazeData = attr.stManual.dehaze_setting.DehazeData;
    attr_v11.stManual.enhance_setting.en        = attr.stManual.enhance_setting.en;
    attr_v11.stManual.enhance_setting.EnhanceData.enhance_value =
        attr.stManual.enhance_setting.EnhanceData.enhance_value;
    attr_v11.stManual.enhance_setting.EnhanceData.enhance_chroma =
        attr.stManual.enhance_setting.EnhanceData.enhance_chroma;
    for (int i = 0; i < DHAZ_ENHANCE_CURVE_KNOTS_NUM; i++)
        attr_v11.stManual.enhance_setting.enhance_curve[i] =
            attr.stManual.enhance_setting.enhance_curve[i];
    attr_v11.stManual.hist_setting.en           = attr.stManual.hist_setting.en;
    attr_v11.stManual.hist_setting.hist_para_en = attr.stManual.hist_setting.hist_para_en;
    attr_v11.stManual.hist_setting.HistData     = attr.stManual.hist_setting.HistData;
    // set info
    attr_v11.Info.updateMDehazeStrth  = attr.stDehazeManu.update;
    attr_v11.Info.MDehazeStrth        = attr.stDehazeManu.level;
    attr_v11.Info.updateMEnhanceStrth = attr.stEnhanceManu.update;
    attr_v11.Info.MEnhanceStrth       = attr.stEnhanceManu.level;

    ret = rk_aiq_user_api2_adehaze_v11_setSwAttrib(sys_ctx, &attr_v11);
#endif
    return ret;
}
XCamReturn rk_aiq_user_api2_adehaze_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                adehaze_sw_V2_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO

    adehaze_sw_v11_t attr_v11;
    memset(&attr_v11, 0x0, sizeof(adehaze_sw_v11_t));
    ret = rk_aiq_user_api2_adehaze_v11_getSwAttrib(sys_ctx, &attr_v11);

    // set sync
    attr->sync.sync_mode = attr_v11.sync.sync_mode;
    attr->sync.done      = attr_v11.sync.done;
    // set mode
    attr->mode = attr_v11.mode;
    // set stAuto
    attr->stAuto.DehazeTuningPara.Enable    = attr_v11.stAuto.DehazeTuningPara.Enable;
    attr->stAuto.DehazeTuningPara.cfg_alpha = attr_v11.stAuto.DehazeTuningPara.cfg_alpha;
    attr->stAuto.DehazeTuningPara.dehaze_setting.en =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.en;
    attr->stAuto.DehazeTuningPara.dehaze_setting.air_lc_en =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.air_lc_en;
    attr->stAuto.DehazeTuningPara.dehaze_setting.stab_fnum =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.stab_fnum;
    attr->stAuto.DehazeTuningPara.dehaze_setting.sigma =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.sigma;
    attr->stAuto.DehazeTuningPara.dehaze_setting.wt_sigma =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.wt_sigma;
    attr->stAuto.DehazeTuningPara.dehaze_setting.air_sigma =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.air_sigma;
    attr->stAuto.DehazeTuningPara.dehaze_setting.tmax_sigma =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.tmax_sigma;
    attr->stAuto.DehazeTuningPara.dehaze_setting.pre_wet =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.pre_wet;
    attr->stAuto.DehazeTuningPara.dehaze_setting.DehazeData =
        attr_v11.stAuto.DehazeTuningPara.dehaze_setting.DehazeData;
    attr->stAuto.DehazeTuningPara.enhance_setting.en =
        attr_v11.stAuto.DehazeTuningPara.enhance_setting.en;
    attr->stAuto.DehazeTuningPara.enhance_setting.EnhanceData =
        attr_v11.stAuto.DehazeTuningPara.enhance_setting.EnhanceData;
    for (int i = 0; i < DHAZ_ENHANCE_CURVE_KNOTS_NUM; i++)
        attr->stAuto.DehazeTuningPara.enhance_setting.enhance_curve[i] =
            attr_v11.stAuto.DehazeTuningPara.enhance_setting.enhance_curve[i];
    attr->stAuto.DehazeTuningPara.hist_setting.en =
        attr_v11.stAuto.DehazeTuningPara.hist_setting.en;
    attr->stAuto.DehazeTuningPara.hist_setting.hist_para_en =
        attr_v11.stAuto.DehazeTuningPara.hist_setting.hist_para_en;
    attr->stAuto.DehazeTuningPara.hist_setting.HistData =
        attr_v11.stAuto.DehazeTuningPara.hist_setting.HistData;
    // set stManual
    attr->stManual.Enable                    = attr_v11.stManual.Enable;
    attr->stManual.cfg_alpha                 = attr_v11.stManual.cfg_alpha;
    attr->stManual.dehaze_setting.en         = attr_v11.stManual.dehaze_setting.en;
    attr->stManual.dehaze_setting.air_lc_en  = attr_v11.stManual.dehaze_setting.air_lc_en;
    attr->stManual.dehaze_setting.stab_fnum  = attr_v11.stManual.dehaze_setting.stab_fnum;
    attr->stManual.dehaze_setting.sigma      = attr_v11.stManual.dehaze_setting.sigma;
    attr->stManual.dehaze_setting.wt_sigma   = attr_v11.stManual.dehaze_setting.wt_sigma;
    attr->stManual.dehaze_setting.air_sigma  = attr_v11.stManual.dehaze_setting.air_sigma;
    attr->stManual.dehaze_setting.tmax_sigma = attr_v11.stManual.dehaze_setting.tmax_sigma;
    attr->stManual.dehaze_setting.pre_wet    = attr_v11.stManual.dehaze_setting.pre_wet;
    attr->stManual.dehaze_setting.DehazeData = attr_v11.stManual.dehaze_setting.DehazeData;
    attr->stManual.enhance_setting.en        = attr_v11.stManual.enhance_setting.en;
    attr->stManual.enhance_setting.EnhanceData.enhance_value =
        attr_v11.stManual.enhance_setting.EnhanceData.enhance_value;
    attr->stManual.enhance_setting.EnhanceData.enhance_chroma =
        attr_v11.stManual.enhance_setting.EnhanceData.enhance_chroma;
    for (int i = 0; i < DHAZ_ENHANCE_CURVE_KNOTS_NUM; i++)
        attr->stManual.enhance_setting.enhance_curve[i] =
            attr_v11.stManual.enhance_setting.enhance_curve[i];
    attr->stManual.hist_setting.en           = attr_v11.stManual.hist_setting.en;
    attr->stManual.hist_setting.hist_para_en = attr_v11.stManual.hist_setting.hist_para_en;
    attr->stManual.hist_setting.HistData     = attr_v11.stManual.hist_setting.HistData;
    // set info
    attr->stDehazeManu.update  = attr_v11.Info.updateMDehazeStrth;
    attr->stDehazeManu.level   = attr_v11.Info.MDehazeStrth;
    attr->stEnhanceManu.update = attr_v11.Info.updateMEnhanceStrth;
    attr->stEnhanceManu.level  = attr_v11.Info.MEnhanceStrth;
#endif
    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V10
XCamReturn rk_aiq_user_api2_adehaze_v10_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v10_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ADHAZ);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setAttribV10(attr);
        } else {
            XCamReturn ret                            = XCAM_RETURN_NO_ERROR;
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAdhazHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdhazHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) {
                    ret = singleCam_algo_handle->setSwAttribV10(attr);
                    if (ret != XCAM_RETURN_NO_ERROR) LOGE("%s returned: %d", __FUNCTION__, ret);
                }
            }
            return ret;
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setSwAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api2_adehaze_v10_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v10_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getAttribV10(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx)
                    continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) return singleCam_algo_handle->getSwAttribV10(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getSwAttribV10(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_adehaze_v10_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v10_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_adehaze_v10_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v10_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
XCamReturn rk_aiq_user_api2_adehaze_v11_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v11_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ADHAZ);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setSwAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setSwAttribV11(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_adehaze_v11_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v11_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getAttribV11(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) return singleCam_algo_handle->getSwAttribV11(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getSwAttribV11(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_adehaze_v11_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v11_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_adehaze_v11_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v11_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V12 && (USE_NEWSTRUCT == 0)
XCamReturn rk_aiq_user_api2_adehaze_v12_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v12_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ADHAZ);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setAttribV12(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) ret = singleCam_algo_handle->setSwAttribV12(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setSwAttribV12(attr);
        }
    }

    return ret;
}

XCamReturn rk_aiq_user_api2_adehaze_v12_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v12_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getAttribV12(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) return singleCam_algo_handle->getSwAttribV12(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getSwAttribV12(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_adehaze_v12_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v12_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_adehaze_v12_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v12_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V14 && (USE_NEWSTRUCT == 0)
XCamReturn rk_aiq_user_api2_adehaze_v14_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v14_t* attr) {
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ADHAZ);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setAttribV14(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) return singleCam_algo_handle->setSwAttribV14(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->setSwAttribV14(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn rk_aiq_user_api2_adehaze_v14_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v14_t* attr) {
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        RkAiqCamGroupAdehazeHandleInt* algo_handle =
            camgroupAlgoHandle<RkAiqCamGroupAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getAttribV14(attr);
        } else {
            const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
            for (auto camCtx : camgroup_ctx->cam_ctxs_array) {
                if (!camCtx) continue;

                RkAiqAdehazeHandleInt* singleCam_algo_handle =
                    algoHandle<RkAiqAdehazeHandleInt>(camCtx, RK_AIQ_ALGO_TYPE_ADHAZ);
                if (singleCam_algo_handle) return singleCam_algo_handle->getSwAttribV14(attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        RkAiqAdehazeHandleInt* algo_handle =
            algoHandle<RkAiqAdehazeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ADHAZ);

        if (algo_handle) {
            return algo_handle->getSwAttribV14(attr);
        }
    }

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn rk_aiq_user_api2_adehaze_v14_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    const adehaze_sw_v14_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_adehaze_v14_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx,
                                                    adehaze_sw_v14_t* attr) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

RKAIQ_END_DECLARE
