#ifndef _RK_AIQ_UAPI_ACNR_INT_V31_H_
#define _RK_AIQ_UAPI_ACNR_INT_V31_H_

#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"
#include "acnrV31/rk_aiq_types_acnr_algo_int_v31.h"

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAlscHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.

XCamReturn
rk_aiq_uapi_acnrV31_SetAttrib(RkAiqAlgoContext *ctx,
                              const rk_aiq_cnr_attrib_v31_t *attr,
                              bool need_sync);

XCamReturn
rk_aiq_uapi_acnrV31_GetAttrib(const RkAiqAlgoContext *ctx,
                              rk_aiq_cnr_attrib_v31_t *attr);

XCamReturn
rk_aiq_uapi_acnrV31_SetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                        const rk_aiq_cnr_strength_v31_t* pStrength);

XCamReturn
rk_aiq_uapi_acnrV31_GetChromaSFStrength(const RkAiqAlgoContext *ctx,
                                        rk_aiq_cnr_strength_v31_t* pStrength);

XCamReturn
rk_aiq_uapi_acnrV31_GetInfo(const RkAiqAlgoContext* ctx,
                            rk_aiq_cnr_info_v31_t* pInfo) ;

#endif
