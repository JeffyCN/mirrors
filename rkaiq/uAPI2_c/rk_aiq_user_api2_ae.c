/******************************************************************************
 *
 * Copyright 2019, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 *
 *
 *****************************************************************************/

#include "uAPI2/rk_aiq_user_api2_ae.h"

//#include "RkAiqCamGroupHandleInt.h"
#include "aiq_core_c/algo_handlers/RkAiqAeHandler.h"
#include "aiq_core_c/algo_camgroup_handlers/RkAiqCamGroupAeHandle.h"
#include "base/xcam_common.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#ifndef RKAIQ_HAVE_AE_V1
XCamReturn rk_aiq_user_api2_ae_setExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSwAttrV2_t expSwAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSwAttrV2_t* pExpSwAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinExpAttrV2_t linExpAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinExpAttrV2_t* pLinExpAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrExpAttrV2_t hdrExpAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrExpAttrV2_t* pHdrExpAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_ae_setLinAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinAeRouteAttr_t linAeRouteAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getLinAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinAeRouteAttr_t* pLinAeRouteAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setHdrAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getHdrAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_IrisAttrV2_t irisAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_IrisAttrV2_t* pIrisAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecSyncTest_t syncTestAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api2_ae_getSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecSyncTest_t* psyncTestAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_queryExpResInfo(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpQueryInfo_t* pExpResInfo)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setExpWinAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpWin_t ExpWin)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getExpWinAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpWin_t* pExpWin)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setAecStatsCfg(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecStatsCfg_t AecStatsCfg)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getAecStatsCfg(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecStatsCfg_t* pAecStatsCfg)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_setFrameHdrAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_FrameHdrAttr_t FrameHdrAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api2_ae_getFrameHdrAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_FrameHdrAttr_t* pFrameHdrAttr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#else
XCamReturn rk_aiq_user_api2_ae_setExpWinAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpWin_t ExpWin)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return (ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setExpWinAttr(algo_handle, ExpWin);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getExpWinAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpWin_t* pExpWin)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return (ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getExpWinAttr(algo_handle, pExpWin);
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_setAecStatsCfg(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecStatsCfg_t AecStatsCfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setAecStatsCfg(algo_handle, AecStatsCfg);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setAecStatsCfg(algo_handle, AecStatsCfg);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setAecStatsCfg(algo_handle, AecStatsCfg);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getAecStatsCfg(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecStatsCfg_t* pAecStatsCfg)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getAecStatsCfg(algo_handle, pAecStatsCfg);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getAecStatsCfg(algo_handle, pAecStatsCfg);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getAecStatsCfg(algo_handle, pAecStatsCfg);
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_setExpSubWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSubWin_t ExpSubWin
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return(ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setExpSubWinAttr(algo_handle, ExpSubWin);
        }
    }
    return(ret);

}
XCamReturn rk_aiq_user_api2_ae_getExpSubWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSubWin_t* pExpSubWin
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return(ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {

        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getExpSubWinAttr(algo_handle, pExpSubWin);
        }

    }

    return(ret);

}

XCamReturn rk_aiq_user_api2_ae_setFrameHdrAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_FrameHdrAttr_t FrameHdrAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setFrameHdrAttr(algo_handle, FrameHdrAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setFrameHdrAttr(algo_handle, FrameHdrAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setFrameHdrAttr(algo_handle, FrameHdrAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getFrameHdrAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_FrameHdrAttr_t* pFrameHdrAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getFrameHdrAttr(algo_handle, pFrameHdrAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getFrameHdrAttr(algo_handle, pFrameHdrAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getFrameHdrAttr(algo_handle, pFrameHdrAttr);
        }
    }

    return (ret);
}

#ifndef USE_NEWSTRUCT
XCamReturn rk_aiq_user_api2_ae_setExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSwAttrV2_t expSwAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setExpSwAttr(algo_handle, expSwAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setExpSwAttr(algo_handle, expSwAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setExpSwAttr(algo_handle, expSwAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSwAttrV2_t* pExpSwAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getExpSwAttr(algo_handle, pExpSwAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getExpSwAttr(algo_handle, pExpSwAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getExpSwAttr(algo_handle, pExpSwAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_setLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinExpAttrV2_t linExpAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setLinExpAttr(algo_handle, linExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setLinExpAttr(algo_handle, linExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setLinExpAttr(algo_handle, linExpAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinExpAttrV2_t* pLinExpAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getLinExpAttr(algo_handle, pLinExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getLinExpAttr(algo_handle, pLinExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getLinExpAttr(algo_handle, pLinExpAttr);
        }
    }
    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_setHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrExpAttrV2_t hdrExpAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setHdrExpAttr(algo_handle, hdrExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setHdrExpAttr(algo_handle, hdrExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setHdrExpAttr(algo_handle, hdrExpAttr);
        }
    }
    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrExpAttrV2_t* pHdrExpAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getHdrExpAttr(algo_handle, pHdrExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getHdrExpAttr(algo_handle, pHdrExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getHdrExpAttr(algo_handle, pHdrExpAttr);
        }
    }
    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_setLinAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinAeRouteAttr_t linAeRouteAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setLinAeRouteAttr(algo_handle, linAeRouteAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setLinAeRouteAttr(algo_handle, linAeRouteAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setLinAeRouteAttr(algo_handle, linAeRouteAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getLinAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinAeRouteAttr_t* pLinAeRouteAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getLinAeRouteAttr(algo_handle, pLinAeRouteAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getLinAeRouteAttr(algo_handle, pLinAeRouteAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getLinAeRouteAttr(algo_handle, pLinAeRouteAttr);
        }
    }
    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_setHdrAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setHdrAeRouteAttr(algo_handle, hdrAeRouteAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setHdrAeRouteAttr(algo_handle, hdrAeRouteAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setHdrAeRouteAttr(algo_handle, hdrAeRouteAttr);
        }
    }
    return(ret);
}
XCamReturn rk_aiq_user_api2_ae_getHdrAeRouteAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getHdrAeRouteAttr(algo_handle, pHdrAeRouteAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getHdrAeRouteAttr(algo_handle, pHdrAeRouteAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getHdrAeRouteAttr(algo_handle, pHdrAeRouteAttr);
        }
    }
    return(ret);
}
XCamReturn rk_aiq_user_api2_ae_setIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_IrisAttrV2_t irisAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return (ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setIrisAttr(algo_handle, irisAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_getIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_IrisAttrV2_t* pIrisAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return (ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getIrisAttr(algo_handle, pIrisAttr);
        }
    }
    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_setSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecSyncTest_t syncTestAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setSyncTestAttr(algo_handle, syncTestAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setSyncTestAttr(algo_handle, syncTestAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setSyncTestAttr(algo_handle, syncTestAttr);
        }
    }
    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_getSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecSyncTest_t* psyncTestAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getSyncTestAttr(algo_handle, psyncTestAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getSyncTestAttr(algo_handle, psyncTestAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getSyncTestAttr(algo_handle, psyncTestAttr);
        }
    }

    return (ret);
}
XCamReturn rk_aiq_user_api2_ae_queryExpResInfo(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpQueryInfo_t* pExpResInfo)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_queryExpInfo(algo_handle, pExpResInfo);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_queryExpInfo(algo_handle, pExpResInfo);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_queryExpInfo(algo_handle, pExpResInfo);
        }
    }
    return(ret);
}

#else
XCamReturn rk_aiq_user_api2_ae_setExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const ae_api_expSwAttr_t expSwAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

#if RKAIQ_HAVE_AIBNR
    ae_api_expSwAttr_t newExpSwAttr = expSwAttr;
#endif

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
#if RKAIQ_HAVE_AIBNR
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                ret = AibnrManager_setFrmRate((AiqAlgoHandler_t*)algo_handle, true, &sys_ctx->_rkAiqManager->mAibnrManager, &newExpSwAttr);
                if (ret != XCAM_RETURN_NO_ERROR) {
                    ret = AiqAlgoCamGroupAeHandler_setExpSwAttr(algo_handle, newExpSwAttr);
                }
            }
#else
            return AiqAlgoCamGroupAeHandler_setExpSwAttr(algo_handle, expSwAttr);
#endif
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];
                if (algo_handle) {
#if RKAIQ_HAVE_AIBNR
                    ret = AibnrManager_setFrmRate((AiqAlgoHandler_t*)algo_handle, false, &sys_ctx->_rkAiqManager->mAibnrManager, &newExpSwAttr);
                    if (ret != XCAM_RETURN_NO_ERROR) {
                        ret = AiqAlgoHandlerAe_setExpSwAttr(algo_handle, newExpSwAttr);
                    }
#else
                    ret = AiqAlgoHandlerAe_setExpSwAttr(algo_handle, expSwAttr);
#endif
                }
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
#if RKAIQ_HAVE_AIBNR
            ret = AibnrManager_setFrmRate((AiqAlgoHandler_t*)algo_handle, false, &sys_ctx->_rkAiqManager->mAibnrManager, &newExpSwAttr);
            if (ret != XCAM_RETURN_NO_ERROR) {
                return AiqAlgoHandlerAe_setExpSwAttr(algo_handle, newExpSwAttr);
            }
#else
            return AiqAlgoHandlerAe_setExpSwAttr(algo_handle, expSwAttr);
#endif
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_getExpSwAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_expSwAttr_t* pExpSwAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getExpSwAttr(algo_handle, pExpSwAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getExpSwAttr(algo_handle, pExpSwAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getExpSwAttr(algo_handle, pExpSwAttr);
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_setLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const ae_api_linExpAttr_t linExpAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setLinExpAttr(algo_handle, linExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setLinExpAttr(algo_handle, linExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setLinExpAttr(algo_handle, linExpAttr);
        }
    }

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_getLinExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_linExpAttr_t* pLinExpAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getLinExpAttr(algo_handle, pLinExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getLinExpAttr(algo_handle, pLinExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getLinExpAttr(algo_handle, pLinExpAttr);
        }
    }
    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_setHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const ae_api_hdrExpAttr_t hdrExpAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setHdrExpAttr(algo_handle, hdrExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setHdrExpAttr(algo_handle, hdrExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setHdrExpAttr(algo_handle, hdrExpAttr);
        }
    }
    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_getHdrExpAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_hdrExpAttr_t* pHdrExpAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getHdrExpAttr(algo_handle, pHdrExpAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getHdrExpAttr(algo_handle, pHdrExpAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getHdrExpAttr(algo_handle, pHdrExpAttr);
        }
    }
    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_setIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const ae_api_irisAttr_t irisAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return(ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {

        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setIrisAttr(algo_handle, irisAttr);
        }
    }

    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_getIrisAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_irisAttr_t* pIrisAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        LOGW("%s: not support camgroup mode!", __FUNCTION__);
        return(ret);
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {

        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getIrisAttr(algo_handle, pIrisAttr);
        }
    }
    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_setSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const ae_api_syncTestAttr_t syncTestAttr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_setSyncTestAttr(algo_handle, syncTestAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_setSyncTestAttr(algo_handle, syncTestAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_setSyncTestAttr(algo_handle, syncTestAttr);
        }
    }
    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_getSyncTestAttr(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_syncTestAttr_t* psyncTestAttr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t*)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getSyncTestAttr(algo_handle, psyncTestAttr);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getSyncTestAttr(algo_handle, psyncTestAttr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getSyncTestAttr(algo_handle, psyncTestAttr);
        }
    }

    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_setAttr(const rk_aiq_sys_ctx_t* sys_ctx, ae_param_t attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ae_api_expSwAttr_t expSwAttr;
    rk_aiq_user_api2_ae_getExpSwAttr(sys_ctx, &expSwAttr);
    expSwAttr.commCtrl = attr.commCtrl;
    rk_aiq_user_api2_ae_setExpSwAttr(sys_ctx, expSwAttr);

    rk_aiq_user_api2_ae_setLinExpAttr(sys_ctx, attr.linAeCtrl);
    rk_aiq_user_api2_ae_setHdrExpAttr(sys_ctx, attr.hdrAeCtrl);
    rk_aiq_user_api2_ae_setIrisAttr(sys_ctx, attr.irisCtrl);
    rk_aiq_user_api2_ae_setSyncTestAttr(sys_ctx, attr.syncTest);

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_getAttr(const rk_aiq_sys_ctx_t* sys_ctx, ae_param_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ae_api_expSwAttr_t expSwAttr;
    rk_aiq_user_api2_ae_getExpSwAttr(sys_ctx, &expSwAttr);
    attr->commCtrl = expSwAttr.commCtrl;

    rk_aiq_user_api2_ae_getLinExpAttr(sys_ctx, &attr->linAeCtrl);
    rk_aiq_user_api2_ae_getHdrExpAttr(sys_ctx, &attr->hdrAeCtrl);
    rk_aiq_user_api2_ae_getIrisAttr(sys_ctx, &attr->irisCtrl);
    rk_aiq_user_api2_ae_getSyncTestAttr(sys_ctx, &attr->syncTest);

    return (ret);
}

XCamReturn rk_aiq_user_api2_ae_queryExpResInfo(
    const rk_aiq_sys_ctx_t* sys_ctx,
    ae_api_queryInfo_t* pExpResInfo)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_queryExpInfo(algo_handle, pExpResInfo);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_queryExpInfo(algo_handle, pExpResInfo);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_queryExpInfo(algo_handle, pExpResInfo);
        }
    }
    return(ret);
}

XCamReturn rk_aiq_user_api2_ae_getRKAeStats(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_RkAeStats_t* pAeHwStats)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAeHandler_t* algo_handle =
            (AiqAlgoCamGroupAeHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoCamGroupAeHandler_getRkAeStats(algo_handle, pAeHwStats);
        } else {
            rk_aiq_sys_ctx_t* camCtx = NULL;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
                camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

                AiqAlgoHandlerAe_t* algo_handle =
                    (AiqAlgoHandlerAe_t*)camCtx ->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

                if (algo_handle)
                    ret = AiqAlgoHandlerAe_getRkAeStats(algo_handle, pAeHwStats);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif

    } else {
        AiqAlgoHandlerAe_t* algo_handle =
            (AiqAlgoHandlerAe_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AE];

        if (algo_handle) {
            return AiqAlgoHandlerAe_getRkAeStats(algo_handle, pAeHwStats);
        }
    }
    return(ret);
}


#endif

#endif

RKAIQ_END_DECLARE

