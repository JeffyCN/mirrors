#ifndef _RK_AIQ_UAPI_ARGBIR_INT_H_
#define _RK_AIQ_UAPI_ARGBIR_INT_H_

#include "argbir_uapi_head.h"
#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_types_argbir_algo_int.h"

// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAwbHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.

XCamReturn rk_aiq_uapi_argbir_v10_SetAttrib(RkAiqAlgoContext* ctx, const RgbirAttrV10_t* attr,
                                            bool need_sync);
XCamReturn rk_aiq_uapi_argbir_v10_GetAttrib(RkAiqAlgoContext* ctx, RgbirAttrV10_t* attr);

#endif
