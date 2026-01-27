#ifndef _RK_AIQ_UAPI_ALDC_INT_H_
#define _RK_AIQ_UAPI_ALDC_INT_H_

#ifndef USE_NEWSTRUCT
#include "algos/aldc/rk_aiq_types_aldc_algo_int.h"
#include "algos/rk_aiq_algo_des.h"
#include "xcore/base/xcam_common.h"

enum LdcGenMeshState {
    kLdcGenMeshWorking = 0,
    kLdcGenMeshFinish,
};

/*
 * need_sync means the implementation should consider
 * the thread synchronization
 * if called by RkAiqAlscHandleInt, the sync has been done
 * in framework. And if called by user app directly,
 * sync should be done in inner. now we just need implement
 * the case of need_sync == false; need_sync is for future usage.
 */

XCamReturn rk_aiq_uapi_aldc_SetAttrib(RkAiqAlgoContext* ctx, const rk_aiq_ldc_attrib_t* attr,
                                      bool need_sync);
XCamReturn rk_aiq_uapi_aldc_GetAttrib(const RkAiqAlgoContext* ctx, rk_aiq_ldc_attrib_t* attr);
#endif

#endif
