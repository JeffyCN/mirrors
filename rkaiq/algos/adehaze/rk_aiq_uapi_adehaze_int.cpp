#include "rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_types_adehaze_algo_prvt.h"
#include "xcam_log.h"

#if RKAIQ_HAVE_DEHAZE_V10
XCamReturn rk_aiq_uapi_adehaze_v10_SetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v10_t* attr,
                                             bool need_sync) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

    // AdehazeHandle->AdehazeAtrrV10.mode = attr->mode;
    // if(attr->mode == DEHAZE_API_MANUAL)
    //    memcpy(&AdehazeHandle->AdehazeAtrrV10.stManual, &attr->stManual,
    //    sizeof(mDehazeAttrV11_t));
    // if(attr->mode == DEHAZE_API_AUTO)
    //    memcpy(&AdehazeHandle->AdehazeAtrrV10.stAuto, &attr->stAuto,
    //    sizeof(CalibDbV2_dehaze_v10_t));

    return ret;
}

XCamReturn rk_aiq_uapi_adehaze_v10_GetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v10_t* attr) {
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

    // attr->mode = AdehazeHandle->AdehazeAtrrV10.mode;
    // memcpy(&attr->stManual, &AdehazeHandle->AdehazeAtrrV10.stManual, sizeof(mDehazeAttrV11_t));
    // memcpy(&attr->stAuto, &AdehazeHandle->AdehazeAtrrV10.stAuto, sizeof(CalibDbV2_dehaze_v10_t));

    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V11 || RKAIQ_HAVE_DEHAZE_V11_DUO
XCamReturn rk_aiq_uapi_adehaze_v11_SetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v11_t* attr,
                                             bool need_sync) {
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

#if RKAIQ_HAVE_DEHAZE_V11
    pAdehazeHandle->AdehazeAtrrV11.mode = attr->mode;
    if (attr->mode == DEHAZE_API_MANUAL) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV11.stManual, &attr->stManual, sizeof(mDehazeAttrV11_t));
        pAdehazeHandle->ifReCalcStManual = true;
    }
    if (attr->mode == DEHAZE_API_AUTO) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV11.stAuto, &attr->stAuto,
               sizeof(CalibDbV2_dehaze_v11_t));
        pAdehazeHandle->ifReCalcStAuto = true;
    }

    if (attr->Info.updateMDehazeStrth) {
        pAdehazeHandle->AdehazeAtrrV11.Info.MDehazeStrth = attr->Info.MDehazeStrth;
        attr->Info.updateMDehazeStrth                    = false;
    }
    if (attr->Info.updateMEnhanceStrth) {
        pAdehazeHandle->AdehazeAtrrV11.Info.MEnhanceStrth = attr->Info.MEnhanceStrth;
        attr->Info.updateMEnhanceStrth                    = false;
    }
    if (attr->Info.updateMEnhanceChromeStrth) {
        pAdehazeHandle->AdehazeAtrrV11.Info.MEnhanceChromeStrth = attr->Info.MEnhanceChromeStrth;
        attr->Info.updateMEnhanceChromeStrth                    = false;
    }
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    pAdehazeHandle->AdehazeAtrrV11duo.mode = attr->mode;
    if (attr->mode == DEHAZE_API_MANUAL) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV11duo.stManual, &attr->stManual,
               sizeof(mDehazeAttrV11_t));
        pAdehazeHandle->ifReCalcStManual = true;
    }
    if (attr->mode == DEHAZE_API_AUTO) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV11duo.stAuto, &attr->stAuto,
               sizeof(CalibDbV2_dehaze_v11_t));
        pAdehazeHandle->ifReCalcStAuto = true;
    }

    if (attr->Info.updateMDehazeStrth) {
        pAdehazeHandle->AdehazeAtrrV11duo.Info.MDehazeStrth = attr->Info.MDehazeStrth;
        attr->Info.updateMDehazeStrth                       = false;
    }
    if (attr->Info.updateMEnhanceStrth) {
        pAdehazeHandle->AdehazeAtrrV11duo.Info.MEnhanceStrth = attr->Info.MEnhanceStrth;
        attr->Info.updateMEnhanceStrth                       = false;
    }
    if (attr->Info.updateMEnhanceChromeStrth) {
        pAdehazeHandle->AdehazeAtrrV11duo.Info.MEnhanceChromeStrth = attr->Info.MEnhanceChromeStrth;
        attr->Info.updateMEnhanceChromeStrth                       = false;
    }
#endif
    return ret;
}

XCamReturn rk_aiq_uapi_adehaze_v11_GetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v11_t* attr) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

#if RKAIQ_HAVE_DEHAZE_V11
    attr->mode = pAdehazeHandle->AdehazeAtrrV11.mode;
    memcpy(&attr->stManual, &pAdehazeHandle->AdehazeAtrrV11.stManual, sizeof(mDehazeAttrV11_t));
    memcpy(&attr->stAuto, &pAdehazeHandle->AdehazeAtrrV11.stAuto, sizeof(CalibDbV2_dehaze_v11_t));
    memcpy(&attr->Info, &pAdehazeHandle->AdehazeAtrrV11.Info, sizeof(mDehazeAttrInfoV11_t));
#endif
#if RKAIQ_HAVE_DEHAZE_V11_DUO
    attr->mode = pAdehazeHandle->AdehazeAtrrV11duo.mode;
    memcpy(&attr->stManual, &pAdehazeHandle->AdehazeAtrrV11duo.stManual, sizeof(mDehazeAttrV11_t));
    memcpy(&attr->stAuto, &pAdehazeHandle->AdehazeAtrrV11duo.stAuto,
           sizeof(CalibDbV2_dehaze_v11_t));
    memcpy(&attr->Info, &pAdehazeHandle->AdehazeAtrrV11duo.Info, sizeof(mDehazeAttrInfoV11_t));
#endif

    return ret;
}
#endif
#if RKAIQ_HAVE_DEHAZE_V12
XCamReturn rk_aiq_uapi_adehaze_v12_SetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v12_t* attr,
                                             bool need_sync) {
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

    pAdehazeHandle->AdehazeAtrrV12.mode = attr->mode;
    if (attr->mode == DEHAZE_API_MANUAL) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV12.stManual, &attr->stManual, sizeof(mDehazeAttrV12_t));
        pAdehazeHandle->ifReCalcStManual = true;
    }
    if (attr->mode == DEHAZE_API_AUTO) {
        memcpy(&pAdehazeHandle->AdehazeAtrrV12.stAuto, &attr->stAuto,
               sizeof(CalibDbV2_dehaze_v12_t));
        pAdehazeHandle->ifReCalcStAuto = true;
    }

    if (attr->Info.updateMDehazeStrth) {
        pAdehazeHandle->AdehazeAtrrV12.Info.MDehazeStrth = attr->Info.MDehazeStrth;
        attr->Info.updateMDehazeStrth                    = false;
    }
    if (attr->Info.updateMEnhanceStrth) {
        pAdehazeHandle->AdehazeAtrrV12.Info.MEnhanceStrth = attr->Info.MEnhanceStrth;
        attr->Info.updateMEnhanceStrth                    = false;
    }
    if (attr->Info.updateMEnhanceChromeStrth) {
        pAdehazeHandle->AdehazeAtrrV12.Info.MEnhanceChromeStrth = attr->Info.MEnhanceChromeStrth;
        attr->Info.updateMEnhanceChromeStrth                    = false;
    }

    return ret;
}

XCamReturn rk_aiq_uapi_adehaze_v12_GetAttrib(RkAiqAlgoContext* ctx, adehaze_sw_v12_t* attr) {
    XCamReturn ret                 = XCAM_RETURN_NO_ERROR;
    AdehazeHandle_t* pAdehazeHandle = (AdehazeHandle_t*)ctx;

    attr->mode = pAdehazeHandle->AdehazeAtrrV12.mode;
    attr->Info = pAdehazeHandle->AdehazeAtrrV12.Info;
    memcpy(&attr->stManual, &pAdehazeHandle->AdehazeAtrrV12.stManual, sizeof(mDehazeAttrV12_t));
    memcpy(&attr->stAuto, &pAdehazeHandle->AdehazeAtrrV12.stAuto, sizeof(CalibDbV2_dehaze_v12_t));

    return ret;
}

#endif
