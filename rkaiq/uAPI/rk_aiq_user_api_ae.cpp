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

#include "base/xcam_common.h"
#include "rk_aiq_user_api_ae.h"
#include "algo_handlers/RkAiqAeHandle.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_AE_V1

XCamReturn rk_aiq_user_api_ae_setExpSwAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSwAttr_t expSwAttr
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setExpSwAttr(expSwAttr);
    }

    return (ret);

}
XCamReturn rk_aiq_user_api_ae_getExpSwAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSwAttr_t*        pExpSwAttr
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getExpSwAttr(pExpSwAttr);
    }

    return (ret);
}
XCamReturn rk_aiq_user_api_ae_setLinAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinAeRouteAttr_t linAeRouteAttr
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setLinAeRouteAttr(linAeRouteAttr);
    }

    return(ret);
}
XCamReturn rk_aiq_user_api_ae_getLinAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinAeRouteAttr_t* pLinAeRouteAttr
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getLinAeRouteAttr(pLinAeRouteAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_setHdrAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setHdrAeRouteAttr(hdrAeRouteAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_getHdrAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getHdrAeRouteAttr(pHdrAeRouteAttr);
    }

    return(ret);

}

XCamReturn rk_aiq_user_api_ae_setLinExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinExpAttr_t linExpAttr
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setLinExpAttr(linExpAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_getLinExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinExpAttr_t* pLinExpAttr
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getLinExpAttr(pLinExpAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_setHdrExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrExpAttr_t hdrExpAttr
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setHdrExpAttr(hdrExpAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_getHdrExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrExpAttr_t* pHdrExpAttr
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getHdrExpAttr(pHdrExpAttr);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_queryExpResInfo
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpQueryInfo_t* pExpResInfo
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->queryExpInfo(pExpResInfo);
    }

    return(ret);

}

XCamReturn rk_aiq_user_api_ae_setExpWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpWin_t ExpWin
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setExpWinAttr(ExpWin);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_getExpWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpWin_t* pExpWin
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getExpWinAttr(pExpWin);
    }

    return(ret);

}

XCamReturn rk_aiq_user_api_ae_setAecStatsCfg
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecStatsCfg_t AecStatsCfg
) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AE);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->setAecStatsCfg(AecStatsCfg);
    }

    return(ret);

}
XCamReturn rk_aiq_user_api_ae_getAecStatsCfg
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecStatsCfg_t* pAecStatsCfg
) {
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAeHandleInt* algo_handle =
        algoHandle<RkAiqAeHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_AE);

    if (algo_handle) {
        return algo_handle->getAecStatsCfg(pAecStatsCfg);
    }

    return(ret);

}

#else

XCamReturn rk_aiq_user_api_ae_setExpSwAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpSwAttr_t expSwAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getExpSwAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpSwAttr_t*        pExpSwAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_setLinAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinAeRouteAttr_t linAeRouteAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getLinAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinAeRouteAttr_t* pLinAeRouteAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_setHdrAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrAeRouteAttr_t hdrAeRouteAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getHdrAeRouteAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn rk_aiq_user_api_ae_setLinExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_LinExpAttr_t linExpAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getLinExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_LinExpAttr_t* pLinExpAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_setHdrExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_HdrExpAttr_t hdrExpAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getHdrExpAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_HdrExpAttr_t* pHdrExpAttr
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_queryExpResInfo
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpQueryInfo_t* pExpResInfo
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_setExpWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_ExpWin_t ExpWin
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getExpWinAttr
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_ExpWin_t* pExpWin
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_setAecStatsCfg
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    const Uapi_AecStatsCfg_t AecStatsCfg
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}
XCamReturn rk_aiq_user_api_ae_getAecStatsCfg
(
    const rk_aiq_sys_ctx_t* sys_ctx,
    Uapi_AecStatsCfg_t* pAecStatsCfg
) {
    return XCAM_RETURN_ERROR_UNKNOWN;
}

#endif

RKAIQ_END_DECLARE

