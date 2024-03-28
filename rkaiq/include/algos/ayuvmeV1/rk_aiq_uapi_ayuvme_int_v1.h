#ifndef _RK_AIQ_UAPI_AYUVME_INT_V1_H_
#define _RK_AIQ_UAPI_AYUVME_INT_V1_H_

#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"
#include "ayuvmeV1/rk_aiq_types_ayuvme_algo_int_v1.h"

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAlscHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.


XCamReturn
rk_aiq_uapi_ayuvmeV1_SetAttrib(RkAiqAlgoContext *ctx,
                               const rk_aiq_yuvme_attrib_v1_t *attr,
                               bool need_sync);

XCamReturn
rk_aiq_uapi_ayuvmeV1_GetAttrib(const RkAiqAlgoContext *ctx,
                               rk_aiq_yuvme_attrib_v1_t *attr);

XCamReturn
rk_aiq_uapi_ayuvmeV1_SetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                       const rk_aiq_yuvme_strength_v1_t* pStrength);

XCamReturn
rk_aiq_uapi_ayuvmeV1_GetLumaSFStrength(const RkAiqAlgoContext *ctx,
                                       rk_aiq_yuvme_strength_v1_t* pStrength);


XCamReturn
rk_aiq_uapi_ayuvmeV1_GetInfo(const RkAiqAlgoContext* ctx,
                             rk_aiq_yuvme_info_v1_t* pInfo);



#endif
