#ifndef _RK_AIQ_UAPI_AGIC_INT_H_
#define _RK_AIQ_UAPI_AGIC_INT_H_

#include "base/xcam_common.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_types_algo_agic_int.h"

typedef AgicAttr_t agic_attrib_t;


// need_sync means the implementation should consider
// the thread synchronization
// if called by RkAiqAdebayerHandleInt, the sync has been done
// in framework. And if called by user app directly,
// sync should be done in inner. now we just need implement
// the case of need_sync == false; need_sync is for future usage.

XCamReturn
rk_aiq_uapi_agic_SetAttrib
(
    RkAiqAlgoContext* ctx,
    agic_attrib_t attr,
    bool need_sync
);

XCamReturn
rk_aiq_uapi_agic_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    agic_attrib_t* attr
);

#endif//_RK_AIQ_UAPI_AGIC_INT_H_
