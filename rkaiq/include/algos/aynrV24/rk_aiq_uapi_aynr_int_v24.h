#ifndef _RK_AIQ_UAPI_AYNR_INT_V24_H_
#define _RK_AIQ_UAPI_AYNR_INT_V24_H_

#include "aynrV24/rk_aiq_types_aynr_algo_int_v24.h"
#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAlscHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.

XCamReturn rk_aiq_uapi_aynrV24_SetAttrib(RkAiqAlgoContext* ctx, const rk_aiq_ynr_attrib_v24_t* attr,
                                         bool need_sync);

XCamReturn rk_aiq_uapi_aynrV24_GetAttrib(const RkAiqAlgoContext* ctx,
                                         rk_aiq_ynr_attrib_v24_t* attr);

XCamReturn rk_aiq_uapi_aynrV24_SetLumaSFStrength(const RkAiqAlgoContext* ctx,
                                                 const rk_aiq_ynr_strength_v24_t* pStrength);

XCamReturn rk_aiq_uapi_aynrV24_GetLumaSFStrength(const RkAiqAlgoContext* ctx,
                                                 rk_aiq_ynr_strength_v24_t* pStrength);

XCamReturn rk_aiq_uapi_aynrV24_GetInfo(const RkAiqAlgoContext* ctx, rk_aiq_ynr_info_v24_t* pInfo);

#endif
